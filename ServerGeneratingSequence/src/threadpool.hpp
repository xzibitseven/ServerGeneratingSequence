#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include "jointhreads.hpp"
#include "lockfreequeue.hpp"

/*
 *  T - type handler
 */
template <typename T>
class ThreadPool {
    public:
        explicit ThreadPool():
            m_done(false),
            m_joiner(m_threads) {
            const std::size_t threadCount{std::thread::hardware_concurrency()};
            try {
                if(threadCount>=2)
                    m_threads.emplace_back(std::thread(&ThreadPool::workerThread, this));
            } catch(...) {
                m_done = true;
                throw;
            }
        }
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
        ThreadPool& operator=(ThreadPool&&) = delete;
        ~ThreadPool() {
            m_done = true;
            m_condition.notify_all();
        }

        void submit(T f) {
            m_workQueue.push(std::move(f));
            m_condition.notify_one();
        }
    private:
        /* Function does the work for each handler in a separate thread. */
        void workerThread() {
            while(!m_done) {
                {
                    std::unique_lock<std::mutex> lock(m_mtx);
                    m_condition.wait(lock,[this](){ return (!m_workQueue.empty() || m_done);});
                }
                std::unique_ptr<T> upHandler = m_workQueue.pop();
                if(upHandler)
                    (*upHandler)();
            }
        }

    private:
        std::atomic_bool m_done;
        std::mutex m_mtx;
        std::condition_variable m_condition;
        LockFreeQueue<T> m_workQueue;
        std::vector<std::thread> m_threads;
        JoinThreads m_joiner;
};

#endif // THREADPOOL_HPP

