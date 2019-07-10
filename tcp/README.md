## Server
```
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
            recv(sock, ...); //当用户关闭连接后，recv失败，服务器也关闭连接
            //...
            send(sock, ...);
        }
    }
```

>     注意：accept的第一个参数为服务器的socket描述字，是服务器开始调用socket()函数生成的，称为监听socket描述字；而accept函数返回的是已连接的socket描述字。一个服务器通常通常仅仅只创建一个监听socket描述字，它在该服务器的生命周期内一直存在。内核为每个由服务器进程接受的客户连接创建了一个已连接socket描述字，当服务器完成了对某个客户的服务，相应的已连接socket描述字就被关闭。

## Client

```
1.创建socket，socket()
2.绑定由操作系统完成
3.与服务器建立连接，connect(socket, addr...)
4.进行会话
    while(1)
    {
        send();
        recv();
    }
```

