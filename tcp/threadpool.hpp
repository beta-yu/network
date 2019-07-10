#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <iostream>
#include <string>
#include <queue>
#include <pthread.h>
#include <unistd.h>
using namespace std;

typedef void *(*hander) (int, string);

class Task
{
public:
    int _sock;
    string _ipaddr;
    hander _func;
public:
    Task(int sock, string ipaddr, hander func)
        :_sock(sock)
        ,_ipaddr(ipaddr)
        ,_func(func)
    {}
    void Run()
    {
        _func(_sock, _ipaddr);
    }
};

class ThreadPool
{
private:
    queue<Task> task_queue; //任务队列
    size_t thread_count;
    pthread_mutex_t mutex; //对任务队列的操作需保证线程安全
    pthread_cond_t cond; //任务队列为空时阻塞线程，加入新任务时唤醒线程。
private:
    void LockQueue()
    {
        pthread_mutex_lock(&mutex);
    }
    void UnLockQueue()
    {
        pthread_mutex_unlock(&mutex);
    }
    void BlockThread()
    {
        pthread_cond_wait(&cond, &mutex);
    }
    void SignalThread()
    {
        pthread_cond_signal(&cond);
    }
    Task GetTask()
    {
        Task t = task_queue.front();
        task_queue.pop();
        return t;
    }

public:
    ThreadPool(size_t count = 3)
        :thread_count(count)
    {}
    static void *run_func(void *arg) //static函数没有参数this
    {
		pthread_detach(pthread_self()); //线程分离
        ThreadPool *tp = (ThreadPool *)arg; //不能用this指针，使用参数传入一个对象指针，用来访问成员方法
        while(1) //每个线程往复的拿任务，执行任务
        {
            tp->LockQueue();
            while(tp->task_queue.empty())
            {
                tp->BlockThread(); //当前没有任务，线程被阻塞
            }
            //poptaskqueue
            Task t = tp->GetTask();
            //unlockmutex
            tp->UnLockQueue();
            //runtask
            t.Run(); //在锁外执行任务
        }
    }
    void InitThreadPool()
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
        pthread_t tid;
        while(thread_count--)
        {
            pthread_create(&tid, NULL, run_func, (void *)this);
        }
    }
    void PutTask(const Task& t)
    {
        LockQueue();
        task_queue.push(t);
        UnLockQueue();
        SignalThread(); //当有新任务时，唤醒可能被阻塞的线程
    }
    ~ThreadPool()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }
};

class Singleton //单例
{
private:
    static ThreadPool *tp;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
public:
    ThreadPool *GetInstance()
    {
        if(tp == NULL)
        {
            pthread_mutex_lock(&mutex); //为null时再加锁，避免当已经存在对象时仍需要等待获得锁
            if(tp == NULL) //再次判断可避免在第一次判断和获得锁之间，有其他线程先一步为单例获得对象，而导致非单例
            {
                tp = new ThreadPool(5);
                tp->InitThreadPool();
            }
            pthread_mutex_unlock(&mutex);
        }
        return tp;
    }
};

ThreadPool *Singleton::tp = NULL;

#endif
