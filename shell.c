#include "shell.h"
#include "input.h"
#include <pthread.h>
#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

extern pthread_t pid;

typedef struct args {
        char *argv[128];
        int argc;
} ARGs;
ARGs args;

void make_args(const char *src, const char *delim)
{
        char tmp[strlen(src) + 1];
        strcpy(tmp, src);
        clear_args(args.argc, args.argv);
        for (char *token = strtok(tmp, delim);
                        token; token = strtok(NULL, delim)) {
                args.argv[args.argc++] = strdup(token);
        }
}

void clear_args(int argc, char *argv[])
{
        for (int i = 0; i < argc; i++) {
                if (argv[i]) {
                        free(argv[i]);
		        argv[i] = NULL;
                }
	}
        args.argc = 0;
}

int searchcmd(const char *arg, int argc, char *argv[])
{
        if (!arg)
                return -1;
        for (int i = 0; i < argc; i++)
                if (!strcmp(arg, argv[i]))
                        return i;
        return -1;
}

static void extexec(char *argv[])
{
        int index = 1;
        if (*argv[0] == '!') {
                memmove(argv[0], argv[0] + 1, strlen(argv[0]));
                index--;
        }
        pid_t pid = fork();
        if (pid == -1)
                return;
        if (!pid)
                execvp(argv[index], argv + index);
}

int translator(int argc, char *argv[])
{
        const char *connaddr = NULL;
	char *support[] = {"ls", "cd", "pwd", "mkdir", "rmdir",
                "rename", "delete", "put", "get", "chwkmd",
		"help", "quit", "exit", "bye", "clear", "open"};
        int pos = searchcmd(argv[0], countof(support), support);
        if (!login) {
                if (pos < 10 || pos > 15)
                        pos = -1;
        } else {
                connaddr = workmode == PORT ? NULL : ftpaddr;
        }
        switch (pos) {
        case 0: ls(argv[1], sfd_cmd, connaddr); break;
        case 1: cd(argv[1], sfd_cmd); break;
        case 2: pwd(sfd_cmd); break;
        case 3: _mkdir(argv[1], sfd_cmd); break;
        case 4: _rmdir(argv[1], sfd_cmd); break;
        case 5: _rename(argv[1], argv[2], sfd_cmd); break;
        case 6: _delete(argv[1], sfd_cmd); break;
        case 7: put(argv[1], sfd_cmd, connaddr); break;
        case 8: get(argv[1], sfd_cmd, connaddr); break;
        case 9: chwkmd(argv[1]); break;
        case 10: help(argc, argv); break;
        case 11:        // quit
        case 12:        // exit
        case 13:        // bye
		clear_args(argc, argv);
		close(sfd_cmd);
		printf("bye.\n");
                return 1;
        case 14:
                printf("\033[2J\033[1;1H");     // 清屏并将光标置顶
                break;
        case 15:
                if ((sfd_cmd = openftp(argv[1], 21)) != -1)
                        login = 1;
                break;
        default:
                if (!strcmp(argv[0], "!") || *argv[0] == '!') {
                        extexec(argv);
                } else if (argv[0]) {
                        pos = searchcmd(argv[0], countof(support), support);
                        if (pos == -1)
                                printf("?Invalid command\n");
                        else
                                printf("Not connected.\n");
                }
        }
        return 0;
}

void *ftp(void *arg)
{
        char *buf = arg;
        make_args(buf, " \t\r\n");
        if (!args.argc)
                return NULL;
        if (translator(args.argc, args.argv) == 1)
                return (void *) 1;
        else
                return NULL;
}

void shell(void)
{
        char buf[1024] = {0};
        while (1) {
                printf("\033[1m\033[34mFTP\033[32m> \033[0m");
                fgets_input(buf, sizeof(buf), stdin, strlen("FTP> "));
                void *retval = NULL;
                pthread_join(pid, &retval);
                if (retval == (void *) 1)
                        break;
        }
}
