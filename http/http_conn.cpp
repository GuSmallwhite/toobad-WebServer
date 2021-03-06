#include "http_conn.h"

void setnonblocking(int socketfd)
{
    int old_opt = fcntl(socketfd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(socketfd, F_SETFL, new_opt);
}

void addfd(int epollfd, int socketfd)
{
    //把该描述符添加到epoll的事件表
    epoll_event m_event;                                   //新建一个事件
    m_event.data.fd = socketfd;                            //使得描述符为本描述符
    m_event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;       //监听输入、边缘触发、挂起
    epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &m_event); //将该事件添加
    setnonblocking(socketfd);                              //设置非阻塞
}

void removefd(int epollfd, int socketfd)
{
    epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, 0); //把该套接字的对应事件删除
    close(socketfd);
}

void modfd(int epollfd, int socketed, int ev)
{
    epoll_event m_event;
    m_event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    m_event.data.fd = socketed;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, socketed, &m_event);
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

void http_conn::init(int socketfd, const sockaddr_in &addr) //套接字的初始化，用于添加进epoll的事件表
{
    m_socket = socketfd;
    m_addr = addr;
    addfd(m_epollfd, m_socket);
    init();
}

void http_conn::init()
{
    filename = "";
    memset(read_buff, '\0', BUFF_READ_SIZE); //清空缓冲区
    memset(write_buff, '\0', BUFF_WRITE_SIZE);
    read_for_now = 0;
    write_for_now = 0;
}

void http_conn::close_conn(string msg)
{
    //将当前的描述符在epoll监听事件里面去除
    if (m_socket != -1)
    {
        removefd(m_epollfd, m_socket);
        m_user_count--;
        m_socket = -1; //-1就是代表没有正在连接的套接字
    }
}

void http_conn::process() //对请求进行处理
{
    //首先进行报文的解析
    HTTP_CODE ret = process_read();
    if (ret == NO_REQUEST)
    {
        modfd(m_epollfd, m_socket, EPOLLIN);
        return;
    }
    //然后进行报文的响应
    bool result = process_write(ret);
    modfd(m_epollfd, m_socket, EPOLLOUT); //最后向epoll的监听的事件表中添加可写事件
}

void http_conn::parser_requestline(const string &text, map<string, string> &m_map)//分析请求行
{
    string m_method = text.substr(0, text.find(" "));
    string m_url = text.substr(text.find_first_of(" ") + 1, text.find_last_of(" ") - text.find_first_of(" ") - 1);
    string m_protocol = text.substr(text.find_last_of(" ") + 1);
    m_map["method"] = m_method;
    m_map["url"] = m_url;
    m_map["protocol"] = m_protocol;
}

void http_conn::parser_header(const string &text, map<string, string> &m_map)//分析请求头部
{
    if (text.size() > 0)
    {
        if (text.find(": ") <= text.size())
        {
            string m_type = text.substr(0, text.find(": "));
            string m_content = text.substr(text.find(": ") + 2);
            m_map[m_type] = m_content;
        }
        else if (text.find("=") <= text.size())
        {
            string m_type = text.substr(0, text.find("="));
            string m_content = text.substr(text.find("=") + 1);
            m_map[m_type] = m_content;
        }
    }
}

void http_conn::parser_postinfo(const string &text, map<string, string> &m_map)//分析post请求
{
    //username=chaishilin&passwd=12345&votename=alibaba
    //cout << "post:   " << text << endl;
    string processd = "";
    string strleft = text;
    while (true)
    {
        processd = strleft.substr(0, strleft.find("&"));
        m_map[processd.substr(0, processd.find("="))] = processd.substr(processd.find("=") + 1);
        strleft = strleft.substr(strleft.find("&") + 1);
        if (strleft == processd)
            break;
    }
}

http_conn::HTTP_CODE http_conn::process_read()
{
    string m_head = "";
    string m_left = read_buff; //把读入缓冲区的数据转化为string
    int flag = 0;
    int do_post_flag = 0;
    while(true)
    {
        m_head = m_left.substr(0, m_left.find("\r\n"));
        m_left = m_left.substr(m_left.find("\r\n") + 2);
        if (flag == 0)
        {
            flag = 1;
            parser_requestline(m_head, m_map);
        }
        else if (do_post_flag)
        {
            parser_postinfo(m_head, m_map);
            break;
        }
        else
        {
            parser_header(m_head, m_map);
        }
        if (m_head == "")
            do_post_flag = 1;//post无请求头部
        if (m_left == "")
            break;
    }
    if (m_map["method"] == "POST")
    {
        //cout << "request" << read_buff << endl;
        return POST_REQUEST;
    }
    else if (m_map["method"] == "GET")
    {
        return GET_REQUEST;
    }
    else
    {
        return NO_REQUEST;
    }
}

bool http_conn::do_request()//确认到底请求的是哪个网页
{
    if (m_map["method"] == "POST")
    {
        redis_clt *m_redis = redis_clt::getinstance();
        if (m_map["url"] == "/base.html" || m_map["url"] == "/") //如果来自于登录界面
        {
            if (m_redis->getUserpasswd(m_map["username"]) == m_map["passwd"])
            {
                if (m_redis->getUserpasswd(m_map["username"]) == "root")               
                    filename = "./root/welcomeroot.html"; //登录进入欢迎界面
                else
                    filename = "./root/welcome.html"; //登录进入欢迎界面
            }
            else
                filename = "./root/error.html"; //进入登录失败界面         
        }
        else if(m_map["url"] == "/regester.html") //如果来自注册界面
        {
            m_redis->setUserpasswd(m_map["username"], m_map["passwd"]);
            filename = "./root/regester.html"; //注册后进入初始登录界面
        }
        
        else 
            filename = "./root/base.html"; //进入初始登录界面
    }
    else if (m_map["method"] == "GET")
    {
        if (m_map["url"] == "/")
        {
            m_map["url"] = "/base.html";
        }
        filename = "./root" + m_map["url"];
    }
    else
    {
        filename = "./root/error.html";
    }
    return true;
}

void http_conn::unmap()
{
    if (file_addr)
    {
        munmap(file_addr, m_file_stat.st_size);
        file_addr = 0;
    }
}

bool http_conn::process_write(HTTP_CODE ret)
{
    if (do_request())
    {
        //filename
        int fd = open(filename.c_str(), O_RDONLY);

        stat(filename.c_str(), &m_file_stat);
        file_addr = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        m_iovec[1].iov_base = file_addr;
        m_iovec[1].iov_len = m_file_stat.st_size;
        m_iovec_length = 2;
        close(fd); 
    }
    else
    {
        if (postmsg != "")
        {
            if(postmsg.length() < 20)
                cout <<"wrong pstmsg : "<< postmsg << endl;
            strcpy(post_temp, postmsg.c_str());
            //cout <<postmsg.size()<<" :" << post_temp << endl;
            m_iovec[1].iov_base = post_temp;
            m_iovec[1].iov_len = (postmsg.size()) * sizeof(char);
            m_iovec_length = 2;
        }
        else
        {
            //cout << "get pstmsg : " << postmsg << endl;
            m_iovec_length = 1;
        }

    }

    return true;
}

bool http_conn::read() //把socket的东西全部读到读缓冲区里面
{
    if (read_for_now > BUFF_READ_SIZE) //如果当前可以写入读缓冲区的位置已经超出了缓冲区长度了
    {
        cout << "read error at beigin" << endl;
        return false;
    }
    int bytes_read = 0;
    while(true)
    {
        bytes_read = recv(m_socket, read_buff + read_for_now, BUFF_READ_SIZE - read_for_now, 0);
        if (bytes_read == -1) //出现错误
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) 
            {
                break;
            }
            cout << "bytes_read == -1" << endl;
            return false;
        }
        else if (bytes_read == 0) 
        {
            cout << "bytes_read == 0" << endl;
            return false; 
            continue;
        }
        read_for_now += bytes_read;
    }
    return true;
}

bool http_conn::write() //将响应内容写到写缓冲区中
{
    int bytes_write = 0;
    string response_head = "HTTP/1.1 200 OK\r\n\r\n";
    char head_temp[response_head.size()];
    strcpy(head_temp, response_head.c_str());
    m_iovec[0].iov_base = head_temp;
    m_iovec[0].iov_len = response_head.size() * sizeof(char);
    bytes_write = writev(m_socket, m_iovec, m_iovec_length);
    
    if (bytes_write <= 0)
    {
        return false;
    }

    unmap();
     if (m_map["Connection"] == "keep-alive")
    {
        return false;
    }
    else
    {
        return false;
    }
}
