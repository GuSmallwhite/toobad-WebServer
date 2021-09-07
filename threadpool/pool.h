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
        list<T*>request_list;//�������

        vector<pthread_t> thread_list;//�̳߳�����
        locker m_lock;//������
        sem m_sem;//�Ƿ�������Ҫ����
        int m_thread_num;//����߳���
        int m_max_request;//���������
    private:
        static void *worker(void *arg); //�����̵߳�ִ�к���������Ǿ�̬����������ͨ����������
        void run();
    public: //��������
        pool(int thread_num = 10, int max_request = 100);
        ~pool();
        bool append(T *requset);//����������������
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
    m_lock.dolock();//�������б������̹߳������Ա������
    if (request_list.size() > m_max_request)
    {
        m_lock.unlock();
        return false;
    }
    request_list.push_back(requset);
    m_lock.unlock();
    m_sem.post();//��������Ҫ����
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
        m_lock.dolock(); //����,��������
        if (request_list.size() <= 0)
        {
            m_lock.unlock();
        }
        else{
            T* request = request_list.front();
            request_list.pop_front(); //������г���
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