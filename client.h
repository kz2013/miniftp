#ifndef __CLIENT_H_
#define __CLIENT_H_
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ftpbase.h"
#include "ftpconnect.h"

int openftp(const char *ipaddr, unsigned short port);
void ls(const char *pathname, int sfd_cmd, const char *ipaddr);
void cd(const char *pathname, int sfd_cmd);
void pwd(int sfd_cmd);
void _mkdir(const char *pathname, int sfd_cmd);
void _rename(const char *pathname, const char *filename, int sfd_cmd);
void _delete(const char *name, int sfd_cmd);
void _rmdir(const char *pathname, int sfd_cmd);
void get(const char *filename, int sfd_cmd, const char *ipaddr);
void put(const char *filename, int sfd_cmd, const char *ipaddr);
void chwkmd(const char *mode);
void help(int argc, char *argv[]);

#endif  // __CLIENT_H_
