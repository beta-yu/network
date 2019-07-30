#include "Socket.hpp"
#include <poll.h>
#include <vector>
#include <errno.h>
#include <cstring>

class PollServer
{
private:
    Socket *listen_sock;
    int port;
    struct pollfd *fds = nullptr;
    size_t fds_size = 2;
private:
    void Handle(nfds_t& nfds, int listen_sock_fd)
    {
        for(nfds_t i = 0; i < nfds; ++i)
        {
            if(fds[i].revents & POLLIN) //该描述符读事件就绪
            {
                if(fds[i].fd == listen_sock_fd) //有新的连接
                {
                    int fd = listen_sock->Accept();
                    if(fd < 0)
                        continue;
                    if(nfds == fds_size) //fds需要扩容，poll可同时维护的连接无上限，因为可对fds扩容
                    {
                        fds_size *= 2;
                        fds = (struct pollfd*)realloc(fds, sizeof(struct pollfd) * fds_size);
                        cout << "fds expind." << endl;
                    }
                    fds[nfds].fd = fd;
                    fds[nfds].events = POLLIN;
                    fds[nfds].revents = 0;
                    ++nfds;
                    cout << "Have a new connection..." << endl;
                }
                else
                {
                    char buf[1024];
                    ssize_t size = recv(fds[i].fd, buf, sizeof(buf), 0);
                    if(size < 0)
                    {
                        cerr << "recv error." << endl;
                    }
                    else if(size == 0) //对端关闭连接
                    {
                        close(fds[i].fd);
                        nfds_t j = 0;
                        for(; j < nfds; ++j)
                        {
                            if(fds[j].fd == fds[i].fd)
                            {
                                break;
                            }
                        }
                        while(j < nfds-1) //需确保fds数组内容连续
                        {
                            fds[j] = fds[j+1];
                            ++j;
                        }
                        --nfds;
                        --i; //因数组从i位置开始后半部分统一前移
                        cout << "Client quit..." << endl;
                    }
                    else
                    {
                        cout << "Recv msg: " << buf << endl;
                    }
                }
            }
        }
    }
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
        fds = new struct pollfd[fds_size];
    }

    void Run()
    {
        int listen_sock_fd = listen_sock->GetSocket();
        fds[0].fd = listen_sock_fd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        nfds_t nfds = 1;
        while(1)
        {
            int n = poll(fds, nfds, -1);
            if(n < 0)
            {
                cerr << "poll error." << endl;
                continue;
            }
            else if(n == 0)
            {
                cout << "timeout..." << endl;
                continue;
            }
            else //有事件就绪
            {
                cout << "Handle..." << endl;
                Handle(nfds, listen_sock_fd);
            }
        }
    }

    ~PollServer()
    {}
};
