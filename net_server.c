#include "net_server.h"

#define handle_error(msg)  do { perror(msg); return -1; } while (0)
#define LISTEN_MAX 16

int s_bind(int domain, int type, unsigned short port)
{
        struct sockaddr_in server;
        // 创建一个通讯端点，返回一个文件描述符
        int sfd = socket(domain, type, 0);
        if (sfd == -1)
                handle_error("socket");
        // 需要对server变量的成员进行初始化
        server.sin_family = domain;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        // 将server绑定到sfd上
        if (bind(sfd, (struct sockaddr *) &server, sizeof(server)) == -1)
                handle_error("bind");

        return sfd;
}

int s_listen(int domain, int type, unsigned short port)
{
        // 创建一个通讯端点，并绑定ip地址信息，返回一个文件描述符
        int sfd = s_bind(domain, type, port);
        if (sfd == -1)
                return -1;
        // 开始监听设定的端口
        if (listen(sfd, LISTEN_MAX) == -1)
                handle_error("listen");

        return sfd;
}

int s_accept(int domain, int type, unsigned short port)
{
        // 创建一个通讯端点，并绑定ip地址信息，返回一个文件描述符
        int sfd = s_listen(domain, type, port);
        if (sfd == -1)
                return -1;
        int cfd = accept(sfd, NULL, NULL);
        if (cfd == -1)
                handle_error("accept");
        close(sfd);

        return cfd;
}

int s_accept_log(int domain, int type, unsigned short port,
                 struct sockaddr_in *client, socklen_t *addrlen)
{
        struct sockaddr_in server;
        // 创建一个通讯端点，返回一个文件描述符
        int sfd = socket(domain, type, 0);
        if (sfd == -1)
                handle_error("socket");
        // 需要对server变量的成员进行初始化
        server.sin_family = domain;
        server.sin_port = htons(port);
        server.sin_addr.s_addr = htonl(INADDR_ANY);
        // 将server绑定到sfd上
        if (bind(sfd, (struct sockaddr *) &server, sizeof(server)) == -1)
                handle_error("bind");
        // 开始监听设定的端口
        if (listen(sfd, LISTEN_MAX) == -1)
                handle_error("listen");

        int cfd = accept(sfd, (struct sockaddr *) client, addrlen);
        if (cfd == -1)
                handle_error("accept");
        close(sfd);

        return cfd;
}
