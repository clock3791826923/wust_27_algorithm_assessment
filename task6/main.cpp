// 程序入口，不得修改
#include <cstdint>
#include <vector>

#include "fake_serial.hpp"
#include "protocol.hpp"

int main() {
    FakeSerial serial(2027);
    Decoder decoder;
    std::uint8_t buf[256];
    while (std::size_t n = serial.read(buf, sizeof(buf))) {
        decoder.feed(buf, n);
        GimbalState st;
        while (decoder.next(st)) {
            serial.check_state(st); // 核对解出的数据
            const std::vector<std::uint8_t> frame =
                encode(GimbalCmd { st.time_ms, st.yaw, st.pitch, 1 });
            serial.write(frame.data(), frame.size()); // 回复一帧云台指令
        }
    }
    serial.report();
}
