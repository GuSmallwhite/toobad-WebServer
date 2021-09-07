#ifndef POOL_H
#define POOL_H

#include "../lock/lock.h"

#include <iostream>
#include <list>
#include <pthread.h>
#include <vector>
using namespace std;

template<typename T>
class pool
{
    private:
        list<T*>request_list;//请求队列

        vector<pthread_t> thread_list;//线程池数组
        locker m_lock;//互斥锁
        sem m_sem;//是否有任务要处理
        int m_thread_num;//最大线程数
        int m_max_request;//最大请求数
    private:
        static void *worker(void *arg); //工作线程的执行函数，因此是静态函数，对象通过参数传入
        void run();
    public: //三个函数
        pool(int thread_num = 10, int max_request = 100);
        ~pool();
        bool append(T *requset);//向请求队列添加任务
};

template <typename T>
pool<T>::pool(int thread_num, int max_request) : m_thread_num(thread_num), m_max_request(max_request)
{
    thread_list = vector<pthread_t>(thread_num, 0);
    for (int i = 0; i < m_thread_num; ++i)
    {
        if (pthread_create(&thread_list[i], NULL, worker, this) != 0)
        {
            cout << "some thing wrong!" << endl;
        }
        if (pthread_detach(thread_list[i])!= 0)
        {
            cout << "detach failed!" << endl;
        }
    }
}

template <typename T>
pool<T>::~pool()
{
    ;
}

template <typename T>
bool pool<T>::append(T *requset)
{
    m_lock.dolock();//工作队列被所有线程共享，所以必须加锁
    if (request_list.size() > m_max_request)
    {
        m_lock.unlock();
        return false;
    }
    request_list.push_back(requset);
    m_lock.unlock();
    m_sem.post();//有任务需要处理
    return true;
}

template <typename T>
void* pool<T>::worker(void *arg)
{
    pool<T> *pool_ptr = static_cast<pool<T> *>(arg);
    pool_ptr->run();
    return pool_ptr;
}

template <typename T>
void pool<T>::run()
{
    while(1)
    {
        m_sem.wait();    //-1
        m_lock.dolock(); //加锁,竞争条件
        if (request_list.size() <= 0)
        {
            m_lock.unlock();
        }
        else{
            T* request = request_list.front();
            request_list.pop_front(); //请求队列出队
            m_lock.unlock();
            if (!request)
            {
                continue;
            }
            request->process();
        }
    }
}

#endif