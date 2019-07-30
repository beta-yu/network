#include "Socket.hpp"
#include <poll.h>
#include <vector>
#include <errno.h>
#include <cstring>
#define MAX 128

class PollServer
{
private:
    Socket *listen_sock;
    int port;
public:
    PollServer(int _port):port(_port)
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
    }

    void Run()
    {
        int listen_sock_fd = listen_sock->GetSocket();
        struct pollfd *fds = new struct pollfd[32];
        fds[0].fd = listen_sock_fd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        while(1)
        {
            int max_fd = 0;
            for(size_t i = 0; i < MAX; ++i) //设置readfds,即告诉系统我关心哪些文件的读就绪事件
            {
                if(fdset[i] == -1)
                    continue;
                FD_SET(fdset[i], &readfds);
                max_fd = fdset[i] > max_fd ? fdset[i] : max_fd; 
            }
            int n = select(max_fd+1, &readfds, nullptr, nullptr, nullptr); //timeval为nullptr，select阻塞式的等待事件就绪
            //cout << "n = " << n << endl;
            if(n < 0)
            {
                cerr << "select error." << endl;
                continue;
            }
            for(size_t i = 0; i < MAX; ++i) 
            {
                if(fdset[i] == -1)
                    continue;
                if(FD_ISSET(fdset[i], &readfds))
                {
                    if(fdset[i] == listen_sock_fd) //有新的连接建立请求
                    {
                        int fd = listen_sock->Accept();
                        if(fd < 0) //Accept可能失败
                            continue;
                        size_t j = 0;
                        while(j < MAX && fdset[j] != -1) ++j;

                        if(j == MAX)
                        {
                            close(fd); //若描述符集合已满，即服务器已无法接受新的连接请求
                            cout << "Server full load..." << endl;
                        }
                        else
                        {
                            fdset[j] = fd; //将新连接描述符加入集合，下次select去检查
                            cout << "Have a new connection..." << endl;
                            // for(auto it : fdset)
                            // {
                            //     cout << it << " ";
                            // }
                            // cout << endl;
                        }
                    }
                    else //已建立的连接有读事件就绪
                    {
                        char buf[1024] = {0};
                        ssize_t size = recv(fdset[i], buf, sizeof(buf)-1, 0);
                        if(size < 0)
                        {
                            cerr << "recv error." << endl;
                            continue;
                        }
                        else if(size == 0) //对端关闭了连接
                        {
                            close(fdset[i]);
                            cout << "Have a connection closed..." << endl;
                            fdset[i] = -1; //在描述符集合中删除该连接，下次select时不再检查
                            // for(auto it : fdset)
                            // {
                            //     cout << it << " ";
                            // }
                            // cout << endl;
                        }
                        else //size > 0，读到了发来的数据
                            cout << "Client: " << buf << endl; 
                    }
                }
            }
        }
    }

    ~PollServer()
    {}
};
