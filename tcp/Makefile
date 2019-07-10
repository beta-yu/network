.PUONY:all
all:tcpServer tcpClient

tcpServer:tcpServer.cc
	g++ -std=c++11 -lpthread -o $@ $^
tcpClient:tcpClient.cc
	g++ -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm -f tcpServer tcpClient
