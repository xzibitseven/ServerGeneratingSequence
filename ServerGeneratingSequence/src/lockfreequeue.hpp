#ifndef LOCKFREEQUEUE_HPP
#define LOCKFREEQUEUE_HPP

#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <iostream>

template<typename T>
class LockFreeQueue {
    private:
        struct Node;

        struct counted_node_ptr {
            std::uint64_t external_count = 0;
            Node* ptr = nullptr;
        };

        std::atomic<counted_node_ptr> m_head;
        std::atomic<counted_node_ptr> m_tail;

        struct node_counter {
            std::uint32_t internal_count:30;
            std::uint32_t external_counters:2;
        };

        struct Node{
            std::atomic<T*> m_data;
            std::atomic<node_counter> m_count;
            std::atomic<counted_node_ptr> m_next;

            Node():
                m_data(nullptr) {

                node_counter new_count;
                new_count.internal_count = 0;
                new_count.external_counters = 2;
                m_count.store(new_count);

                counted_node_ptr new_next;
                new_next.ptr = nullptr;
                new_next.external_count = 0;
                m_next.store(new_next);
            }

            void release_ref() {
                node_counter old_counter = m_count.load(std::memory_order_relaxed);
                node_counter new_counter;
                do{
                    new_counter = old_counter;
                    --new_counter.internal_count;
                }
                while(!m_count.compare_exchange_strong(old_counter, new_counter,
                                                   std::memory_order_acquire,std::memory_order_relaxed));

                if(!new_counter.internal_count && !new_counter.external_counters){
                    delete this;
                }
            }
        };

    public:
        LockFreeQueue(){
            counted_node_ptr newHead;
            newHead.ptr = new Node;
            m_head.store(newHead);
            m_tail = m_head.load();
        }
        LockFreeQueue(const LockFreeQueue&) = delete;
        LockFreeQueue& operator=(const LockFreeQueue&) = delete;
        ~LockFreeQueue() {
            counted_node_ptr old_head = m_head.load();
            while(old_head.ptr) {
                old_head.ptr->m_next.load();
                m_head.store(old_head.ptr->m_next.load());
                delete old_head.ptr;
                old_head = m_head.load();
            }
        }

        std::unique_ptr<T> pop(){

            counted_node_ptr old_head = m_head.load(std::memory_order_relaxed);
            for(;;) {
                increase_external_count(m_head, old_head);
                Node* const ptr = old_head.ptr;
                if(ptr == m_tail.load().ptr)
                    return std::unique_ptr<T>();

                counted_node_ptr next = ptr->m_next.load();
                if(m_head.compare_exchange_strong(old_head, next)){
                    T* const res = ptr->m_data.exchange(nullptr);
                    free_external_counter(old_head);
                    return std::unique_ptr<T>(res);
                }
                ptr->release_ref();
            }
        }

        void push(T newValue) {

            std::unique_ptr<T> newData(new T(newValue));
            counted_node_ptr new_next;
            new_next.ptr = new Node;
            new_next.external_count = 1;
            counted_node_ptr oldTail = m_tail.load();

            for(;;) {
                increase_external_count(m_tail, oldTail);
                T* oldData  = nullptr;
                if(oldTail.ptr->m_data.compare_exchange_strong(oldData, newData.get())) {
                   counted_node_ptr old_next;// = {0};
                   if(!oldTail.ptr->m_next.compare_exchange_strong(old_next, new_next)) {
                       delete new_next.ptr;
                       new_next = old_next;
                   }

                   set_new_tail(oldTail, new_next);
                   newData.release();
                   break;
                }else{
                    counted_node_ptr old_next;// = {0};
                    if(oldTail.ptr->m_next.compare_exchange_strong(old_next, new_next)){
                        old_next = new_next;
                        new_next.ptr = new Node;
                    }
                    set_new_tail(oldTail, old_next);
                }
            }
        }

        bool empty() {
            counted_node_ptr old_head = m_head.load(std::memory_order_relaxed);
            increase_external_count(m_head, old_head);
            Node* const ptr = old_head.ptr;
            return (ptr == m_tail.load().ptr);
        }


    private:
        static void increase_external_count(std::atomic<counted_node_ptr>& counter,
                                            counted_node_ptr& old_counter){
            counted_node_ptr new_counter;
            do{
                new_counter = old_counter;
                ++new_counter.external_count;
            }
            while(!counter.compare_exchange_strong(old_counter,new_counter,
                                                   std::memory_order_acquire, std::memory_order_relaxed));
            old_counter.external_count = new_counter.external_count;

        }

        static void free_external_counter(counted_node_ptr& old_node_ptr){
            Node* const ptr = old_node_ptr.ptr;
            int const count_increase = old_node_ptr.external_count-2;
            node_counter old_counter = ptr->m_count.load(std::memory_order_relaxed);
            node_counter new_counter;

            do{
                new_counter = old_counter;
                --new_counter.external_counters;
                new_counter.internal_count += count_increase;
            }
            while(!ptr->m_count.compare_exchange_strong(old_counter,new_counter,
                                                        std::memory_order_acquire, std::memory_order_relaxed));

            if(!new_counter.internal_count && !new_counter.external_counters){
                delete ptr;
            }

        }

        void set_new_tail(counted_node_ptr& old_tail, counted_node_ptr const& new_tail){
            Node* const current_tail_ptr = old_tail.ptr;
            while(!m_tail.compare_exchange_weak(old_tail, new_tail) && old_tail.ptr == current_tail_ptr);


            if(old_tail.ptr==current_tail_ptr)
                free_external_counter(old_tail);
            else
                current_tail_ptr->release_ref();
        }
};

#endif // LOCKFREEQUEUE_HPP
