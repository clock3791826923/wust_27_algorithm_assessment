# 问答题的回答
## 一、介绍一些在c++中你常用的STL容器
> 每个容器具体的具体语法我就用我在学习过程中的手写笔记的图片展示（字有点烂O(∩_∩)O~~尴尬）点击note
1. **vector**  
[note1](vector1.JPG)   [note2](vector2.JPG)   
vector是动态数组容器  
优势：连续内存存储，可以随机访问，尾部增删高效、用起来简单等  
劣势：中间插入数据代价高，扩容有搬移成本，可能浪费内存等
2. **deque**  
[note1](deque1.JPG)  [note2](deque2.JPG)  
deque是双端队列容器  
优势：分段连续存储，头部尾部都可以增删，可以随机访问等  
劣势：顺序遍历不如vector，随机访问比vector慢一点，中间插入依然代价高，不能预留大块连续空间等
3. **stack&queue**  
[note](stack&queue.JPG)  
stack栈后进先出 queue队列先进先出  
stack特点：只能访问栈顶元素，不提供遍历（无迭代器）、访问和中间操作  
queue特点:和stack类似，就是能访问队头和队尾以及先进先出模式
4. **list**  
[note1](list1.JPG)    [note2](list2.JPG)  
list是双向链表容器  
优势：插入和删除高效，引用和指针稳定性好，支持双向遍历等  
劣势：不能随机访问，遍历性能较差，内存占用高等
5. **set&map**  
[setnote](set.JPG)   [mapnote](map.JPG)  
set和map都基于二叉搜索树 set可以自动排序
## 二、你对c++新标准有哪些了解，有哪些实用的特性，简要介绍一下  
>黑马教程外学的一些知识  

1.参数可用auto不用打一长串比如（std::filesysyte::path）这么长的代码  
2.用contexpr搭配模板可以在编译期间实例化，提前计算等提高算法运算效率  
3.线程：
```bash
std::thread p(producer);
std::thread c(consumer);
p.join();
c.join();
```
搭配：生产者消费者模型
```bash
生产者：
std::unique_lock<std::mutex> lock(mtx);
std::condition_variable g_cv;
g_cv.notify_one();
消费者：
std::unique_lock<std::mutex> lock(mtx);
bool isempty=g_queue.empty();
g_cv.wait(lock,!isempty);
```
4.chrono  
```bash
std::chrono::system_clock::now();//获取系统时间
std::chrono::steady_clock::now();//获取单调递增时间
std::chrono::high_resolution_clock::now();//获取最高精度时间

d.count();//返回时长的计数值
std::chrono::millisecons
std::chrono::duration_cast<目标单位>(d);
std::this_thread::sleep_for(std::chrono millisecons);//停顿
应用
std::chrono::steady_clock::time_point send_time()=std::chorono::steady_clock::now();
double hs=std::chrono::duration<double, std::milli>(ft- p.send_time).count();//算时间段
```
5.filesysystem  
```bash
std::filesystem::path path;
std::error_code ec;
std::filesystem::exists(path,ec);
auto time=std::filesystem::last_write_time(path,ec);//获取最后改动时间,加ec可以得到文件的错误
```
6.std::optional<函数return类型>  函数名（参数）   //无值可以输出std::nullopt  
## 三、为什么c++需要预先编译而不能像python一样直接运行，从源文件到可执行文件编译工具链做了哪些操作
c++是编译型语言，在编译时需要被翻译成二进制机器码给CPU执行并对语法错误进行检查；python是解释型语言，由解释器边翻译边执行。
## 四、在linux中为什么要用sudo，为什么不全程使用root账
sudo是普通用户,root属于完全权限，如果用root有可能在误操作时把整个系统搞崩，用sudo更加安全可控。
## 五、在RM中算法组主要做什么工作，新赛季中有哪些规则变动属于算法组的工作
我认为算法组是战队的大脑和眼睛，视觉负责让机器能识别和瞄准，运控让机器能定位并控制机器的移动。新赛季将英雄和工程机器人和并成重装机器人，应该在决策算法上会有改变，然后对新机器人的视觉方面进行调整。
## 六、请你总结一下这段时间的学习经历及经验，以及后续对RM比赛的计划
这段时间我学习了linux系统，cmake,clangd这些工具的使用,学习了c++的一些基础知识，并通过作业和考核题自己另外又补充了不少算法知识，比如多线程，shell,YAML参数怎么和算法联系，串口通信等，收获满满。目前已经了解了上位机怎么和下位机联系，感觉距离能调机器又进了一步，但感觉离真正能去设计rm的算法还要走很长一段路，还需继续学习。所以后续选完方向后，我会继续学习，通过一些开源的代码学习别人的代码思路，理解整机需求，希望能为战队出一份力。
