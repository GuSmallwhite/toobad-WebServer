#ifndef REDIS_H
#define REDIS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <string.h>
#include "../lock/lock.h"

using namespace std;

class redis_clt
{
private:
    locker m_redis_lock;
    static redis_clt *m_redis_instance;
    struct timeval timeout;
    redisContext *m_redisContext;
    redisReply *m_redisReply;

private:
    string getReply(string m_command);
    redis_clt();

public:
    string setUserpasswd(string username, string passwd)
    {
        return getReply("set " + username + " " + passwd);
    }

    string getUserpasswd(string username)
    {
        return getReply("get " + username);
    }


    static redis_clt *getinstance();
};

#endif
