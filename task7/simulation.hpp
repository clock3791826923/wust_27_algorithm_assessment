// 模拟环境，不得修改
#pragma once

#include <chrono>

using Clock = std::chrono::steady_clock;

struct Frame {
    int id;
    Clock::time_point stamp;
}; // id 从 0 开始连续递增，stamp 为采集时刻
struct Detection {
    int id;
    Clock::time_point stamp;
    bool detected;
}; // detected == false 表示该帧被跳过检测
struct Target {
    int id = -1;
    Clock::time_point stamp;
}; // 最近一次有效观测的帧号和采集时刻

constexpr int kFrames = 2500; // 共 2500 帧（10 s）
constexpr std::chrono::milliseconds kPeriod { 4 }; // 相机 250 Hz

// 模拟神经网络推理：耗时 4~12 ms（平均 8 ms），由帧号哈希唯一确定，相邻帧耗时差异较大。
// 用 sleep 模拟推理在 GPU 上进行，不占用 CPU
Detection detect(const Frame& f);

// 模拟跟踪器：耗时 0.5 ms，内部有状态，同一时刻只能被一个线程调用，且必须按帧号递增调用
class Tracker {
public:
    Target track(const Detection& d);
    int order_errors() const {
        return order_errors_;
    } // 乱序或重复调用的次数
    int detected_count() const {
        return detected_count_;
    } // 已被跟踪的、真正完成了检测的帧数（detected == true）；请在所有线程结束后读取

private:
    int last_id_ = -1;
    int order_errors_ = 0;
    int detected_count_ = 0;
    Target target_;
};
