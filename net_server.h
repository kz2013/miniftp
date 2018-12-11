/*
 * 网络编程服务器端socket，bind和listen封装
 * */
#ifndef __NET_SERVER_H_
#define __NET_SERVER_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int s_bind(int domain, int type, unsigned short port);
int s_listen(int domain, int type, unsigned short port);
int s_accept(int domain, int type, unsigned short port);
int s_accept_log(int domain, int type, unsigned short port,
                 struct sockaddr_in *client, socklen_t *addrlen);

#endif  // __NET_SERVER_H_
