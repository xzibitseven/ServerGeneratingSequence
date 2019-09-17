#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>

/*
 * queue with blocks
 */

template<typename T>
class  ThreadSafeQueue{
    private:
        struct Node{
            std::shared_ptr<T> m_spData;
            std::unique_ptr<Node> m_upNext;
        };

        std::mutex m_headMutex;
        std::unique_ptr<Node> m_upHead;
        std::mutex m_tailMutex;
        Node* m_pTail;
        std::condition_variable m_dataCond;

        Node* getTail(){
            std::lock_guard<std::mutex> tailLock(m_tailMutex);
            return m_pTail;
        }

        std::unique_ptr<Node> popHead(){
            std::unique_ptr<Node> oldHead = std::move(m_upHead);
            m_upHead = std::move(oldHead->m_upNext);
            return oldHead;
        }
        std::unique_lock<std::mutex> waitForData(){
            std::unique_lock<std::mutex> headLock(m_headMutex);
            m_dataCond.wait(headLock, [&]{return m_upHead.get()!=getTail();});
            return std::move(headLock);
        }
        std::unique_ptr<Node> waitPopHead(){
            std::unique_lock<std::mutex> headLock(waitForData());
            return popHead();
        }

        std::unique_ptr<Node> waitPopHead(T& value){
            std::unique_lock<std::mutex> headLock(waitForData());
            value = std::move(*m_upHead->m_spData);
            return popHead();
        }


        std::unique_ptr<Node> tryPopHead(){
            std::lock_guard<std::mutex> headLock(m_headMutex);
            if(m_upHead.get() == getTail()){
                return std::unique_ptr<Node>();
            }
            return popHead();
        }
        std::unique_ptr<Node> tryPopHead(T& value){
            std::lock_guard<std::mutex> headLock(m_headMutex);
            if(m_upHead.get() == getTail()){
                return std::unique_ptr<Node>();
            }
            value = std::move(*m_upHead->m_spData);
            return popHead();
        }

    public:
        ThreadSafeQueue():
            m_upHead(new Node),
            m_pTail(m_upHead.get()){}
        ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;

        std::shared_ptr<T> waitAndPop(){
            std::unique_ptr<Node> const oldHead = waitPopHead();
            return oldHead->m_spData;
        }
        std::shared_ptr<T> waitAndPop(T& value){
            std::unique_ptr<Node> const oldHead = waitPopHead(value);
        }

        std::shared_ptr<T> tryPop(){
            std::unique_ptr<Node>  oldHead = tryPopHead();
            return oldHead ? oldHead->m_spData : std::shared_ptr<T>();
        }
        bool tryPop(T& value){
            std::unique_ptr<Node> const oldHead = tryPopHead(value);
            return bool(oldHead);
        }
        bool empty() {
            std::lock_guard<std::mutex> headLock(m_headMutex);
            return (m_upHead.get() == getTail());
        }
        void push(T newValue){
            std::shared_ptr<T> newData(std::make_shared<T>(std::move(newValue)));
            std::unique_ptr<Node> p(new Node);
            {
                std::lock_guard<std::mutex> tailLock(m_tailMutex);
                m_pTail->m_spData = newData;
                Node* const newTail = p.get();
                m_pTail->m_upNext = std::move(p);
                m_pTail = newTail;
            }
            m_dataCond.notify_one();
        }
};


#endif // THREADSAFEQUEUE_HPP
