#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include "../lock/lock.h"
#include "../userdata/redis.h"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

class http_conn
{
    public:
        static const int BUFF_READ_SIZE = 2048;//����������С
        static const int BUFF_WRITE_SIZE = 2048;//д��������С
        enum METHOD //�������󷽷�
        {
            GET = 0,
            POST
        };
        enum MAIN_STATE //  ��״̬��״̬�����ڷ��������У����ڷ���ͷ�������ڷ�������
        {
            REQUESTLINE = 0,
            HEADER,
            CONTENT
        };
        enum LINE_STATE // ��״̬��״̬�����еĶ�ȡ״̬�������������в�����
        {
            LINE_OK = 0,
            LINE_BAD,
            LINE_OPEN
        };
        enum HTTP_CODE//����������http������������������������get��������post������Դ���ͻ����ѹر�����
        {
            NO_REQUEST,
            GET_REQUEST,
            POST_REQUEST,
            NO_RESOURCE,
            CLOSED_CONNECTION
        };
    public:
        http_conn(){};
        ~http_conn(){};
    public:
        void init(int socketfd, const sockaddr_in &addr); //��ʼ���׽���
        void init();                                      //ʵ�־����������ֵ�ĳ�ʼ��
        void close_conn(string msg = "");                 //�ر�����
        void process();                                   //��������
        bool read();                                      //һ���Ե���recv��ȡ�������ݣ���ȡ�����������ȫ�����ݣ�������������,���ص����Ƿ�Ƴɹ�����Ϣ
        bool write();                                     //������д
    public:
        static int m_epollfd; //����socket�ϵ��¼���ע�ᵽͬһ��epoll�ں�ʱ�����
        static int m_user_count;//�û�����
    private:
        HTTP_CODE process_read();          //�Ӷ���������ȡ�������ݽ��н���
        bool process_write(HTTP_CODE ret); //д����Ӧ��д��������
        void parser_header(const string &text, map<string, string> &m_map);      //�������������
        void parser_requestline(const string &text, map<string, string> &m_map); //��������ĵ�һ��
        void parser_postinfo(const string &text, map<string, string> &m_map);    //����post��������
        bool do_request(); //ȷ���������������һ��ҳ��
        void unmap();
    private:
        locker m_redis_lock;
        int m_socket; //��ǰ�������http���ӵ��׽���
        sockaddr_in m_addr;

        struct stat m_file_stat;
        struct iovec m_iovec[2];
        int m_iovec_length;
        string filename;
        string postmsg;
        char *file_addr;
        char post_temp[];
        char read_buff[BUFF_READ_SIZE];   //ÿ��http���Ӷ���һ������������д������
        char write_buff[BUFF_WRITE_SIZE]; //ÿ��http���Ӷ���һ������������д������
        int read_for_now = 0;
        int write_for_now = 0;

        map<string, string> m_map; //http���ӵĸ�������
};
#endif