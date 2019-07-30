#include "EpollServer.hpp"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        cout << "Usage: " << argv[0] << " port" << endl;
        return 1;
    }
    EpollServer server(atoi(argv[1]));
    server.InitServer();
    server.Run();
}