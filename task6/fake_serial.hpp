// 模拟环境，不得修改
#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "packets.hpp"

// 模拟一条有干扰的串口，另一端是以 500 Hz 发送 GimbalState 的下位机
class FakeSerial {
public:
    explicit FakeSerial(std::uint64_t seed);

    // 读取下位机发来的字节，最多 max_len 个，返回实际读到的字节数。
    // 每次读到的长度是随机的，可能只有半帧，也可能有好几帧；返回 0 表示数据已全部读完
    std::size_t read(std::uint8_t* buf, std::size_t max_len);

    // 核对解析出的 GimbalState 是否是下位机发出的一帧完好数据
    void check_state(const GimbalState& st);

    // 向下位机发送一帧 GimbalCmd。下位机要求一次 write 恰好是一整帧，帧格式和 CRC 正确，
    // 且 time_ms、yaw、pitch 与最近一次 check_state 的状态相同，fire 为 1
    void write(const std::uint8_t* data, std::size_t len);

    // 输出统计结果
    void report() const;

private:
    std::vector<std::uint8_t> stream_; // 下位机发出的全部字节（含干扰）
    std::size_t pos_ = 0; // 已被读走的字节数
    std::uint64_t rng_; // 决定每次 read 的长度
    std::vector<GimbalState> intact_; // 完好的 GimbalState 帧
    std::unordered_map<std::uint32_t, std::size_t> index_; // time_ms -> intact_ 下标
    std::vector<bool> decoded_; // intact_ 中每一帧是否已被正确解出
    GimbalState last_ {}; // 最近一次 check_state 的状态
    int sent_ = 0, decoded_count_ = 0, wrong_ = 0;
    int cmd_received_ = 0, cmd_bad_ = 0;
};
