// 程序入口，不得修改
#include "my_scheduler.hpp"
#include "simulator.hpp"

int main() {
    MyScheduler sched;
    simulate([&](const View& v, int e) { return sched.decide(v, e); }, 2027);
}
