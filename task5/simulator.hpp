// 模拟环境，不得修改
#pragma once

#include <array>
#include <cstdint>
#include <functional>

constexpr int kFloors = 6; // 楼层 1 ~ 6
constexpr int kElevators = 2; // 两部电梯
constexpr std::array<int, kElevators> kCapacity { 14, 15 }; // 额定载客人数
constexpr int kMoveTime = 4; // 移动一层耗时 4 s
constexpr int kDoorTime = 5; // 一次停靠中开门、关门共耗时 5 s
constexpr int kTransferTime = 1; // 每位乘客进出轿厢耗时 1 s
constexpr int kDayStart = 7 * 3600 + 30 * 60; // 07:30 开始有乘客
constexpr int kDayEnd = 22 * 3600; // 22:00 之后不再有新乘客
constexpr int kTimeLimit = 23 * 3600; // 23:00 前未全部送达（含下梯晚于 23:00），判定失败

// 作息时间表：上课时刻、下课时刻、该时段上课人数的相对比例。作息表是公开信息，调度器可以使用
struct ClassPeriod {
    int begin, end;
    double scale;
};
constexpr std::array<ClassPeriod, 5> kClasses { {
    { 8 * 3600 + 20 * 60, 10 * 3600, 1.0 }, // 1-2 节  08:20 ~ 10:00
    { 10 * 3600 + 20 * 60, 12 * 3600, 0.9 }, // 3-4 节  10:20 ~ 12:00
    { 14 * 3600, 15 * 3600 + 40 * 60, 0.8 }, // 5-6 节  14:00 ~ 15:40
    { 16 * 3600, 17 * 3600 + 40 * 60, 0.6 }, // 7-8 节  16:00 ~ 17:40
    { 18 * 3600 + 40 * 60, 20 * 3600 + 20 * 60, 0.5 }, // 9-10 节 18:40 ~ 20:20
} };

enum class Action {
    Up, // 上行一层
    Down, // 下行一层
    OpenUp, // 在本层开门，方向灯为上行：先下客，再让本层等候上行的乘客上客
    OpenDown, // 在本层开门，方向灯为下行：先下客，再让本层等候下行的乘客上客
    Idle, // 原地等待 1 s
};

// 一部电梯的状态
struct Car {
    int floor; // 空闲时为当前楼层；busy 为 true 时为本次动作完成后的楼层（移动中即目标楼层）
    int load; // 轿厢内人数
    int capacity; // 额定载客人数
    bool busy; // 是否正在执行动作
    Action doing; // 正在执行（或最近一次执行）的动作
    std::array<int, kFloors + 1>
        car_call; // 轿厢内各楼层按钮被按下的时刻，-1 表示未按下（下标 1 ~ kFloors）
};

// 调度器决策时能看到的信息。和真实电梯一样，只能看到按钮，看不到乘客本身
struct View {
    int now; // 当前时刻，从 0 点起算的秒数
    std::array<int, kFloors + 1> hall_up; // 各层上行召唤按钮被按下的时刻，-1 表示未按下
    std::array<int, kFloors + 1> hall_down; // 各层下行召唤按钮被按下的时刻，-1 表示未按下
    std::array<Car, kElevators> cars; // 两部电梯的状态
};

// 模拟一整天的电梯运行并输出统计结果。
// decide(v, e) 在电梯 e 空闲（上一个动作已完成、门已关闭）时被调用，返回它的下一个动作；seed 决定当天的客流
void simulate(const std::function<Action(const View&, int)>& decide, std::uint64_t seed);
