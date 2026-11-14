#include "serial.hpp"

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

void run_serial() {
    Tracker tracker;
    Latest<Target> latest;
    std::atomic<bool> stop { false };
    std::thread solver(solver_loop, std::cref(latest), std::cref(stop));

    std::mutex mtx;
    std::condition_variable cv;
    std::optional<Frame> slot; // 容量为 1 的帧槽
    bool camera_done = false;

    const auto t0 = Clock::now();
    std::thread camera([&] {
        for (int i = 0; i < kFrames; ++i) {
            std::this_thread::sleep_until(t0 + i * kPeriod);
            std::lock_guard lk(mtx);
            slot = Frame { i, Clock::now() }; // 旧帧还没被取走时直接覆盖
            cv.notify_one();
        }
        std::lock_guard lk(mtx);
        camera_done = true;
        cv.notify_one();
    });

    std::thread worker([&] {
        while (true) {
            Frame f;
            {
                std::unique_lock lk(mtx);
                cv.wait(lk, [&] { return slot || camera_done; });
                if (!slot)
                    break;
                f = *slot;
                slot.reset();
            }
            Detection d = detect(f);
            latest.write(tracker.track(d));
        }
    });

    camera.join();
    worker.join();
    stop = true;
    solver.join();
    std::printf(
        "order_errors %d  detected %d/%d\n",
        tracker.order_errors(),
        tracker.detected_count(),
        kFrames
    );
}
