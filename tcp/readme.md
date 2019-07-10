## Server
1.创建socket, socket()
2.绑定地址、端口，bind(AF_INET, INADDR_ANY, Port)
3.开始监听，listen(socket, )
4.循环的接受请求,建立连接
    while(1)
    {
        sock = accept(socket, sockaddr_in, len);
        5.与该用户连接会话
        while(1)
        {
            recv(sock, ...);
            //...
            send(sock, ...);
        }
    }

## Client
1.创建socket，socket()
2.绑定由操作系统完成
3.与服务器建立连接，connect(socket, addr...)
4.进行会话
    while(1)
    {
        send();
        recv();
    }