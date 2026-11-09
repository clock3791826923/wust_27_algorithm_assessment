// 模拟环境，不得修改
#include "simulator.hpp"

#include <algorithm>
#include <cstdio>
#include <deque>
#include <vector>

namespace {

struct Passenger {
    int arrive, from, to; // 到达时刻、出发楼层、目的楼层
    int patience; // 最多愿意等多久（s），超过就改走楼梯
    int board = -1; // 上梯时刻
    int leave = -1; // 下梯时刻
};

// 固定种子的伪随机数（splitmix64），保证所有人生成的乘客序列相同
class Rng {
public:
    explicit Rng(std::uint64_t seed): s_(seed) {}
    std::uint64_t next() {
        std::uint64_t z = (s_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
    int uniform(int lo, int hi) {
        return lo + static_cast<int>(next() % (hi - lo + 1));
    }
    double real() {
        return (next() >> 11) * 0x1.0p-53;
    }

private:
    std::uint64_t s_;
};

// 一天的客流：上课前 15 min 大量乘客从 1 楼上楼，下课后 10 min 大量乘客下到 1 楼，其余时间只有零散客流。
// 去高层的人更倾向于坐电梯；乘客等待超过 patience 就改走楼梯，要走的楼层越多越有耐心
std::vector<Passenger> make_traffic(std::uint64_t seed) {
    Rng rng(seed);
    auto upper_floor = [&] { // 2 ~ 6 层，权重依次为 1 ~ 5
        int r = rng.uniform(1, 15);
        int f = 2;
        while (r > f - 1)
            r -= f - 1, ++f;
        return f;
    };
    std::vector<Passenger> ps;
    auto add = [&](int t, int from, int to) {
        const int floors = from > to ? from - to : to - from;
        ps.push_back({ t, from, to, 30 + 30 * floors });
    };
    for (int t = kDayStart; t < kDayEnd; ++t) {
        double up = 1.0 / 120, down = 1.0 / 120, inter = 1.0 / 240; // 每秒到达一名乘客的概率
        for (const ClassPeriod& c: kClasses) {
            if (t >= c.begin - 900 && t < c.begin)
                up += 0.40 * c.scale, inter += 0.02 * c.scale;
            if (t >= c.end && t < c.end + 600)
                down += 0.45 * c.scale, inter += 0.02 * c.scale;
        }
        if (rng.real() < up)
            add(t, 1, upper_floor());
        if (rng.real() < down)
            add(t, upper_floor(), 1);
        if (rng.real() < inter) {
            const int from = rng.uniform(1, kFloors);
            int to;
            do
                to = rng.uniform(1, kFloors);
            while (to == from);
            add(t, from, to);
        }
    }
    return ps;
}

} // namespace

void simulate(const std::function<Action(const View&, int)>& decide, std::uint64_t seed) {
    std::vector<Passenger> ps = make_traffic(seed);
    const int n = static_cast<int>(ps.size());
    std::array<std::deque<int>, kFloors + 1> wait_up, wait_down;
    std::array<std::vector<int>, kElevators> inside;
    std::array<int, kElevators> busy_until {};
    std::array<int, kElevators> doors_ready {}; // 下客完毕、可以开始上客的时刻
    std::vector<int> pending_leave;
    View v {};
    v.hall_up.fill(-1);
    v.hall_down.fill(-1);
    for (int e = 0; e < kElevators; ++e) {
        v.cars[e] = { 1, 0, kCapacity[e], false, Action::Idle, {} };
        v.cars[e].car_call.fill(-1);
    }
    int next = 0, delivered = 0, gave_up = 0;

    for (v.now = kDayStart; delivered + gave_up < n; ++v.now) {
        {
            auto it = std::remove_if(pending_leave.begin(), pending_leave.end(), [&](int id) {
                if (ps[id].leave > v.now)
                    return false;
                ++delivered;
                return true;
            });
            pending_leave.erase(it, pending_leave.end());
        }
        if (delivered + gave_up >= n)
            break;
        if (v.now >= kTimeLimit) {
            bool stranded = false;
            for (int e = 0; e < kElevators; ++e)
                if (!inside[e].empty())
                    stranded = true;
            for (const Passenger& p: ps)
                if (p.leave > kTimeLimit)
                    stranded = true;
            if (stranded || v.now > kTimeLimit) {
                std::printf("FAIL: 23:00 前未能全部送达\n");
                return;
            }
        }
        for (; next < n && ps[next].arrive <= v.now; ++next) { // 新到达的乘客按下召唤按钮
            const Passenger& p = ps[next];
            const bool up = p.to > p.from;
            (up ? wait_up : wait_down)[p.from].push_back(next);
            int& button = up ? v.hall_up[p.from] : v.hall_down[p.from];
            if (button == -1)
                button = v.now;
        }
        for (int e = 0; e < kElevators; ++e)
            v.cars[e].busy = busy_until[e] > v.now;
        for (int e = 0; e < kElevators; ++e) { // 乘客上梯后，记录轿厢内按钮被按下的时刻
            Car& c = v.cars[e];
            for (int id: inside[e]) {
                if (ps[id].board <= v.now && c.car_call[ps[id].to] == -1)
                    c.car_call[ps[id].to] = ps[id].board; // 乘客上梯时按下的轿厢按钮
            }
        }
        for (int e = 0; e < kElevators; ++e) { // 开门期间新到的同方向乘客可以继续上梯
            Car& c = v.cars[e];
            if (c.doing != Action::OpenUp && c.doing != Action::OpenDown)
                continue;
            if (busy_until[e] < v.now) // 门已经关了一段时间
                continue;
            if (v.now < doors_ready[e]) // 还有下梯的乘客没走完
                continue;
            const bool dir_up = c.doing == Action::OpenUp;
            auto& q = dir_up ? wait_up[c.floor] : wait_down[c.floor];
            int& hall = dir_up ? v.hall_up[c.floor] : v.hall_down[c.floor];
            bool boarded = false;
            while (!q.empty() && static_cast<int>(inside[e].size()) < c.capacity) {
                const int id = q.front();
                q.pop_front();
                ps[id].board = v.now;
                inside[e].push_back(id);
                busy_until[e] += kTransferTime;
                boarded = true;
            }
            if (boarded) {
                c.load = static_cast<int>(inside[e].size());
                c.busy = true;
                if (q.empty())
                    hall = -1; // 上完最后一位，召唤按钮熄灭
            }
        }
        for (int e = 0; e < kElevators; ++e) { // 开门动作结束：门关后仍未上梯的乘客重新按下召唤按钮
            Car& c = v.cars[e];
            if (busy_until[e] != v.now)
                continue;
            if (c.doing != Action::OpenUp && c.doing != Action::OpenDown)
                continue;
            const bool dir_up = c.doing == Action::OpenUp;
            auto& q = dir_up ? wait_up[c.floor] : wait_down[c.floor];
            int& button = dir_up ? v.hall_up[c.floor] : v.hall_down[c.floor];
            if (!q.empty() && button == -1)
                button = v.now;
        }
        for (int e = 0; e < kElevators; ++e) { // 空闲的电梯依次决策
            if (busy_until[e] > v.now)
                continue;
            Car& c = v.cars[e];
            const Action a = decide(v, e);
            if ((a == Action::Up && c.floor == kFloors) || (a == Action::Down && c.floor == 1)) {
                std::printf("FAIL: 电梯 %d 在 %d 层执行了越界移动\n", e, c.floor);
                return;
            }
            c.doing = a;
            c.busy = true;
            if (a == Action::Up || a == Action::Down) {
                c.floor += a == Action::Up ? 1 : -1;
                busy_until[e] = v.now + kMoveTime;
            } else if (a == Action::OpenUp || a == Action::OpenDown) {
                const int f = c.floor;
                int out = 0;
                auto it = std::remove_if(inside[e].begin(), inside[e].end(), [&](int id) {
                    if (ps[id].to != f)
                        return false;
                    ++out;
                    ps[id].leave = v.now + kDoorTime + kTransferTime * out; // 开门后第 out 位下梯
                    pending_leave.push_back(id);
                    return true;
                });
                inside[e].erase(it, inside[e].end());
                c.car_call[f] = -1;
                doors_ready[e] = v.now + (out > 0 ? kDoorTime + kTransferTime * out : 0);
                auto& q = a == Action::OpenUp ? wait_up[f] : wait_down[f];
                int& button = a == Action::OpenUp ? v.hall_up[f] : v.hall_down[f];
                button = -1;
                int in = 0;
                while (!q.empty() && static_cast<int>(inside[e].size()) < c.capacity) {
                    const int id = q.front();
                    q.pop_front();
                    ++in;
                    ps[id].board =
                        v.now + kDoorTime + kTransferTime * (out + in); // 下客完后第 in 位上梯
                    inside[e].push_back(id);
                }
                busy_until[e] = v.now + kDoorTime + kTransferTime * (out + in);
            } else {
                busy_until[e] = v.now + 1;
            }
            c.load = static_cast<int>(inside[e].size());
        }
        for (int f = 1; f <= kFloors; ++f) { // 等太久的乘客改走楼梯，已按下的按钮不会熄灭
            for (auto* q: { &wait_up[f], &wait_down[f] }) {
                auto it = std::remove_if(q->begin(), q->end(), [&](int id) {
                    return v.now - ps[id].arrive > ps[id].patience;
                });
                gave_up += static_cast<int>(q->end() - it);
                q->erase(it, q->end());
            }
        }
    }

    for (const Passenger& p: ps) {
        if (p.leave > kTimeLimit) {
            std::printf("FAIL: 23:00 前未能全部送达\n");
            return;
        }
    }

    std::vector<int> trips;
    double trip_sum = 0, wait_sum = 0;
    for (const Passenger& p: ps) {
        if (p.leave == -1)
            continue;
        trips.push_back(p.leave - p.arrive);
        trip_sum += p.leave - p.arrive;
        wait_sum += p.board - p.arrive;
    }
    std::sort(trips.begin(), trips.end());
    const int d = static_cast<int>(trips.size());
    std::printf(
        "served %d/%d (%.1f%%)  gave_up %d  trip avg %5.1f s  p95 %3d s  wait avg %5.1f s\n",
        d,
        n,
        100.0 * d / n,
        gave_up,
        d ? trip_sum / d : 0.0,
        d ? trips[d * 95 / 100] : 0,
        d ? wait_sum / d : 0.0
    );
}
