#ifndef __FTPBASE_H_
#define __FTPBASE_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char cmdbuf[128];
char rbuf[1024];	// 读缓存
char wbuf[1024];	// 写缓存

enum WORKMODE {PORT, PASV};

extern char ftpaddr[16]; // 用于记录ftp服务器的ip地址
extern int workmode;     // 用于确定工作模式(主动模式：PORT；被动模式：PASV)
extern int sfd_cmd;      // 用于确定命令连接的连接描述符
extern int login;        // 用于确定登录状态(未登录：0；登录：1)

void do_cmds(int sfd);
int do_cmds_background(int sfd);
void show_results(int sfd);

unsigned short pasvmode(int sfd_cmd);
unsigned short portmode(int sfd_cmd);
unsigned short getport(int sfd_cmd, const char *ipaddr);

#endif	// __FTPBASE_H_
