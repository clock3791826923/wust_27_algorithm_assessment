#pragma once

#include <mutex>

// 单写多读的最新值缓冲
template<typename T>
class Latest {
public:
    void write(const T& v) {
        std::lock_guard lk(mtx_);
        value_ = v;
    }
    T read() const {
        std::lock_guard lk(mtx_);
        return value_;
    }

private:
    mutable std::mutex mtx_;
    T value_ {};
};
