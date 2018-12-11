/*
 * 网络编程客户端socket和connect封装
 * */
#ifndef __NET_CLIENT_H_
#define __NET_CLIENT_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int s_connect(int domain, int type, const char *ipaddr, unsigned short port);

#endif  // __NET_CLIENT_H_
