#pragma once
#include "simulator.hpp"
class MyScheduler 
{
public:
    Action decide(const View& v, int e);
private:
    int dir_1=1; //0号电梯的方向：+1 向上，-1 向下
    int dir_2=1;
    bool hc1(const View& v, int floor, bool room);
    bool hc2(const View& v, int floor, bool room);
};