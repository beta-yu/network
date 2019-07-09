#ifndef __TCPCLIENT_HPP_
#define __TCPCLIENT_HPP_

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
using namespace std;
class tcpClient
{
private:
    int _sock;
    string _ip;
    string _port;
public:
    tcpClient(const string& ip, const string& port) : _ip(ip),_port(port)
    {}
    void InitClient()
    {
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        if(_sock < 0)
        {
            cerr << "socket error" << endl;
            exit(2);
        }
        // uint32_t ip;
        // if(inet_pton(AF_INET, _ip.c_str(), &ip) != 1)
        // {
        //     cerr << "ip error" << endl;
        //     exit(3);
        // }
        sockaddr_in server_addr;
        server_addr.sin_family =AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
        server_addr.sin_port = htons(atoi(_port.c_str()));
        if(connect(_sock, (sockaddr *)&server_addr, sizeof(server_addr)) != 0)
        {
            cerr << "connect error" << endl;
            exit(4);
        }
    }
    void Run()
    {
        while(1)
        {
            string msg;
            cout << "Please enter: ";
            cin >> msg;
            if(send(_sock, msg.c_str(), msg.size(), 0) < 0)
            {
                cerr << "send error" << endl;
                continue;
            }
            char buf[1024];
            ssize_t size = recv(_sock, buf, sizeof(buf)-1, 0);
            if(size < 0)
            {
                cerr << "recv error" << endl;
                continue;
            }
            buf[size] = 0;
            cout << "Result: " << buf << endl;
        }
    }
    ~tcpClient()
    {
        if(_sock >= 0)
            close(_sock);
    }
};

#endif //__TCPCLIENT_HPP_