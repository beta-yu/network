#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <errno.h>
using namespace std;

int main()
{
	////writev()
	//int fd = open("./text", O_WRONLY | O_CREAT, 0644);
	//if(fd < 0)
	//{
	//	cerr << "open file failed." << endl;
	//	exit(1);
	//}
	//char buf1[] = "hello\n";
	//char buf2[] = "world\n";
	//struct iovec iov1 = {buf1, strlen(buf1)};
	//struct iovec iov2 = {buf2, strlen(buf2)};
	//struct iovec iov[] = {iov1, iov2};
	//ssize_t size = writev(fd, iov, 2);
	//cout << size << endl;


	//readv()
	int fd = open("./text", O_RDONLY);
	if(fd < 0)
	{
		cerr << "open file failed: " << strerror(errno) << endl;
		exit(1);
	}
	char buf1[20] = {0};
	char buf2[20] = {0};
	struct iovec iov1 = {buf1, 6}; //向buf1中读取6个字节
	struct iovec iov2 = {buf2, 6};
	struct iovec iov[] = {iov1, iov2};
	ssize_t size = readv(fd, iov, 2);
	cout << size << endl;
	cout << "buf1: "<< buf1 << endl;
	cout << "buf2: "<< buf2 << endl;
	exit(0);
}
