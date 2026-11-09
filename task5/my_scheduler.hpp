#pragma once
#include "simulator.hpp"
// 阶段一：只控制 0 号电梯。1 号电梯一直 Idle，当作不存在。
class MyScheduler 
{
public:
    Action decide(const View& v, int e);
private:
    int dir_1=1; //0号电梯的方向：+1 向上，-1 向下
    int dir_2=1;
    static bool hc1(const View& v, int floor, bool room);
    static bool hc2(const View& v, int floor, bool room);
};