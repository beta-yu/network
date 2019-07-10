#ifndef __TCPSERVER_HPP_
#define __TCPSERVER_HPP_

#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "threadpool.hpp"

using namespace std;
class tcpServer
{
private:
    int _listen_sock; //该套接字负责接收请求
    int _port; 
    ThreadPool *tp;
    static unordered_map<string, string> dict;
public:
    tcpServer(int port) : _port(port)
    {}

    void Initserver()
    {
        tp = Singleton().GetInstance();
        _listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if(_listen_sock < 0)
        {
            cerr << "listen socket error" << endl;
            exit(2); //套接字创建失败
        }
        sockaddr_in local_addr;
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        local_addr.sin_port = htons(_port);
        if(bind(_listen_sock, (sockaddr *)&local_addr, sizeof(local_addr)) != 0)
        {
            cerr << "bind error" << endl;
            exit(3); //绑定失败
        }
        if(listen(_listen_sock, 5) != 0) //监听_listen_sock，最多允许5个用户处于连接等待状态
        {
            cerr << "listen error" << endl;
            exit(4);
        }
    }

    static void *serverIO(int sock, string ipaddr)
    {
        while(1) //与该连接进行交互
        {
            char buf[1024];
            ssize_t size = recv(sock, buf, sizeof(buf)-1, 0);
            if(size == 0)
            {
                cout << ipaddr << " client quit." << endl;
                close(sock);
                break; //没有获取到有效信息，或对方关闭了连接，recv返回0
            }
            else if(size < 0)
                cerr << "recv error" << endl;
            else
            {
                buf[size] = 0;
                cout << "From " << ipaddr << " recv message: " << buf << endl;
                string ret;
                auto it = dict.find(buf);
                if(it != dict.end())
                    ret = it->second;
                else
                    ret = "Not found!";
                if(send(sock, ret.c_str(), ret.size(), 0) < 0)
                    cout << "send error" << endl;
            }
        }
    }

    void Run()
    {
        while(1)
        {
            //signal(SIGCHLD, SIG_IGN); //父进程忽略子进程退出时发来的信号，子进程资源会被自动回收
            sockaddr_in useraddr;
            socklen_t len = sizeof(useraddr);
            //从_listen_sock的队列的第一个连接中获取信息，并返回一个新的套接字用来处理该连接
            //每个连接绑定一个唯一的socket(返回的sock)，通过该socket进行会话
            int sock = accept(_listen_sock, (sockaddr *)&useraddr, &len);
            //单进程一次只能从连接队列获取一个连接来处理
            if(sock < 0)
            {
                cerr << "accept error" << endl;
                continue;
            }
            cout << "IP: "<< inet_ntoa(useraddr.sin_addr) <<" client coming..." << endl;
            //处理用户会话有三种方式：
            //1.多进程，通过创建子进程来处理与用户会话，父进程继续下一次循环，accept，再创建子进程
            // pid_t pid = fork();
            // if(pid == -1)
            // {
            //     cerr << "fork error" << endl;
            // }
            // else if(pid == 0)
            // {
            //     serverIO(sock);
            // }

            //2.多线程，有多进程类似

            //3.通过线程池来处理用户会话，只需将会话任务加入线程池任务队列
            tp->PutTask(Task(sock, inet_ntoa(useraddr.sin_addr), serverIO));
        }
    }

    ~tcpServer()
    {
        if(_listen_sock >= 0)
            close(_listen_sock);
    }
};

unordered_map<string, string> tcpServer::dict = {
            {"apple", "苹果"},
            {"banana", "香蕉"},
            {"orange", "橘子"}
        };

#endif //__TCPSERVER_HPP_

//当一个用户请求建立连接，该请求会先抵达服务器listen队列，直到服务器从队列中accept该请求
//线程池：会将该请求抽象为一个Task，通过PushTask加入线程池任务队列，
//      每个线程从任务队列中拿任务去执行，即去执行serverIO()，直至与该用户会话结束，serverIO退出，
//      该线程才能去继续拿任务，执行...，因此线程池中线程个数决定了服务器同时可维护的会话个数。