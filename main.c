#include "client.h"
#include "ftpconnect.h"
#include "shell.h"
#include "input.h"
#include <pthread.h>

pthread_t pid;
extern const struct winsize winsize;

void sigproc(int signal)
{
        switch (signal) {
                case SIGINT: pthread_cancel(pid); printf("\n"); break;
                case SIGCHLD: wait(NULL); break;
                case SIGWINCH: getwinsize(1); break;
                default: break;
        }
}

void helloworld(void)
{
        int pos = (winsize.ws_col - 27) / 2;
        printf("\033[2J\033[%d;%dH", (winsize.ws_row - 8) / 2, pos);
        // 清屏并将光标放置在合适位置
        printf("+-------------------------+\n");
        printf("\033[%dG|                         |\n", pos);
        printf("\033[%dG|     欢迎使用FTP系统     |\n", pos);
        printf("\033[%dG|                         |\n", pos);
        printf("\033[%dG|         制作团队：      |\n", pos);
        printf("\033[%dG|             CSD1803     |\n", pos);
        printf("\033[%dG|\033[1m\033[34m             FTP项目组 \033[0m  |\n", pos);
        printf("\033[%dG+-------------------------+\n", pos);
        getchar();
        printf("\033[2J\033[H");        // 清屏并将光标置顶
        fflush(stdout);
}

int main(void)
{
        if (getwinsize(1) != 1)
                return 1;
        helloworld();

        signal(SIGINT, sigproc);
        signal(SIGWINCH, sigproc);
        signal(SIGCHLD, sigproc);
        shell();
        return 0;
}
