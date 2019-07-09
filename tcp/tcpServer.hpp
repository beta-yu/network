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

using namespace std;
class tcpServer
{
private:
    int _listen_sock; ////该套接字负责接收请求
    int _port; 

public:
    tcpServer(int port) : _port(port)
    {}

    void Initserver()
    {
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

    void serverIO(int sock)
    {
        static unordered_map<string, string> dict = {
            {"apple", "苹果"},
            {"banana", "香蕉"},
            {"orange", "橘子"}
        };

        while(1) //与该连接进行交互
        {
            char buf[1024];
            ssize_t size = recv(sock, buf, sizeof(buf)-1, 0);
            if(size == 0)
            {
                cout << "Client quit." << endl;
                close(sock);
                break; //没有获取到有效信息，或对方关闭了连接，recv返回0
            }
            else if(size < 0)
            {
                cerr << "recv error" << endl;
            }
            else
            {
                buf[size] = 0;
                cout << "Recv message: " << buf << endl;
                string ret;
                auto it = dict.find(buf);
                if(it != dict.end())
                {
                    ret = it->second;
                }
                else
                {
                    ret = "Not found!";
                }
                if(send(sock, ret.c_str(), ret.size(), 0) < 0)
                {
                    cout << "send error" << endl;
                }
            }
        }
    }

    void Run()
    {
        while(1)
        {
            signal(SIGCHLD, SIG_IGN); //父进程忽略子进程退出时发来的信号，子进程资源会被自动回收
            sockaddr_in useraddr;
            socklen_t len = sizeof(useraddr);
            //从_listen_sock的队列的第一个连接中获取信息，并返回一个新的套接字用来处理该连接
            int sock = accept(_listen_sock, (sockaddr *)&useraddr, &len);
            //单进程一次只能从连接队列获取一个连接来处理
            if(sock < 0)
            {
                cerr << "accept error" << endl;
                continue;
            }
            cout << "IP: "<< inet_ntoa(useraddr.sin_addr) <<" client coming..." << endl;
            pid_t pid = fork();
            if(pid == -1)
            {
                cerr << "fork error" << endl;
            }
            else if(pid == 0)
            {
                serverIO(sock);
            }
        }
    }

    ~tcpServer()
    {
        if(_listen_sock >= 0)
            close(_listen_sock);
    }
};

#endif //__TCPSERVER_HPP_