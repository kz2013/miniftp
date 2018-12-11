#include "ftpbase.h"

char ftpaddr[16] = {0}; // 用于记录ftp服务器的ip地址
int workmode = PASV;    // 用于确定工作模式(主动模式：PORT；被动模式：PASV)
int sfd_cmd = -1;       // 用于确定命令连接的连接描述符
int login = 0;          // 用于确定登录状态(未登录：0；登录：1)

void do_cmds(int sfd)
{
        // 向服务器发送消息
        send(sfd, cmdbuf, strlen(cmdbuf), 0);
        // 显示响应结果
        show_results(sfd);
}

int do_cmds_background(int sfd)
{
        // 向服务器发送消息
        send(sfd, cmdbuf, strlen(cmdbuf), 0);
        // 阻塞等待服务器的响应消息，返回实际收到的字节数
        return recv(sfd, rbuf, sizeof(rbuf) - 1, 0);
}

void show_results(int sfd)
{
        int r = 0;
        do {
                // 阻塞等待服务器的响应消息
                r = recv(sfd, rbuf, sizeof(rbuf) - 1, 0);
                if (r == -1) {
                        perror("recv");
                        return;
                }
                // 将服务器的响应消息输出到屏幕
                fwrite(rbuf, r, 1, stdout);
        } while (r >= sizeof(rbuf) - 1);
        fflush(stdout);
}

unsigned short pasvmode(int sfd_cmd)
{
        // 进入被动模式
        sprintf(cmdbuf, "PASV\r\n");
        do_cmds(sfd_cmd);
        int p1, p2;
        sscanf(rbuf, "%*[^(]%*c%*d,%*d,%*d,%*d,%d,%d", &p1, &p2);
        unsigned short port = (p1 << 8) | p2;

        return port;
}

unsigned short portmode(int sfd_cmd)
{
        // 进入主动模式
        int h1, h2, h3, h4, p1, p2;
	struct sockaddr_in myaddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	char ipaddr[16] = {0};
	// 获取本机ip地址
	int failed = getsockname(sfd_cmd, (struct sockaddr *) &myaddr, &addrlen);
	if (failed)     // 如果没有获取到本机的ip地址
		return 0;
	inet_ntop(AF_INET, &myaddr.sin_addr, ipaddr, sizeof(ipaddr));
        sscanf(ipaddr, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
        unsigned short port = rand() % 64512 + 1024;
        p1 = port >> 8;
        p2 = port & 0x00ff;
        sprintf(cmdbuf, "PORT %d,%d,%d,%d,%d,%d\r\n", h1, h2, h3, h4, p1, p2);
        do_cmds(sfd_cmd);

        return port;
}

unsigned short getport(int sfd_cmd, const char *ipaddr)
{
        if (!ipaddr)
                return portmode(sfd_cmd);
        else
                return pasvmode(sfd_cmd);
}
