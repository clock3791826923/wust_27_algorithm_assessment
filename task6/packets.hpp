// 数据包定义，不得修改
#pragma once

#include <cstdint>

// CMD = 0x01，上位机 -> 下位机，数据段 13 字节
struct GimbalCmd {
    std::uint32_t time_ms; // 上位机时间戳（ms）
    float yaw, pitch; // 目标角度（度）
    std::uint8_t fire; // 1 开火，0 不开火
};

// CMD = 0x02，下位机 -> 上位机，数据段 17 字节
struct GimbalState {
    std::uint32_t time_ms; // 下位机时间戳（ms）
    float yaw, pitch; // 当前云台角度（度）
    float bullet_speed; // 弹速（m/s）
    std::uint8_t enemy_color; // 0 红，1 蓝
};
