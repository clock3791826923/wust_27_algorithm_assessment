#pragma once
#include"packets.hpp"
#include"fake_serial.hpp"
#include<iostream>
#include<vector>
std::uint16_t crc16(const std::uint8_t* data, std::size_t len);//计算CRC16校验码
std::vector<std::uint8_t> encode(const GimbalCmd& cmd);//电控组数据变成一个完整的帧
class Decoder 
{
public:
    void feed(const std::uint8_t* data, std::size_t len);//往暂存区放数据
    bool next(GimbalState& out);//读取一个完整的GimbalState包，如果没有返回false
private:
    std::vector<std::uint8_t> zcq;
};