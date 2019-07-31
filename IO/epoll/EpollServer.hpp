#include "../poll/Socket.hpp"
#include <sys/epoll.h>
#include <errno.h>
#include <cstring>
#include <string>
#include <fcntl.h>
#define MAX 128

class Util
{
private:
  static struct epoll_event event;
public:
  static void SetNonBlock(int fd)
  {
    int fl = fcntl(fd, F_GETFD);
    if(fl < 0)
    {
      cerr << "getfd failed." << endl;
      return;
    }
    fcntl(fd, F_SETFL, fl | O_NONBLOCK);
  }
  static bool EpollAdd(int eventpoll, int fd)
  {
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = fd;
    if(epoll_ctl(eventpoll, EPOLL_CTL_ADD, fd, &event) < 0)
    {
      cerr << "epoll_ctl_add failed." << endl;
      return false;
    }
    return true;
  }
  static bool EpollDel(int eventpoll, int fd)
  {
    if(epoll_ctl(eventpoll, EPOLL_CTL_DEL, fd, nullptr) < 0)
    {
      cerr << "epoll_ctl_del failed." << endl;
      return false;
    }
    close(fd);
    return true;
  }
  static bool EpollMod(int eventpoll, int fd, uint32_t events)
  {
    event.events = events | EPOLLET;
    event.data.fd = fd;
    if(epoll_ctl(eventpoll, EPOLL_CTL_MOD, fd, &event) < 0)
    {
      cerr << "epoll_ctl_mod failed." << endl;
      return false;
    }
    return true;
  }
};

struct epoll_event Util::event = epoll_event();

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
          while(1) //必须确定已经处理完就绪的事件
          {
            int fd = listen_sock->Accept();
            if(fd < 0)
            {
              if(errno == EAGAIN) //套接字被设置为非阻塞，不存在就绪事件时errno被设置为EAGIN
                break;
              continue;              
            }
            Util::SetNonBlock(fd);
            if(!Util::EpollAdd(eventpoll, fd))
              continue;
          }
          cout << "Have a new connection." << endl;
        }
        else //非listen_socket事件就绪
        {
          char buf[10240]; //假定足够大
          bool close_flag = false;
          while(1)
          {
            ssize_t size = recv(_events[i].data.fd, buf, sizeof(buf)-1, 0);
            buf[size] = 0;
            if(size < 0) //出错，包括EAGAIN
            {
              if(errno == EAGAIN) //已无数据可读
                break;
              cerr << "recv error." << endl;
              if(!Util::EpollDel(eventpoll, _events[i].data.fd))
                continue;
              close(_events[i].data.fd);
              close_flag = true;
              break;
            }
            else if(size == 0) //对端关闭连接
            {
              if(!Util::EpollDel(eventpoll, _events[i].data.fd))
                continue;
              close(_events[i].data.fd);
              close_flag = true;
              cout << "Client quit." << endl;
              break;
            }
            else
            {
              cout << buf;
            }
            if(size < sizeof(buf)-1)
              break;
          }
          cout << endl;
          //收到消息需进行回复，注册关心处理该连接的描述符写事件就绪
          if(!close_flag)
            Util::EpollMod(eventpoll, _events[i].data.fd, EPOLLOUT); 
          //bug,不知道读到什么是一个请求结束，需根据协议确定，有可能一个请求分为两次发送
        }
      }
      if(_events[i].events & EPOLLOUT) //写事件就绪
      {
        string msg = "HTTP/1.0 200 OK\r\n\r\n<html><h1>The response from a epoll server.</h1></html>";
        ssize_t len = 0;
        while(1)
        {
          ssize_t size = send(_events[i].data.fd, msg.c_str(), msg.size(), 0);
          if(size < 0)
          {
            if(errno == EAGAIN)
              break;
            cerr << "send failed." << endl;
          }
          len += size;
          if(len >= msg.size())
            break;
        }
        Util::EpollDel(eventpoll, _events[i].data.fd);     
        close(_events[i].data.fd);
        cout << "link close." << endl; //基于短连接，一次交互后关闭连接
      }
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
    Util::SetNonBlock(listen_sock_fd);
    if(!Util::EpollAdd(eventpoll, listen_sock_fd))
      exit(5);
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

//ET模式epoll server
//BUG：在读数据时，并不知道特定协议，因此不能确定一个完整的报文，可能会存在一个报文分两次发...