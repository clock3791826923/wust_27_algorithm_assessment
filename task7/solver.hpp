#pragma once

#include <atomic>

#include "latest.hpp"
#include "simulation.hpp"

// 解算线程：1000 Hz 读取最新目标（弹道解算和指令下发在本题中省略），stop 置为 true 后退出
void solver_loop(const Latest<Target>& latest, const std::atomic<bool>& stop);
