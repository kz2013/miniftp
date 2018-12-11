#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include "input.h"
#include "shell.h"

#define NOTUSING        0
#define NOSPACE         1
#define SPACE           2
#define stdascii(c)     ((c) >= 0 && (c) <= 127)
#define min(x, y)       ((x) < (y) ? (x) : (y))

extern pthread_t pid;
const struct winsize winsize = {0};     // 在<sys/ioctl.h>中声明

typedef struct {
        int x;
        int y;
} point;

struct input {
        char buf[4096];         // 输入缓冲区
        int i;                  // 输入缓冲区的下标(当前位置)
        point first;            // 首字符坐标(以该行的行首为原点)
        point cur;              // 光标坐标
        point last;             // 尾字符坐标
        char line_tail[256];    // 每行尾字符坐标代码
};

int getwinsize(int fd)
{
        struct winsize *ws = &winsize;
        // 检测文件描述符fd是否对应tty终端文件，是则返回1，否则返回0
        if (!isatty(fd))
                return 0;
        // 获取终端窗口的大小，将获取的值放到size结构体中
        if (ioctl(fd, TIOCGWINSZ, ws) < 0) {
                perror("ioctl");
                return -1;
        }
        return 1;
}

static void reprint(struct input *input, int n, point *fix)
{
        printf("\033[J");
        if (input == NULL || n < 0 || fix == NULL)
                return;
        if (fix->x >= sizeof(input->line_tail) - 1)
                return;

        int i = 0, j = 0, characters = input->cur.y - 1;
        char *s = &input->buf[input->i];
        *fix = input->cur;

        while (i < n) {
                if (stdascii(s[i])) {
                        i++;
                        characters++;
                } else {
                        i += 3;
                        characters += 2;
                }
                if (characters >= winsize.ws_col) {
                        fwrite(&s[j], i - j, 1, stdout);
                        if (characters == winsize.ws_col)
                                printf("\n");
                        input->line_tail[fix->x] =
                                characters == winsize.ws_col ? NOSPACE : SPACE;
                        fix->x++;
                        if (fix->x >= sizeof(input->line_tail)) {
                                fix->y = 1;
                                if (fix == &input->cur)
                                        input->i += i - j;
                                return;
                        }
                        if (characters == winsize.ws_col)
                                characters = 0;
                        else
                                characters = 2;
                        j = i;
                }
        }
        fwrite(&s[j], i - j, 1, stdout);
        fflush(stdout);
        fix->y = characters + 1;
        if (fix == &input->cur)
                input->i += n;
        input->last = *fix;
        return;
}

static void recover_cursor(const point *dest, const point *src)
{
        if (dest->x == src->x)
                printf("\033[%dG", dest->y);
        else if (dest->x < src->x)
                printf("\033[%dA\033[%dG", src->x - dest->x, dest->y);
        else
                printf("\033[%dB\033[%dG", dest->x - src->x, dest->y);
        fflush(stdout);
}

static void on_delete(struct input *input)
{
        int n = strlen(&input->buf[input->i]);
        if (n) {
                if (stdascii(input->buf[input->i])) {
                        memmove(&input->buf[input->i],
                                        &input->buf[input->i + 1], n);
                        n--;
                } else {
                        memmove(&input->buf[input->i],
                                        &input->buf[input->i + 3], n - 2);
                        input->buf[input->i + n - 2] = '\0';
                        input->buf[input->i + n - 1] = '\0';
                        n -= 3;
                }
                reprint(input, n, &input->last);
                recover_cursor(&input->cur, &input->last);
        }
}

static void on_left(struct input *input)
{
        if (input->i > 0) {
                if (stdascii(input->buf[input->i - 1])) {
                        input->i--;
                } else {
                        input->i -= 3;
                }
                if (input->cur.y == 1) {        // 隐含条件input->cur.x > 0
                        input->cur.x--;
                        if (stdascii(input->buf[input->i]))
                                input->cur.y = winsize.ws_col;
                        else
                                input->cur.y = winsize.ws_col - 1;
                        if (input->line_tail[input->cur.x] == SPACE)
                                input->cur.y--;
                        printf("\033[A\033[%dG", input->cur.y);
                } else {
                        if (stdascii(input->buf[input->i]))
                                input->cur.y--;
                        else
                                input->cur.y -= 2;
                        printf("\033[%dG", input->cur.y);
                }
                fflush(stdout);
        }
}

static void on_right(struct input *input)
{
        if (strlen(&input->buf[input->i])) {
                if (stdascii(input->buf[input->i])) {
                        input->i++;
                } else {
                        input->i += 3;
                }
                if (input->cur.y == winsize.ws_col - 2 &&
                                !stdascii(input->buf[input->i - 1]) &&
                                !stdascii(input->buf[input->i]) ||
                                input->cur.y == winsize.ws_col - 1 &&
                                !stdascii(input->buf[input->i - 1]) ||
                                input->cur.y == winsize.ws_col) {
                        // 隐含条件input->last.x > input->cur.x
                        input->cur.x++;
                        input->cur.y = 1;
                        printf("\033[B\033[G");
                } else {
                        if (stdascii(input->buf[input->i - 1]))
                                input->cur.y++;
                        else
                                input->cur.y += 2;
                        printf("\033[%dG", input->cur.y);
                }
                fflush(stdout);
        }
}

static void on_home(struct input *input)
{
        if (input->cur.x)
                printf("\033[%dA", input->cur.x);
        printf("\033[%dG", input->first.y);
        input->cur = input->first;
        input->i = 0;
        fflush(stdout);
}

static void on_end(struct input *input)
{
        if (input->cur.x < input->last.x)
                printf("\033[%dB", input->last.x - input->cur.x);
        printf("\033[%dG", input->last.y);
        input->i = strlen(input->buf);
        input->cur = input->last;
        fflush(stdout);
}

static void on_backspace(struct input *input)
{
        if (input->i) {
                on_left(input);
                on_delete(input);
        }
}

static void on_enter(char *s, int size, struct input *input)
{
        on_end(input);
        printf("\n");
        //printf("%c", input->buf[input->i] = '\n');
        s[size - 1] = '\0';
        strncpy(s, input->buf, min(input->i + 1, size - 1));
        pthread_create(&pid, NULL, ftp, s);
}

static void insert(struct input *input, char *tmp, int r)
{
        int n = strlen(&input->buf[input->i]);
        if (n)
                memmove(&input->buf[input->i + r], &input->buf[input->i], n);
        strncpy(&input->buf[input->i], tmp, r);

        reprint(input, r, &input->cur);
        reprint(input, n, &input->last);
        recover_cursor(&input->cur, &input->last);
}

char *fgets_input(char *s, int size, FILE *stream, int prompt_len)
{
        struct input input = {0};               // 当前输入对象
        static char tmp[1024] = {0};            // 当前读取的字符
        int r = 0;                              // 当前读取的字符数
        struct termios stdin_backup, stdin_new; // 在<termios.h>中定义

        if (getwinsize(1) != 1 || stream == NULL)
                goto error;

        fflush(stdout);
        input.last.y = input.cur.y = input.first.y = prompt_len + 1;

        char *p = NULL;
        if (p = strchr(tmp, '\n')) {
                insert(&input, tmp, p - tmp);
                on_enter(s, size, &input);
                memmove(tmp, p + 1, strlen(p));
                goto quit;
        }
        if (tmp[0]) {
                insert(&input, tmp, strlen(tmp));
                memset(tmp, 0, strlen(tmp));
        }

        tcgetattr(1, &stdin_backup);    // 获取终端属性
        stdin_new = stdin_backup;
        stdin_new.c_lflag &= ~(ECHO | ICANON);  // 设为0要用'&'，设为1要用'|'
        tcsetattr(1, TCSANOW, &stdin_new);

        while ((r = read(stream->_fileno, tmp, sizeof(tmp))) != -1) {
                if (p = strchr(tmp, '\n')) {
                        insert(&input, tmp, p - tmp);
                        on_enter(s, size, &input);
                        memmove(tmp, p + 1, strlen(p));
                        goto quit;
                }
                if (r > 1) {
                        if (!strncmp(tmp, "\033[3~", 4))        // Delete键
                                on_delete(&input);
                        else if (!strncmp(tmp, "\033[D", r))    // 左方向键
                                on_left(&input);
                        else if (!strncmp(tmp, "\033[C", r))    // 右方向键
                                on_right(&input);
                        else if (!strncmp(tmp, "\033[H", r))    // Home键
                                on_home(&input);
                        else if (!strncmp(tmp, "\033[F", r))    // End键
                                on_end(&input);
                        else if (!strncmp(tmp, "\033", 1));
                        else
                                insert(&input, tmp, r);
                } else {
                        switch (tmp[0]) { // 检测按下了什么键
                                case 127   : on_backspace(&input); break;
                                case '\033': break;
                                case '\t'  : tmp[0] = ' ';
                                default    : insert(&input, tmp, r);
                        }
                }
                memset(tmp, 0, r);
        }
        perror("read");
        goto error;
quit:
        tcsetattr(1, TCSANOW, &stdin_backup);
        return s;
error:
        return NULL;
}

//printf("input:|%c|=%d=%#o=%#x\n", tmp[0], tmp[0], tmp[0], tmp[0]);
/*
// 测试用例：
int main(int argc, char *argv[])
{
        char str[128] = {0};
        printf("FTP> ");
        fgets_input(str, 128, 0, 5);
        puts(str);
        return 0;
}
*/
