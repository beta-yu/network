.PUONY:all
all:udpServer udpClient

udpServer:udpServer.cc
	g++ -std=c++11 -o $@ $^
udpClient:udpClient.cc
	g++ -std=c++11 -o $@ $^

.PHONY:clean
clean:
	rm -f udpServer udpClient
