#include "my_scheduler.hpp"
#include "simulator.hpp"
//room表示还有空位
Action MyScheduler::decide(const View& v,int e) 
{
    if (e==0)
    {
        //if (e!=0) return Action::Idle;
        Car c=v.cars[0];
        int f=c.floor;//当前楼层
        bool room;
        if (c.load<c.capacity) room=true;
        else room=false;
        // 当前楼层有活：立刻开门
        bool up=room && v.hall_up[f]!=-1;
        bool down=room && v.hall_down[f]!=-1;
        bool car=c.car_call[f]!=-1;
        if (up || down || car)
        {
            if (up && down)
            {
                if (dir_1>0) return Action::OpenUp;
                else return Action::OpenDown;
            }//两边都有人，顺着当前方向
            if (up)
                return Action::OpenUp;
            if (down)
                return Action::OpenDown;
            return Action::OpenUp; //只是轿厢内有人要下梯，方向无所谓
        }
        //本层没活：顺着dir_ 找最近的一个召唤；前面没有就调头再找
        int target=-1;//目标楼层
        int p=0;
        while (p<2 && target<0)
        {
            if (p==1)
                dir_1=-dir_1;
            int fl=f+dir_1;
            while (fl>=1 && fl<=kFloors)
            {
                if (hc1(v, fl, room)) 
                {
                    target=fl;
                    break;
                }
                fl+=dir_1;
            }
            p++;
        }
        //一个召唤都没有：休息
        if (target<0)
            return Action::Idle;
        if (target>f) return Action::Up;
        else return Action::Down;
    } 
    else
    {
        Car c=v.cars[1];
        int f=c.floor;//当前楼层
        bool room;
        if (c.load<c.capacity) room=true;
        else room=false;
        // 当前楼层有活：立刻开门
        bool up=room && v.hall_up[f]!=-1;
        bool down=room && v.hall_down[f]!=-1;
        bool car=c.car_call[f]!=-1;
        if (up || down || car)
        {
            if (up && down)
            {
                if (dir_2>0) return Action::OpenUp;
                else return Action::OpenDown;
            }//两边都有人，顺着当前方向
            if (up)
                return Action::OpenUp;
            if (down)
                return Action::OpenDown;
            return Action::OpenDown; //只是轿厢内有人要下梯，方向无所谓
        }
        //本层没活：顺着dir_ 找最近的一个召唤；前面没有就调头再找
        int target=-1;//目标楼层
        int p=0;
        while (p<2 && target<0)
        {
            if (p==1)
                dir_2=-dir_2;
            int fl=f+dir_2;
            while (fl>=1 && fl<=kFloors)
            {
                if (hc2(v, fl, room)) 
                {
                    target = fl;
                    break;
                }
                fl+=dir_2;
            }
            p++;
        }
        //一个召唤都没有：休息
        if (target<0)
            return Action::Idle;
        if (target>f) return Action::Up;
        else return Action::Down;
    }
}
bool MyScheduler::hc1(const View& v, int floor, bool room) 
{
    if (v.cars[0].car_call[floor]!=-1)//轿厢里有人要下梯
        return true;
    return room && (v.hall_up[floor]!=-1 || v.hall_down[floor]!=-1);//本层有人按了召唤钮
}//本层有活：有人按了这层的召唤钮，或者轿厢里有人要在这层下
bool MyScheduler::hc2(const View& v, int floor, bool room) 
{
    if (v.cars[1].car_call[floor]!=-1)//轿厢里有人要下梯
        return true;
    return room && (v.hall_up[floor]!=-1 || v.hall_down[floor]!=-1);//本层有人按了召唤钮
}//本层有活：有人按了这层的召唤钮，或者轿厢里有人要在这层下