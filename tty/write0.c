/* write0.c
 *  
 *   	purpose: send messages to another terminal
 *      method:  open the other terminal for output then
 *     	 	 copy from stdin to that terminal
 *      shows:  a terminal is just a file supporting regular i/o
 *      usage: write0 ttyname                                                                     
 *        
 *
 * 	tty跟普通文件一样，read, write 一个tty就相当于与另外的
 * 	用户进行网络聊天了。(要是不是在一台主机上呢/?)
 *	
 *	how to run this program?
 *
 *	tty0::
 *	[root@ tty]# who
 *     	root     pts/0        2017-04-07 21:27 (192.168.238.1)
 *	root     pts/1        2017-04-07 21:27 (192.168.238.1)
 *	
 *	tty1:
 *
 *	[root@ tty]# ./write0  /dev/pts/0
 *		dhehgigh
 *		hello
 *
 *
 * */

#include<stdio.h>
#include<fcntl.h>	// for open
//#include< unistd.h>	// read & write
int main(int argc,char **argv)
{
	int fd ;
	char buf[BUFSIZ];
	
	if(argc != 2)
	{
		fprintf(stderr,"usage: write0 ttyname\n");
		exit(1);
	}
	fd = open(argv[1],O_WRONLY);  // 只写打开
	{	
		if(fd == -1)
		{
			perror(argv[1]);
			exit(1);
		}
	}

	/*从stdin获取输入，如果输入不为空，就一直向fd写*/
	while(fgets(buf,BUFSIZ,stdin) != NULL)
	{
		if(write(fd,buf,strlen(buf)) == -1) 
			break;
	}
	close(fd);
}
	
	


