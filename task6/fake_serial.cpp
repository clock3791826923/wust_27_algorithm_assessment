// 模拟环境，不得修改
#include "fake_serial.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

constexpr int kStateFrames = 5000; // 下位机以 500 Hz 发送 10 s
constexpr std::uint8_t kSof = 0xA5;

// 固定种子的伪随机数（splitmix64），保证所有人收到的字节流相同
std::uint64_t splitmix64(std::uint64_t& s) {
    std::uint64_t z = (s += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

class Rng {
public:
    explicit Rng(std::uint64_t seed): s_(seed) {}
    std::uint64_t next() {
        return splitmix64(s_);
    }
    int uniform(int lo, int hi) {
        return lo + static_cast<int>(next() % (hi - lo + 1));
    }
    double real() {
        return static_cast<double>(next() >> 11) / 9007199254740992.0;
    } // [0, 1)

private:
    std::uint64_t s_;
};

std::uint16_t reference_crc16(const std::uint8_t* data, std::size_t len) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < len; ++i) {
        crc ^= static_cast<std::uint16_t>(data[i] << 8);
        for (int b = 0; b < 8; ++b) {
            crc = (crc & 0x8000) ? static_cast<std::uint16_t>((crc << 1) ^ 0x1021)
                                 : static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}

void put_u32(std::vector<std::uint8_t>& out, std::uint32_t v) {
    for (int i = 0; i < 4; ++i)
        out.push_back(static_cast<std::uint8_t>(v >> (8 * i)));
}

void put_f32(std::vector<std::uint8_t>& out, float f) {
    std::uint32_t v;
    std::memcpy(&v, &f, sizeof(v));
    put_u32(out, v);
}

std::uint32_t get_u32(const std::uint8_t* p) {
    return static_cast<std::uint32_t>(p[0]) | static_cast<std::uint32_t>(p[1]) << 8
        | static_cast<std::uint32_t>(p[2]) << 16 | static_cast<std::uint32_t>(p[3]) << 24;
}

float get_f32(const std::uint8_t* p) {
    const std::uint32_t v = get_u32(p);
    float f;
    std::memcpy(&f, &v, sizeof(f));
    return f;
}

std::vector<std::uint8_t> make_frame(std::uint8_t cmd, const std::vector<std::uint8_t>& data) {
    std::vector<std::uint8_t> f { kSof, static_cast<std::uint8_t>(data.size()), cmd };
    f.insert(f.end(), data.begin(), data.end());
    const std::uint16_t crc = reference_crc16(f.data(), f.size());
    f.push_back(static_cast<std::uint8_t>(crc & 0xFF));
    f.push_back(static_cast<std::uint8_t>(crc >> 8));
    return f;
}

std::vector<std::uint8_t> state_frame(const GimbalState& st) {
    std::vector<std::uint8_t> data;
    put_u32(data, st.time_ms);
    put_f32(data, st.yaw);
    put_f32(data, st.pitch);
    put_f32(data, st.bullet_speed);
    data.push_back(st.enemy_color);
    return make_frame(0x02, data);
}

bool same(const GimbalState& a, const GimbalState& b) {
    return a.time_ms == b.time_ms && a.yaw == b.yaw && a.pitch == b.pitch
        && a.bullet_speed == b.bullet_speed && a.enemy_color == b.enemy_color;
}

} // namespace

FakeSerial::FakeSerial(std::uint64_t seed): rng_(seed ^ 0x5eedULL) {
    Rng rng(seed);
    auto garbage = [&](int n) { // 乱码中故意多放一些 0xA5，制造假帧头
        for (int i = 0; i < n; ++i) {
            stream_.push_back(rng.real() < 0.125 ? kSof : static_cast<std::uint8_t>(rng.next()));
        }
    };
    garbage(200); // 上电乱码
    const auto color = static_cast<std::uint8_t>(rng.uniform(0, 1));
    for (int i = 0; i < kStateFrames; ++i) {
        const double t = i * 0.002;
        const GimbalState st {
            static_cast<std::uint32_t>(10000 + 2 * i),
            static_cast<float>(30.0 * std::sin(0.8 * t) + 0.01 * rng.real()),
            static_cast<float>(5.0 * std::sin(1.3 * t) + 0.01 * rng.real()),
            static_cast<float>(15.5 + 0.5 * rng.real()),
            color,
        };
        if (rng.real() < 0.05) { // 其他命令字的帧（例如转发的裁判系统数据），CRC 正确
            std::vector<std::uint8_t> data(static_cast<std::size_t>(rng.uniform(0, 30)));
            for (auto& b: data)
                b = static_cast<std::uint8_t>(rng.next());
            const auto other = make_frame(static_cast<std::uint8_t>(rng.uniform(3, 9)), data);
            stream_.insert(stream_.end(), other.begin(), other.end());
        }
        if (rng.real() < 0.04)
            garbage(rng.uniform(1, 20)); // 电磁干扰产生的乱码

        std::vector<std::uint8_t> f = state_frame(st);
        const double r = rng.real();
        if (r < 0.03) { // 某个比特翻转
            f[static_cast<std::size_t>(rng.uniform(0, static_cast<int>(f.size()) - 1))] ^=
                static_cast<std::uint8_t>(1u << rng.uniform(0, 7));
        } else if (r < 0.06) { // 丢失 1~5 个字节
            const int k = rng.uniform(1, 5);
            const int at = rng.uniform(0, static_cast<int>(f.size()) - k);
            f.erase(f.begin() + at, f.begin() + at + k);
        } else {
            index_[st.time_ms] = intact_.size();
            intact_.push_back(st);
        }
        stream_.insert(stream_.end(), f.begin(), f.end());
        ++sent_;
    }
    decoded_.assign(intact_.size(), false);
}

std::size_t FakeSerial::read(std::uint8_t* buf, std::size_t max_len) {
    if (pos_ >= stream_.size() || max_len == 0)
        return 0;
    const std::uint64_t r = splitmix64(rng_);
    std::size_t n =
        (r % 10 == 0) ? max_len : 1 + (r >> 8) % 32; // 多数时候读到 1~32 字节，偶尔一次读满
    n = std::min({ n, max_len, stream_.size() - pos_ });
    std::memcpy(buf, stream_.data() + pos_, n);
    pos_ += n;
    return n;
}

void FakeSerial::check_state(const GimbalState& st) {
    last_ = st;
    const auto it = index_.find(st.time_ms);
    if (it == index_.end() || decoded_[it->second] || !same(intact_[it->second], st)) {
        ++wrong_; // 不是完好的帧、重复解出，或者数据被解错
        return;
    }
    decoded_[it->second] = true;
    ++decoded_count_;
}

void FakeSerial::write(const std::uint8_t* data, std::size_t len) {
    ++cmd_received_;
    const bool ok = len == 18 && data[0] == kSof && data[1] == 13 && data[2] == 0x01
        && reference_crc16(data, 16) == (data[16] | data[17] << 8)
        && get_u32(data + 3) == last_.time_ms && get_f32(data + 7) == last_.yaw
        && get_f32(data + 11) == last_.pitch && data[15] == 1;
    if (!ok)
        ++cmd_bad_;
}

void FakeSerial::report() const {
    std::printf(
        "state frames: sent %d  intact %zu  decoded %d  wrong %d\n",
        sent_,
        intact_.size(),
        decoded_count_,
        wrong_
    );
    std::printf("cmd frames:   received %d  bad %d\n", cmd_received_, cmd_bad_);
}
