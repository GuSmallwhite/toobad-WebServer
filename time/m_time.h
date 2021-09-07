#ifndef M_TIME_H
#define M_TIME_H

#include <time.h>

class t_client;
//�û����ݽṹ���ͻ���socket��ַ��socket�ļ�����������ʱ��
class client_data
{
    public:
        sockaddr_in address;
        int sockfd;
        t_client* timer;
};
//��ʱ����
class t_client
{
    public:
        t_client() : pre(nullptr), next(nullptr){};
    public:
        time_t livetime;//����ĳ�ʱʱ��
        void (*cb_func)(client_data *);//����ص�����
        client_data *user_data;
        t_client *pre;//ָ��ǰһ����ʱ��
        t_client *next;//ָ����һ����ʱ��
};
//��ʱ������
class t_client_list
{
    private:
        void add_timer(t_client *timer, t_client *lst_head);
    
    public:
        t_client_list() : head(nullptr), tail(nullptr){};
        ~t_client_list();
        void add_timer(t_client *timer);//��Ӷ�ʱ��
        void adjust_timer(t_client *timer);//��add����
        void del_timer(t_client *timer);//ɾ����ʱ��
        void tick();//��շǻ�Ծ����
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
    time_t cur=time(NULL);//���ϵͳ��ʱʱ��
    t_client* temp=head;//��ͷ��㿪ʼ����ÿ����ʱ����ֱ������һ����δ���ڵĶ�ʱ��
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
            head->pre=NULL;//ִ���궨ʱ���еĶ�ʱ������ɾ��������ͷ���
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

t_client *t_client_list::remove_from_list(t_client *timer)//��timer���������Ƴ�
{
    if(!timer)
        return timer;
    if((timer==head)&&(timer==tail))//Ŀ��ֻ��һ����ʱ��
    {
       
        head=tail=nullptr;
    }
    else if(timer==head)//����������������ʱ������Ŀ�궨ʱ��Ϊͷ��㣬��ͷ�������Ϊԭ��һ���ڵ㣬Ȼ��ɾ��Ŀ�궨ʱ��
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

void t_client_list::del_timer(t_client *timer) //�����б��е�ʱ���¼�ɾ��,�����е�timer��Ӧ���������Ѿ��ر���
{
    if (!timer)
        return;
    remove_from_list(timer);
    delete timer;
}

void t_client_list::adjust_timer(t_client *timer) //�����б��е�ʱ���¼����а���ʱ��˳��ĵ��������²���
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
