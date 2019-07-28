//使用存储映射mmap实现文件拷贝
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
using namespace std;

#define COPYINCR (1024*1024*1024) 
//1GB，可映射最大，考虑内存不足时，可能无法将一个很大的文件中的所有内容一次都映射到内存中

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        cerr << "usage: " << argv[0] << " [fromfile] [tofile]" << endl;
        exit(1);
    }
    int fdin, fdout;
    fdin = open(argv[1], O_RDONLY);
    if(fdin < 0)
    {
        cerr << "open file failed." << endl;
        exit(2);
    }
    fdout = open(argv[2], O_RDWR | O_CREAT, 0644);
    if(fdout < 0)
    {
        cerr << "open file failed." << endl;
        exit(3);
    }
    struct stat src_f;
    if(fstat(fdin, &src_f) != 0) //获取输入文件信息,temp.st_size为文件大小
    {
        cerr << "fstat error" << endl;
        exit(4);
    }
    if(ftruncate(fdout, src_f.st_size) != 0) //设置输出文件大小
    {
        cerr << "ftruncate failed." << endl;
        exit(5);
    }
    off_t fsz = 0; //已拷贝字节数
    size_t copysz; //每次拷贝字节数
    while(fsz < src_f.st_size)
    {
        if((src_f.st_size - fsz) > COPYINCR)
            copysz = COPYINCR;
        else
            copysz = src_f.st_size - fsz;
        
        //建立fdin的存储映射区，
        //映射copysz个字节，
        //addr = 0 映射区起始地址由系统选择，
        //PROT_READ 映射区可读,
        //MAP_SHARED 对映射区的存储操作会修改被映射文件，
        //off 映射区相对文件的起始偏移量，从文件fsz字节开始映射copysz个字节。
        void *src = mmap(0, copysz, PROT_READ, MAP_SHARED, fdin, fsz);
        if(src == MAP_FAILED)
        {
            cerr << __LINE__ << " map failed: " << strerror(errno) << endl;
            exit(6);
        }
        void *dst = mmap(0, copysz, PROT_WRITE, MAP_SHARED, fdout, fsz); 
		//为什么设置PROT_WRITE必须文件以可读可写打开，只可写不行？
        if(dst == MAP_FAILED)
        {
            cerr << __LINE__ << " map failed: " << strerror(errno) << endl;            
            exit(7);
        }
        memcpy(dst, src, copysz); //copy
        munmap(src, copysz); 
		//在映射文件后一部分数据之前，需要对前一部分解除映射，
		//MAP_SHARED区磁盘文件的更新，会在将数据写到存储映射区后的某个时刻，按内核虚拟存储算法自动进行
        munmap(dst, copysz);
        fsz += copysz;
    }
    exit(0);
}
