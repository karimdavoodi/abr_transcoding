#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>


template <typename T>
class Queue {
    private:
        std::mutex mx;
        std::condition_variable cv_empty;
        std::condition_variable cv_full;
        std::queue<T*> q;
        int capacity;
    
    public:
        Queue(int capacity):capacity{capacity}{}
        bool empty() { return q.empty(); }
        bool full() { return (int)q.size() >= capacity; }
        void push(T* data);
        int size() { return q.size(); }
        T* pop();
        ~Queue(){}
};
template<typename T>
void Queue<T>::push(T* elem)
{
    if(elem == nullptr) 
        return;

    std::unique_lock<std::mutex> lck(mx);
    cv_full.wait(lck, [this](){ return !this->full(); });
    q.push(elem);
    cv_empty.notify_all();
}

template<typename T>
T* Queue<T>::pop()
{
    T* elem = nullptr;

    std::unique_lock<std::mutex> lck(mx);
    cv_empty.wait(lck, [this](){ return !this->empty(); });
    elem = q.front();
    q.pop();
    cv_full.notify_all();
    return elem;
}
