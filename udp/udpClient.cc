#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::cin;
using std::endl;
using std::string;

void Usage(string arg)
{
    cout << "Using: " << arg << " ipaddr port" << endl;
}
int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        return 1;
    }
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0); //创建套接字
    if(sock_fd < 0)
    {
        cout << "socket error" << endl;
        return 2;
    }
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //inet_addr完成1.将字符串（点分十进制格式的IP地址）转为一个整形；2.将整形转网络字节序的IPV4地址
    server_addr.sin_port = htons(atoi(argv[2])); //port:16bit
    while(1)
    {
        cout << "Please Enter: ";
        string msg;
        cin >> msg;
        //向服务器发送请求，OS会为套接字分配可用端口，绑定本机地址信息
        if(sendto(sock_fd, msg.c_str(), msg.size(), 0, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            cerr << "sendto error" << endl;
            continue;
        }
        char buf[1024];
        sockaddr_in addr;
        socklen_t len = sizeof(addr);
        //接收服务器响应信息
        int size = recvfrom(sock_fd, buf, sizeof(buf)-1, 0, (sockaddr *)&addr, &len);
        if(size < 0)
        {
            cerr << "recvfrom error" << endl;
            continue;
        }
        buf[size] = 0;
        cout << "return result: ";
        cout << buf << endl;  
    }
    close(sock_fd);
    return 0;
}