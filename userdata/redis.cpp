#include "redis.h"
#include <map>

redis_clt *redis_clt::m_redis_instance = new redis_clt();

redis_clt *redis_clt::getinstance()
{
    return m_redis_instance;
}

string redis_clt::getReply(string m_command)
{
    m_redis_lock.dolock();
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    //cout << m_redisReply->type << endl;
    string temp = "";
    if (m_redisReply->elements == 0 && m_redisReply->type == 1)
    {
        //»Ø¸´Ò»ÐÐ
        if (m_redisReply->len > 0)
        {
            temp = string(m_redisReply->str);
            //cout << "reply:   " << temp << endl;
        }
        else
            cout << "return nothing?" << endl;
    }
    else if (m_redisReply->type == 3)
    {
        //cout << "do code" << endl;
        int tempcode = m_redisReply->integer;
        temp = to_string(tempcode);
    }
    freeReplyObject(m_redisReply);
    m_redis_lock.unlock();
    return temp;
}

redis_clt::redis_clt()
{
    timeout = {2, 0};
    m_redisContext = (redisContext *)redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    m_redisReply = nullptr;
}



