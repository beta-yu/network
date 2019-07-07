#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;

void Usage(string msg)
{
    cout << "Using: " << msg << " ipaddr port" << endl;
}

void handler(const string& msg, string& result)
{
    static unordered_map<string, string> dict = {
        {"apple", "苹果"},
        {"banana", "香蕉"},
        {"orange", "橘子"}
    };

    auto it = dict.find(msg);
    if(it == dict.end())
        result = "Not Found!";
    else
        result = it->second;
    
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]); //使用错误提示信息
        return 1;
    }
    int sock_fd = -1;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //创建套接字
    if(sock_fd < 0)
    {
        cerr << "socket error" << endl;
        return 2;
    }
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(argv[1]);
    local_addr.sin_port = htons((uint16_t)atoi(argv[2]));
    if(bind(sock_fd, (sockaddr *)&local_addr, sizeof(local_addr)) != 0) //绑定套接字
    {
        cerr << "bind error" << endl;
        return 3;
    }
    while(1) //循环处理事件
    {
        sockaddr_in source_addr;
        socklen_t len = sizeof(source_addr);
        char buf[1024];
        cout << "Recv: ";
        //接受信息，保存发送方sockaddr_in
        int size = recvfrom(sock_fd, buf, sizeof(buf)-1, 0, (sockaddr *)&source_addr, &len);
        if(size < 0)
        {
            cerr << "recvfrom error" << endl;
            continue;
        }
        buf[size] = 0;
        cout << buf << endl;
        string msg;
        handler(buf, msg); //处理信息
        //返回信息
        if(sendto(sock_fd, msg.c_str(), msg.size(), 0, (sockaddr *)&source_addr, len) < 0)
        {
            cerr << "sendto error" << endl;
            continue; 
        }
    }
    close(sock_fd);
    return 0;
}