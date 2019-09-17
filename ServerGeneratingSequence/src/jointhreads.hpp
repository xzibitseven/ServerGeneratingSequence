#ifndef JOINTHREADS_HPP
#define JOINTHREADS_HPP

#include <thread>
#include <vector>

/*
 * Class JoinThreads is a wrapper around the thread vector, to safely and correctly terminate threads.
 */

class JoinThreads {
    public:
        explicit JoinThreads(std::vector<std::thread>& threads):
                    m_threads(threads){}
        JoinThreads(const JoinThreads&) = delete;
        JoinThreads(JoinThreads&&) = delete;
        JoinThreads& operator=(const JoinThreads&) = delete;
        JoinThreads& operator=(JoinThreads&&) = delete;
        ~JoinThreads(){
            for(auto& th : m_threads){
                if(th.joinable())
                    th.join();
            }
        }
    private:
        std::vector<std::thread>& m_threads;
};

#endif // JOINTHREADS_HPP
