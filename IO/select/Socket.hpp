#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;
#define BACKLOG 5

class Socket
{
private:
    int sock;
    int port;
public:
    Socket(int _port):sock(-1), port(_port)
    {}
    void Create()
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
        {
            cerr << "create socket failed." << endl;
            exit(1);
        }
        int opt = 1;
        int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(ret != 0)
            cerr << "setsockopt failed." << endl;
    }
    void Bind()
    {
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        address.sin_port = htons(port);
        if(bind(sock, (sockaddr *)&address, sizeof(address)) != 0)
        {
            cerr << "bind failed." << endl;
            exit(2);            
        }
    }
    void Listen()
    {
        if(listen(sock, BACKLOG) != 0)
        {
            cerr << "listen failed." << endl;
            exit(3);
        }
    }
    int Accept()
    {
        sockaddr peer;
        socklen_t len = sizeof(peer);
        int fd = accept(sock, (sockaddr *)&peer, &len);
        if(fd < 0)
        {
            cerr << "accept failed." << endl;
        }
        return fd;
    }
    int GetSocket()
    {
        return sock;
    }
    ~Socket()
    {
        if(sock > 0)
        {
            close(sock);
        }
    }
};