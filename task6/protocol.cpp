#include "protocol.hpp"
#include<cstring>
void put_u32(std::vector<std::uint8_t>& out, std::uint32_t v) 
{
    for (int i=0; i<4; i++)
        out.push_back(static_cast<std::uint8_t>(v>>(8*i)));//转成小端序列 v>> 8*i 右移8*i位，取最低8位
}
void put_f32(std::vector<std::uint8_t>& out, float f) 
{
    std::uint32_t v;
    std::memcpy(&v, &f, sizeof(v));//把float的内存内容直接拷贝到uint32_t里
    put_u32(out, v);
}
std::uint32_t get_u32(std::uint8_t* p) 
{
    return static_cast<std::uint32_t>(p[0]) | static_cast<std::uint32_t>(p[1])<<8
        | static_cast<std::uint32_t>(p[2])<<16 | static_cast<std::uint32_t>(p[3])<<24;
}
float get_f32(std::uint8_t* p) 
{
    std::uint32_t v=get_u32(p);
    float f;
    std::memcpy(&f, &v, sizeof(f));
    return f;
}
//学习笔记：
//用多项式是为了在进行二进制计算时无需考虑进位问题
//除数由多项式各项系数组成
std::uint16_t crc16(const std::uint8_t* data, std::size_t len) 
{
    std::uint16_t crc=0xFFFF;//初始化crc寄存器（全是1）
    for (std::size_t i=0; i<len; i++)
    {
        crc=crc^static_cast<std::uint16_t>(data[i])<<8;//把data[i]放到crc寄存器的高8位，低8位清零
        for (int b=0; b <8; b++) 
        {
            if (crc & 0x8000)//如果crc寄存器的最高位是1，则左移一位后与多项式0x1021异或
                crc = static_cast<std::uint16_t>((crc << 1) ^ 0x1021);
            else//如果crc寄存器的最高位是0，则左移一位
                crc = static_cast<std::uint16_t>(crc << 1);
        }
    }
    return crc;
}
std::vector<std::uint8_t> encode(const GimbalCmd& cmd) 
{
    std::vector<std::uint8_t> f;
    f.push_back(0xA5);//帧头
    f.push_back(13);//长度
    f.push_back(0x01);//包类型 0x01表示GimbalCmd包 0x02表示GimbalState包
    put_u32(f, cmd.time_ms);//转成小端序列再拷入
    put_f32(f, cmd.yaw);
    put_f32(f, cmd.pitch);
    f.push_back(cmd.fire);
    std::uint16_t crc=crc16(f.data(), f.size());//.data()返回指向vector首元素的指针，size()返回vector的大小
    f.push_back(static_cast<std::uint8_t>(crc & 0xFF));//塞入低8位,和00000000000011111111做与运算，取出低8位
    f.push_back(static_cast<std::uint8_t>(crc >> 8));//塞入高8位
    return f;
}
void Decoder::feed(const std::uint8_t* data, std::size_t len) 
{
    zcq.insert(zcq.end(), data, data+len);
}
bool Decoder::next(GimbalState& out) 
{
    while (true) 
    {
        if (zcq.size() < 3)//长度不够，等下一次feed
            return false;
        if (zcq[0]!=0xA5) 
        { // 不是帧头，删掉
            zcq.erase(zcq.begin());
            continue;
        }
        std::size_t len=zcq[1];
        if (len>64) 
        { // 长度非法，这个A5不是真帧头
            zcq.erase(zcq.begin());
            continue;
        }
        std::size_t total=3+len+2;
        if (zcq.size()<total) // 半包，等下一次feed
            return false;
        std::uint16_t crc_calc=crc16(zcq.data(), 3 + len);
        std::uint16_t crc_in=static_cast<std::uint16_t>(zcq[3+len] | (zcq[3+len+1]<<8));//把两个字节拼成一个16位整数，低字节在前，高字节在后
        if (crc_calc!=crc_in) 
        { // 校验失败，从A5之后重扫
            zcq.erase(zcq.begin());
            continue;
        }
        if (zcq[2]!=0x02 || len!=17) 
        { //电控的帧，滚蛋
            zcq.erase(zcq.begin(), zcq.begin()+total);
            continue;
        }
        std::uint8_t* d=zcq.data()+3;//d指向第一个数据的内存
        out.time_ms=get_u32(d);//时间转回32位
        out.yaw=get_f32(d+4);//yaw转回float
        out.pitch=get_f32(d+8);//pitch转回float
        out.bullet_speed=get_f32(d+12);
        out.enemy_color=d[16];
        zcq.erase(zcq.begin(), zcq.begin()+total);
        return true;
    }
}
