.PUONY:all
all:udpServer udpClient

udpServer:udpServer.cc
	g++ -o $@ $^
udpClient:udpClient.cc
	g++ -o $@ $^

.PHONY:clean
clean:
	rm -f udpServer udpClient
