#pragma once

// 串行版本：相机线程 + 单个处理线程（检测 -> 跟踪），运行结束后输出跟踪器的乱序次数
void run_serial();
