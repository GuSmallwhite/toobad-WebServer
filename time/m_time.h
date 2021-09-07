#ifndef M_TIME_H
#define M_TIME_H

#include <time.h>

class t_client;
//用户数据结构：客户端socket地址，socket文件描述符，计时器
class client_data
{
    public:
        sockaddr_in address;
        int sockfd;
        t_client* timer;
};
//定时器类
class t_client
{
    public:
        t_client() : pre(nullptr), next(nullptr){};
    public:
        time_t livetime;//任务的超时时间
        void (*cb_func)(client_data *);//任务回调函数
        client_data *user_data;
        t_client *pre;//指向前一个定时器
        t_client *next;//指向下一个定时器
};
//定时器链表
class t_client_list
{
    private:
        void add_timer(t_client *timer, t_client *lst_head);
    
    public:
        t_client_list() : head(nullptr), tail(nullptr){};
        ~t_client_list();
        void add_timer(t_client *timer);//添加定时器
        void adjust_timer(t_client *timer);//被add调用
        void del_timer(t_client *timer);//删除定时器
        void tick();//清空非活跃链接
        t_client *remove_from_list(t_client *timer);
    public:
        t_client *head;
        t_client *tail;
};
t_client_list::~t_client_list()
{
    t_client *temp = head;
    while (temp)
    {
        head = temp->next;
        delete temp;
        temp = head;
    }
}

void t_client_list::tick()
{
    if(!head)
    {
        return;
    }
    time_t cur=time(NULL);//获得系统当时时间
    t_client* temp=head;//从头结点开始处理每个定时器，直至遇到一个尚未到期的定时器
    while(temp)
    {
        if(cur<temp->livetime)
        {
            break;
        }
        else
        {
            temp->cb_func(temp->user_data);
        }
        head=temp->next;
        if(head)
            head->pre=NULL;//执行完定时器中的定时任务后就删除并重置头结点
        delete temp;
        temp=head;
    }
}

void t_client_list::add_timer(t_client* timer)
{
    if(!timer)
    {
        cout<<"not a timer"<<endl;
        return;
    }
    if(!head)
    {
        head=timer;
        tail=timer;
        return;
    }
    if(timer->livetime<head->livetime)
    {
        timer->next=head;
        head->pre=timer;
        head=timer;
        return;
    }
    else
    {
        t_client* temp=head;
        while (temp)
        {
            if(temp->livetime>timer->livetime)
            {
                t_client *temppre=timer->pre;
                timer->pre=temp->pre;
                timer->next=temp;
                temppre->next = timer;
                temp->pre = timer;
                return;
            }
            temp=temp->next;
        }
        tail->next = timer;
        timer->pre = tail;
        tail = timer;
        return;     
    }
}

t_client *t_client_list::remove_from_list(t_client *timer)//将timer从链表中移除
{
    if(!timer)
        return timer;
    if((timer==head)&&(timer==tail))//目标只有一个定时器
    {
       
        head=tail=nullptr;
    }
    else if(timer==head)//链表至少有两个定时器，且目标定时器为头结点，则将头结点重置为原下一个节点，然后删除目标定时器
    {
        head = head->next;
        head->pre = nullptr;
        timer->next = nullptr;
    }
    else if(timer==tail)
    {
        tail = tail->pre;
        tail->next = nullptr;
        timer->pre = nullptr;
    }
    else
    {
        timer->pre->next = timer->next;
        timer->next->pre = timer->pre;
        timer->pre = nullptr;
        timer->next = nullptr;
    }
    return timer;
}

void t_client_list::del_timer(t_client *timer) //对于列表中的时间事件删除,参数中的timer对应的描述符已经关闭了
{
    if (!timer)
        return;
    remove_from_list(timer);
    delete timer;
}

void t_client_list::adjust_timer(t_client *timer) //对于列表中的时间事件进行按照时间顺序的调整再重新插入
{
    if (timer)
    {
        t_client *temp = remove_from_list(timer);
        //cout << "remove ok" << endl;
        add_timer(temp);
        //cout << "add ok" << endl;
    }

}

#endif
