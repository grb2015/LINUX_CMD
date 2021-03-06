/* webserv.c - a minimal web server (version 0.2)
 *      usage: ws portnumber
 *   features: supports the GET command only
 *             runs in the current directory
 *             forks a new child to handle each request
 *             has MAJOR security holes, for demo purposes only
 *             has many other weaknesses, but is a good start
 *      build: cc webserv.c socklib.c -o webserv
 */

/************************************************************
*	brief : 一个简单的http服务器，只能处理GET请求
*	
*	history : 2017-05-03 renbin.guo craeted

*	usage:

	gcc 12_webserver.c socklib.c -o sebserv

	1. on the server:	
		grb-sim@xxx:~/github/LINUX_CMD/12_webserver$ ./webserv  12345
	
	
	2. on the client:
	
	grb-sim@xxx:~/github/LINUX_CMD/12_webserver$ hostname
	xxx	//my host name is 'xxx'
	grb-sim@xxx:~/github/LINUX_CMD/12_webserver$
	grb-sim@xxx:~$ telnet xxx 12345
	Trying 127.0.1.1...
	Connected to xxx.
	Escape character is '^]'.
	GET test.html HTTP/1.0           //注意:这里需要敲2个回车，也就是要有一个空行!!!!

	HTTP/1.0 200 OK
	Content-type: text/html			// 注意,text/html是放在与12_webserver.c一个目录下

	3333333333
	Connection closed by foreign host.
	grb-sim@xxx:~$ 

	3. on the server
	grb-sim@xxx:~/github/LINUX_CMD/12_webserver$ ./webserv 12345
	got a call: request = GET test.html HTTP/1.0

	------------------------------------------------------------
	note:
		bug ?  本机的IP是. 172.21.1.81  它是一个内网地址，所以无法用外部的浏览器通过 http://xxx:12345来打开,用本地的浏览器是否可以呢?

*        关于架构，每次来一个请求，就开一个子进程去处理。而父进程继续监听。
*
*
************************************************************/
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<string.h>

main(int ac, char *av[])
{
	int 	sock, fd;
	FILE	*fpin;
	char	request[BUFSIZ];

	if ( ac == 1 ){
		fprintf(stderr,"usage: ws portnum\n");
		exit(1);
	}
	sock = make_server_socket( atoi(av[1]) );
	if ( sock == -1 ) exit(2);

	/* main loop here */

	while(1){
		/* take a call and buffer it */
		fd = accept( sock, NULL, NULL );
		fpin = fdopen(fd, "r" );

		/* read request */
		// 读取客户端的第一行		
		fgets(request,BUFSIZ,fpin);
		printf("got a call: request = %s", request);
		read_til_crnl(fpin);

		/* do what client asks */
		process_rq(request, fd);

		fclose(fpin);
	}
}

/* ------------------------------------------------------ *
   read_til_crnl(FILE *)
   skip over all request info until a CRNL is seen
   ------------------------------------------------------ */

// // fgets返回的是buf的首地址，
// // 一直读取，直到读取的为NULL或者遇到\r\n，我们知道，在客户端输入的时候，是输入过一个空行的，fgets读空行就会返回NULL
read_til_crnl(FILE *fp)
{
	char	buf[BUFSIZ];
	while( fgets(buf,BUFSIZ,fp) != NULL && strcmp(buf,"\r\n") != 0 )
		;
}

/* ------------------------------------------------------ *
   process_rq( char *rq, int fd )
   do what the request asks for and write reply to fd 
   handles request in a new process
   rq is HTTP command:  GET /foo/bar.html HTTP/1.0


   fd:   注意这里的fd是fd = accept( sock, NULL, NULL ); 向这个fd写数据，就相当于给客户端发数据
   ------------------------------------------------------ */

process_rq( char *rq, int fd )
{
	char	cmd[BUFSIZ], arg[BUFSIZ];

	/* create a new process and return if not the child */
	// 每次有请求，我们就开一个子进程来处理，而父进程返回，继续监听。
	if ( fork() != 0 )
		return;
	//  rq 这里是 GET test.html HTTP/1.0 ，所以 cmd = GET ，arg=./test.html
	strcpy(arg, "./");		/* precede args with ./ */
	if ( sscanf(rq, "%s%s", cmd, arg+2) != 2 )
		return;

	// 如果不是GET方法，则不支持,直接返回
	if ( strcmp(cmd,"GET") != 0 )
		cannot_do(fd);
	// 如果arg路径不存在,返回404
	else if ( not_exist( arg ) )
		do_404(arg, fd );
	// 如果arg是一个目录，则打印该目录下的所有文件
	else if ( isadir( arg ) )
		do_ls( arg, fd );
	// 如果arg 请求的是以cgi结尾的文件,则是可执行的文件,执行
	else if ( ends_in_cgi( arg ) )
		do_exec( arg, fd );
	// 其它情况, 输出该文件的内容  yes we are !
	else
		do_cat( arg, fd );
}

/* ------------------------------------------------------ *
   the reply header thing: all functions need one
   if content_type is NULL then don't send content type

   breif    : renbin.guo added 2017-07-06 向客户端输出:
                    HTTP/1.0 200 OK\r\n
                    Content-type: text/html
   ------------------------------------------------------ */

header( FILE *fp, char *content_type )
{
	fprintf(fp, "HTTP/1.0 200 OK\r\n");
	if ( content_type )
		fprintf(fp, "Content-type: %s\r\n", content_type );
}

/* ------------------------------------------------------ *
   simple functions first:
        cannot_do(fd)       unimplemented HTTP command
    and do_404(item,fd)     no such object
   ------------------------------------------------------ */

cannot_do(int fd)
{
	FILE	*fp = fdopen(fd,"w");

	fprintf(fp, "HTTP/1.0 501 Not Implemented\r\n");
	fprintf(fp, "Content-type: text/plain\r\n");
	fprintf(fp, "\r\n");

	fprintf(fp, "That command is not yet implemented\r\n");
	fclose(fp);
}

do_404(char *item, int fd)
{
	FILE	*fp = fdopen(fd,"w");

	fprintf(fp, "HTTP/1.0 404 Not Found\r\n");
	fprintf(fp, "Content-type: text/plain\r\n");
	fprintf(fp, "\r\n");

	fprintf(fp, "The item you requested: %s\r\nis not found\r\n", 
			item);
	fclose(fp);
}

/* ------------------------------------------------------ *
   the directory listing section
   isadir() uses stat, not_exist() uses stat
   do_ls runs ls. It should not
   ------------------------------------------------------ */

isadir(char *f)
{
	struct stat info;
	return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}

not_exist(char *f)
{
	struct stat info;
	return( stat(f,&info) == -1 );
}

do_ls(char *dir, int fd)
{
	FILE	*fp ;

	fp = fdopen(fd,"w");
	header(fp, "text/plain");
	fprintf(fp,"\r\n");
	fflush(fp);

	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	execlp("ls","ls","-l",dir,NULL);
	perror(dir);
	exit(1);
}

/* ------------------------------------------------------ *
   the cgi stuff.  function to check extension and
   one to run the program.
   ------------------------------------------------------ */

char * file_type(char *f)
/* returns 'extension' of file */
{
	char	*cp;
	if ( (cp = strrchr(f, '.' )) != NULL )
		return cp+1;
	return "";
}

ends_in_cgi(char *f)
{
	return ( strcmp( file_type(f), "cgi" ) == 0 );
}

do_exec( char *prog, int fd )
{
	FILE	*fp ;

	fp = fdopen(fd,"w");
	header(fp, NULL);
	fflush(fp);
	dup2(fd, 1);
	dup2(fd, 2);
	close(fd);
	execl(prog,prog,NULL);
	perror(prog);
}
/* ------------------------------------------------------ *
   do_cat(filename,fd)
   sends back contents after a header
   ------------------------------------------------------ */

do_cat(char *f, int fd)
{
	char	*extension = file_type(f);
	char	*content = "text/plain";
	FILE	*fpsock, *fpfile;
	int	c;

	if ( strcmp(extension,"html") == 0 )
		content = "text/html";
	else if ( strcmp(extension, "gif") == 0 )
		content = "image/gif";
	else if ( strcmp(extension, "jpg") == 0 )
		content = "image/jpeg";
	else if ( strcmp(extension, "jpeg") == 0 )
		content = "image/jpeg";

    // renbin.guo added 2017/07/06 这里的fd是创建socket返回的fd。用于与客户端通信
	fpsock = fdopen(fd, "w");
	fpfile = fopen( f , "r");
	if ( fpsock != NULL && fpfile != NULL )
	{
		header( fpsock, content );
		fprintf(fpsock, "\r\n");    // 向客户端输出  \r\n 

		/* 打印test.html文件中的内容*/
		while( (c = getc(fpfile) ) != EOF )
			putc(c, fpsock);
		fclose(fpfile);
		fclose(fpsock);
	}
	exit(0);
}

