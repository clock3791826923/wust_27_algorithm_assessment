#include "pipeline.hpp"
#include "ordered_queue.hpp"
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include "latest.hpp"
#include "simulation.hpp"
#include "solver.hpp"
#include <deque>
//学习临时笔记（这里很多问dsh,在对话中搞清过程）：
//理解接口：
//数据                 字段                   谁产生           谁使用       含义
//Frame	           id,stamp	               相机线程	          worker	  一帧原始图像
//Detection	       id, stamp, detected	   worker	        tracker   这一帧的检测结果
//Target	       id, stamp	           Tracker::track	solver	   最近一次有效观测
//Latest<Target>   一个 Target	           tracker 线程	      solver	 只保留最新值

//过程理解：相机产帧->放进frame存图->worker取帧YOLO->detect检测得到detection->放进有序队列a==>>（tracker取队列->tracker跟踪得到target敌人状态->放到latest缓冲)==>>解算线程
//                                                                                                                  消费者线程                        解算线程
//线程：woker*2(这里我打算把两个并行)、tracker、solver、主线程
void run_pipeline(int limit) 
{
//启动解算线程
    Tracker tracker;//追踪器
    Latest<Target> latest;//更新最新状态，掉过时的
    std::atomic<bool> stop { false };//原子操作
    std::thread solver(solver_loop, std::cref(latest), std::cref(stop));

//启动消费者线程(tracker)
    OrderedQueue<Detection> a;//收集乱序队列，Detection是推理产生的
    std::thread tracker_thread([&]
    {
        Detection d;//空表
        while (a.pop(d))//有就取，2500帧都处理完就结束线程
        {
            Target t=tracker.track(d);//跟踪
            latest.write(t);//结果发给解算
        }
    });

//启动生产者线程(worker)
    std::mutex mtx;//防止主线程和worker线程改running时乱套
    int running=0;//正在推理的个数
    std::vector<std::thread> workers; 
    auto t0=Clock::now();//记录开始时间
    for (int id=0; id<kFrames; id++) //枚举2500帧
    {
        std::this_thread::sleep_until(t0 + id * kPeriod);//睡到第id帧开始时间
        Frame f { id, Clock::now() };//造帧
        bool started=false;//默认这一帧不被允许推理
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (running<limit)
            {
                running++;
                started=true;
            }
        }
        if (!started) //丢帧,造个假帧不然在track卡了
        {
            a.push(Detection { f.id, f.stamp, false });
            continue;
        }
        workers.emplace_back([&, f] {// 开一个线程去推理,emplace_push可以
            a.push(detect(f));
            std::lock_guard<std::mutex> lock(mtx);
            running--;
        });
    }
//join
    for (auto& w : workers)
        w.join();
    a.close();//结束线程
    tracker_thread.join();
    stop=true;
    solver.join();
    std::printf("order_errors %d  detected %d/%d\n",tracker.order_errors(), tracker.detected_count(), kFrames);
}