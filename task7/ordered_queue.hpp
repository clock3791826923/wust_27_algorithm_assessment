#pragma once
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<map>
//这里我选用map因为用键值对的方式存储，更加方便
template<typename T>
class OrderedQueue 
{
public:
    void push(T item)//放入元素，取过的扔掉
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (closed) return;
        if (item.id<next_id) return;
        items.emplace(item.id,item);//插入
        cv.notify_all();
    }
    bool pop(T& out)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock,[&] { return items.count(next_id)>0 || closed; });
        auto it=items.find(next_id);
        if (it==items.end()) return false;
        out=it->second;//得到T
        items.erase(it);
        next_id++;
        return true;
    }
    void close()//没有新元素了
    {
        std::lock_guard<std::mutex> lock(mtx);
        closed=true;
        cv.notify_all();
    }
private:
    std::mutex mtx;
    bool closed=false;
    int next_id=0;
    std::map<int,T> items;
    std::condition_variable cv;
};  