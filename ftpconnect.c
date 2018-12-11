#include "ftpconnect.h"

int ftpconnect(const char *ipaddr, unsigned short port)
{
        int sfd = -1;
        if (ipaddr)
                sfd = s_connect(AF_INET, SOCK_STREAM, ipaddr, port);
        else
                sfd = s_accept(AF_INET, SOCK_STREAM, port);

        // 设置读写超时时间为3秒
        struct timeval timeout = {3, 0};
        setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO,
                        (char *) &timeout, sizeof(struct timeval));
        setsockopt(sfd, SOL_SOCKET, SO_SNDTIMEO,
                        (char *) &timeout, sizeof(struct timeval));
        return sfd;
}
