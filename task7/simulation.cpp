// 模拟环境，不得修改
#include "simulation.hpp"

#include <thread>

using namespace std::chrono_literals;

Detection detect(const Frame& f) {
    unsigned h = static_cast<unsigned>(f.id);
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    std::this_thread::sleep_for(std::chrono::microseconds(4000 + h % 8001));
    return { f.id, f.stamp, true };
}

Target Tracker::track(const Detection& d) {
    if (d.id <= last_id_)
        ++order_errors_; // 乱序或重复调用
    last_id_ = d.id;
    std::this_thread::sleep_for(500us);
    if (d.detected) {
        target_ = { d.id, d.stamp };
        ++detected_count_;
    }
    return target_;
}
