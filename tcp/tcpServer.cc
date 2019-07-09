#include "tcpServer.hpp"

void usage(const string& msg)
{
    cout << "ERROR. exp: " << msg << " port" << endl;
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        usage(argv[0]);
        return 1;
    }
    tcpServer *ts = new tcpServer(atoi(argv[1]));
    ts->Initserver();
    ts->Run();
    return 0;
}