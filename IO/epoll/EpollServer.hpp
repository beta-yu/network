#include "../poll/Socket.hpp"
#include <sys/epoll.h>
#include <errno.h>
#include <cstring>
#include <string>
#define MAX 128

class EpollServer
{
private:
    Socket *listen_sock;
    int port;
    int eventpoll; //epoll模型标识
private:
    void Handle(struct epoll_event *_events, int sz)
    {
        for(int i = 0; i < sz; ++i)
        {
            if(_events[i].events & EPOLLIN) //读事件就绪
            {
                if(_events[i].data.fd == listen_sock->GetSocket()) //有新的连接建立请求
                {
                    int fd = listen_sock->Accept();
                    if(fd < 0)
                        continue;
                    struct epoll_event event;
                    event.events = EPOLLIN;
                    event.data.fd = fd;
                    if(epoll_ctl(eventpoll, EPOLL_CTL_ADD, fd, &event) < 0)
                    {
                        cerr << "epoll_ctl_add failed." << endl;
                        continue;
                    }
                    cout << "Have a new connection." << endl;
                }
                else //非listen_socket事件就绪
                {
                    char buf[1024];
                    ssize_t size = recv(_events[i].data.fd, buf, sizeof(buf)-1, 0);
                    buf[size] = 0;
                    if(size < 0)
                    {
                        cerr << "recv error." << endl;
                    }
                    else if(size == 0) //对端关闭连接
                    {
                        if(epoll_ctl(eventpoll, EPOLL_CTL_DEL, _events[i].data.fd, nullptr) < 0)
                        //EPOLL_CTL_DEL操作忽略参数epoll_event *event
                        {
                            cerr << "epoll_ctl_del failed." << endl;
                            cerr << strerror(errno) << endl;
                            continue;
                        }
                        close(_events[i].data.fd);
                        cout << "Client quit." << endl;
                    }
                    else
                    {
                        cout << "Recv msg: " << buf << endl;
                        //收到消息需进行回复，注册关心处理该连接的描述符写事件就绪
                        struct epoll_event event;
                        event.events = EPOLLOUT;
                        event.data.fd = _events[i].data.fd;
                        if(epoll_ctl(eventpoll, EPOLL_CTL_MOD, _events[i].data.fd, &event) < 0)
                            cerr << "epoll_ctl_mod failed." << endl;
                    }
                }
            }
            // if(_events[i].events & EPOLLOUT) //写事件就绪
            // {
            //     string msg = "I received.";
            //     if(send(_events[i].data.fd, msg.c_str(), msg.size(), 0) < 0)
            //         cerr << "send failed." << endl;
            //     //发送完数据，关心对方是否会回复，关心该连接描述符的读事件
            //     struct epoll_event event;
            //     event.events = EPOLLIN;
            //     event.data.fd = _events[i].data.fd;
            //     if(epoll_ctl(eventpoll, EPOLL_CTL_MOD, _events[i].data.fd, &event) < 0)
            //         cerr << "epoll_ctl_mod failed." << endl;
            // }
        }
    }
public:
    EpollServer(int _port):port(_port),eventpoll(-1)
    {}

    void InitServer()
    {
        listen_sock = new Socket(port);
        listen_sock->Create();
        int opt = 1;
        int ret = setsockopt(listen_sock->GetSocket(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(ret != 0)
            cerr << "setsockopt failed." << endl;
        listen_sock->Bind();
        listen_sock->Listen();
        eventpoll = epoll_create(512); //函数参数在新的实现中已被忽略
        if(eventpoll < 0)
        {
            cerr << "epoll_create failed." << endl;
            exit(4);
        }
    }

    void Run()
    {
        int listen_sock_fd = listen_sock->GetSocket();
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = listen_sock_fd;
        if(epoll_ctl(eventpoll, EPOLL_CTL_ADD, listen_sock_fd, &event) != 0)
        {
            cerr << "epoll_ctl_add failed." << endl;
            exit(5);
        }
        struct epoll_event *_events = new epoll_event[MAX];
        while(1)
        {
            int sz = epoll_wait(eventpoll, _events, MAX, -1); //timeout单位为毫秒，-1表示永久阻塞
            //返回值为就绪事件数目，即events中前sz个epoll_event都有事件就绪
            if(sz < 0)
            {
                cerr << "epoll_wait failed." << endl;
                continue;
            }
            else if(sz == 0)
            {
                cout << "epoll_wait timeout..." << endl;
                continue;
            }
            else //事件就绪，处理事件
            {
                Handle(_events, sz);
            }
        }
    }
    ~EpollServer()
    {
        if(eventpoll != -1)
            close(eventpoll);
    }
};
