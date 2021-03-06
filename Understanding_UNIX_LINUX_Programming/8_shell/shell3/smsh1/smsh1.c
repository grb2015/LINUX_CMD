/* smsh1.c small-shell verison1
 * 	first readlly useful version after prompting shell
 * 	this one parses the command line into strings
 * 	user fork, exec, wait , and ignores signals
 *
 * 	将之前的多行命令的shell改进为一行命令
 * history :	2017-04-25 renbin.guo created
 * 
 * note:	把所有函数接口都加到一个smsh.h里面了
 * 
 * 		bug1: github #6 执行一次at命令后，就发生了错误 core dump
 * 		bug2: 直接运行ls ,非常严重的错误
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include "smsh.h"

void setup();
#define	 DFL_PROMPT "> "
int main()
{
	char *cmdline;
	char *prompt;
	char **arglist;
	int result;

	prompt = DFL_PROMPT;
	setup();
	
	
	while( (cmdline = next_cmd(prompt,stdin)) != NULL) // 获取下一个命令行(字符串形式),存放在cmdlien里面,如果返回不为NULL
	{
		if( (arglist = splitline(cmdline)) != NULL) // 解析上面获得的命令行的字符串,解析后放在 指针数组char**arglist里面
		{
			result = execute(arglist);	// 执行命令
			freelist(arglist);		// 释放
		}
		free(cmdline);
	}
	return 0;
}

void setup()
{
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);
}
	
