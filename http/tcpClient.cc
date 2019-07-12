#include "tcpClient.hpp"

void Usage(const char* msg)
{
    cout <<"ERROR. exp: "<< msg << " ipaddr port" << endl;
}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        return 1;
    }
    tcpClient *tc = new tcpClient(argv[1], argv[2]);
    tc->InitClient();
    tc->Run();
    return 0;
}
