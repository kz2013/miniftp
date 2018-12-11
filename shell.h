#ifndef __SHELL_H_
#define __SHELL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "client.h"
#include <sys/types.h>
#include <sys/wait.h>

typedef char *WORD;

void make_args(const char *src, const char *delim);
void clear_args(int argc, char *argv[]);
int searchcmd(const char *arg, int argc, char *argv[]);
int translator(int argc, char *argv[]);
void *ftp(void *arg);
void shell(void);

#endif  // __SHELL_H_
