#include "net_client.h"
#define handle_error(msg)  do { perror(msg); return -1; } while (0)

int s_connect(int domain, int type, const char *ipaddr, unsigned short port)
{
        struct sockaddr_in client;
        // 创建一个通讯端点，返回一个文件描述符
        int sfd = socket(domain, type, 0);
        if (sfd == -1)
                handle_error("socket");
        // 需要对client变量的成员进行初始化
        client.sin_family = domain;
        client.sin_port = htons(port);
        inet_pton(domain, ipaddr, &client.sin_addr);
        //client.sin_addr.s_addr = htonl(INADDR_ANY);
        if (connect(sfd, (struct sockaddr *) &client, sizeof(client)) == -1)
                handle_error("connect");

        return sfd;
}
