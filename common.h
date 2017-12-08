/************************************************************************
//Describe: common.h: header file for communicaiton.cpp and control.cpp
//Author: FrankHXW@gmail.com
//Version: v1.0.0
//Date:   11/14/2016@THU
//Copyright(c) 2015~2016 FrankHXW,All rights reserved.
************************************************************************/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <map>
#include <deque>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "stdint.h"
#include "pthread.h"
#include "omp.h"
#include "math.h"
#include "limits.h"
#include "colormod.h"
#include <syslog.h>


using namespace std;

///sleep and time
inline void SleepUs(uint32_t _us)
{
    usleep(_us);
}


inline void SleepMs(uint32_t _ms)
{
    usleep(_ms*1000);
}


inline long GetCurrentTimeMs(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

inline string GetCurrentTimeTiff(void)
{
    struct tm *t;
    time_t tt;
    time(&tt);
    t = localtime(&tt);
    char temp[100];
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "%4d-%02d-%02d %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    //printf("%4d年%02d月%02d日 %02d:%02d:%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    string CurrentTimeTiff(temp);
    return CurrentTimeTiff;
}

///递归新建多级文件夹
#if defined(_MSC_VER)
#include <direct.h>
#include <io.h>
#define ACCESS _access
#define MKDIR(a) _mkdir((a))
#elif defined(__GNUC__)
#include <stdarg.h>
#include <sys/stat.h>
#define ACCESS access
#define MKDIR(a) mkdir((a),0755)
#endif
inline bool CreatDirectory(const char *_path)
{
    int32_t length = int32_t(strlen(_path));
    char *path = new char[length + 2];
    memcpy(path, _path, length);
    //在路径末尾加/
    if (path[length - 1] != '\\' && path[length - 1] != '/')
    {
        path[length] = '/';
        path[length + 1] = '\0';
    }
    //创建目录
    for (int32_t i = 0; i <= length; ++i)
    {
        if (path[i] == '\\' || (i!=0&&path[i] == '/'))
        {
            path[i] = '\0';
            //如果不存在,创建
            int32_t error = ACCESS(path, 0);
            if (error != 0)
            {
                error = MKDIR(path);
                if (error != 0)
                {
                    cout<<"make dir failed @"<<path<<endl;
                    return false;
                }
            }
            //支持linux,将所有\换成/
            path[i] = '/';
        }
    }
    delete[]path;
    return true;
}



///p_thread封装类
class PThreadClass
{
public:
    PThreadClass()
    {
        /* empty */
    }
    virtual ~PThreadClass()
    {
        /* empty */
    }

    /** Returns true if the thread was successfully started, false if there was an error starting the thread */
    bool StartInternalThread()
    {
        return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
    }

    /** Will not return until the internal thread has exited. */
    void WaitForInternalThreadToExit()
    {
        (void) pthread_join(_thread, NULL);
    }

protected:
    /** Implement this method in your subclass with the code you want your thread to run. */
    virtual int Run(void) = 0;

private:
    static void * InternalThreadEntryFunc(void * This)
    {
        ((PThreadClass *)This)->Run();
        return NULL;
    }

    pthread_t _thread;
};






///pthread_mutex封装类
class PMutex
{
public:
    PMutex()
    {
        pthread_mutex_init( &mMutex, NULL );
    }
    ~PMutex()
    {
        pthread_mutex_destroy( &mMutex );
    }

    void Lock()
    {
        pthread_mutex_lock( &mMutex );
    }
    void Unlock()
    {
        pthread_mutex_unlock( &mMutex );
    }

private:
    pthread_mutex_t  mMutex;
};


///线程消息队列模板定义，使用该模板的类必须重载两个"=="操作符
template <class T>
class ThreadMessageDeque
{
public:
    ThreadMessageDeque() {}
    ~ThreadMessageDeque() {};

    ///消息队列尾部插入
    void PushBack(T* _unit)
    {
        mutex_.Lock();
        deque_.push_back(_unit);
        mutex_.Unlock();
    }

    ///弹出消息队列首部
    T* PopFront(void)
    {
        mutex_.Lock();
        T* tmp=deque_.front();
        deque_.pop_front();
        mutex_.Unlock();
        return tmp;
    }

    ///判断消息队列是否空
    bool Empty(void)
    {
        mutex_.Lock();
        bool tmp=deque_.empty();
        mutex_.Unlock();
        return tmp;
    }

    ///从消息队列删除完全相等的消息
    void Erase(T *_unit)
    {
        mutex_.Lock();
        for(auto itr=deque_.begin(); itr!=deque_.end();)
        {
            if(*(*itr)==*_unit)
            {
                itr= deque_.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
        mutex_.Unlock();
    }

    ///从消息队列删除请求者的所有消息
    void Erase(string &_requestor)
    {
        mutex_.Lock();
        for(auto itr=deque_.begin(); itr!=deque_.end();)
        {
            if(*(*itr)==_requestor)
            {
                itr= deque_.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
        mutex_.Unlock();
    }

private:
    ///相机控制线程消息队列
    std::deque<T *> deque_;

    ///相机控制线程消息队列同步锁
    PMutex mutex_;
};







/************************************************************************************************************************************************/



















