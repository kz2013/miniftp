#include "client.h"
#include "strkit.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#define countof(arr) (sizeof(arr) / sizeof(arr[0]))

int openftp(const char *ipaddr, unsigned short port)
{
        char username[16] = {0};
        char password[16] = {0};
        char serverip[64] = {0};
        char systype[8] = {0};
        if (!ipaddr) {
                printf("(to) ");
                fgets(serverip, sizeof(serverip), stdin);
                strtrim(serverip);
                if (serverip[0] == '\0') {
                        printf("usage: open host-name [port](目前不支持自定义端口)\n");
                        return -1;
                }
                serverip[strlen(serverip)] = '\0';
        } else {
                strcpy(serverip, ipaddr);
        }

        int sfd_cmd = ftpconnect(serverip, port);
        if (sfd_cmd == -1)
                return -1;

        show_results(sfd_cmd);          // 显示欢迎信息
        printf("Name (%s:tarena): ", ipaddr);
        scanf("%s", username);          // 输入用户名
        //strcpy(username, "csd1803");
        sprintf(cmdbuf, "USER %s\r\n", username);
        do_cmds(sfd_cmd);

        printf("Password:\033[8m\033[?25l");
        scanf("%s", password);          // 输入密码
        printf("\033[0m\033[?25h");
        //strcpy(password, "csd1803");
        sprintf(cmdbuf, "PASS %s\r\n", password);
        do_cmds(sfd_cmd);

        if (!strncmp(rbuf, "230", 3)) {
                // 获取服务器系统类型
                sprintf(cmdbuf, "SYST\r\n");
                do_cmds_background(sfd_cmd);
                sscanf(rbuf, "%*s%s", systype);

                // 设置传输模式(A = ASCII; I = Image (binary))
                sprintf(cmdbuf, "TYPE I\r\n");
                do_cmds_background(sfd_cmd);

                printf("Remote system type is %s.\n", systype);
                printf("Using binary mode to transfer files.\n");

		strcpy(ftpaddr, serverip);      // 将获取到的服务器ip保存起来
                return sfd_cmd;
        }
        printf("Login failed.\n");
        close(sfd_cmd);
        return -1;
}

void ls(const char *pathname, int sfd_cmd, const char *ipaddr)
{
        unsigned short port = getport(sfd_cmd, ipaddr);
        if (!pathname)
                sprintf(cmdbuf, "LIST\r\n");
        else
                sprintf(cmdbuf, "LIST %s\r\n", pathname);
        do_cmds(sfd_cmd);
        int cfd_data = ftpconnect(ipaddr, port);

        // 输出获得的目录信息
        show_results(cfd_data);
        close(cfd_data);

        // 输出服务器响应结果
        show_results(sfd_cmd);
}

void cd(const char *pathname, int sfd_cmd)
{
        char name[256] = {0};
        if (!pathname) {
                printf("(remote-directory) ");
                fgets(name, sizeof(name), stdin);
                strtrim(name);
                if (name[0] == '\0') {
                        printf("usage: cd remote-directory\n");
                        return;
                }
                name[strlen(name)] = '\0';
                sprintf(cmdbuf, "CWD %s\r\n", name);
        } else {
                sprintf(cmdbuf, "CWD %s\r\n", pathname);
        }
        do_cmds(sfd_cmd);
}

void pwd(int sfd_cmd)
{
        sprintf(cmdbuf, "PWD\r\n");
        do_cmds(sfd_cmd);
}

void _mkdir(const char *pathname, int sfd_cmd)
{
        char name[256] = {0};
        if (!pathname) {
                printf("(directory-name) ");
                fgets(name, sizeof(name), stdin);
                strtrim(name);
                if (name[0] == '\0') {
                        printf("usage: mkdir directory-name\n");
                        return;
                }
                name[strlen(name)] = '\0';
                sprintf(cmdbuf, "MKD %s\r\n", name);
        } else {
                sprintf(cmdbuf, "MKD %s\r\n", pathname);
        }
        do_cmds(sfd_cmd);
}

void _rename(const char *srcname, const char *destname, int sfd_cmd)
{
        char name[2][128] = {0};        // name[0]：原文件名，name[1]：新文件名
        const char *p_name = srcname;
        for (int i = 0; i < 2; i++, p_name = destname) {
                if (p_name == NULL) {
                        if (i == 0)
                                printf("(from-name) ");
                        else
                                printf("(to-name) ");
                        fgets(name[i], sizeof(name[i]) - 1, stdin);
                        strtrim(name[i]);
                        if (name[i][0] == '\0')
                                printf("rename from-name to-name\n");
                        name[i][strlen(name[i])] = '\0';
                } else {
                        strcpy(name[i], p_name);
                }
        }
        sprintf(cmdbuf, "RNFR %s\r\n", name[0]);
        do_cmds(sfd_cmd);
        sprintf(cmdbuf, "RNTO %s\r\n", name[1]);
        do_cmds(sfd_cmd);
}

void _delete(const char *filename, int sfd_cmd)
{
        char name[256] = {0};
        if (filename == NULL) {
                printf("(remote-file) ");
                fgets(name, sizeof(name), stdin);
                strtrim(name);
                if (name[0] == '\0') {
                        printf("usage: delete remote-file\n");
                        return;
                }
                name[strlen(name)] = '\0';
                sprintf(cmdbuf, "DELE %s\r\n", name);
        } else {
                sprintf(cmdbuf, "DELE %s\r\n", filename);
        }
        do_cmds(sfd_cmd);
}

void _rmdir(const char *pathname, int sfd_cmd)
{
        char name[256] = {0};
        if (pathname == NULL) {
                printf("(directory-name) ");
                fgets(name, sizeof(name), stdin);
                strtrim(name);
                if (name[0] == '\0') {
                        printf("usage: rmdir directory-name\n");
                        return;
                }
                name[strlen(name)] = '\0';
                sprintf(cmdbuf, "RMD %s\r\n", name);
        } else {
                sprintf(cmdbuf, "RMD %s\r\n", pathname);
        }
        do_cmds(sfd_cmd);
}

void get(const char *filename, int sfd_cmd, const char *ipaddr)
{
        unsigned short port = getport(sfd_cmd, ipaddr);
        // 发送命令
        sprintf(cmdbuf, "RETR %s\r\n", filename);
        do_cmds(sfd_cmd);
        // 建立数据连接
        int cfd_data = ftpconnect(ipaddr, port);
        // 下载文件
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);

        // 重定向标准输出
        int tmpout = dup(1);
        dup2(fd, 1);
        close(fd);
        show_results(cfd_data);
        // 恢复现场
        dup2(tmpout, 1);
        close(tmpout);

        close(cfd_data);
        show_results(sfd_cmd);
}

void put(const char *filename, int sfd_cmd, const char *ipaddr)
{
        unsigned short port = getport(sfd_cmd, ipaddr);
        // 发送命令
        if (strrchr(filename, '/'))
                sprintf(cmdbuf, "STOR %s\r\n", strrchr(filename, '/') + 1);
        else
                sprintf(cmdbuf, "STOR %s\r\n", filename);
        do_cmds(sfd_cmd);
        // 创建数据连接
        int cfd_data = ftpconnect(ipaddr, port);
        // 上传文件
        int fd = open(filename, O_RDONLY);
        int r = 0;
        while (r = read(fd, wbuf, sizeof(wbuf) - 1))
                send(cfd_data, wbuf, r, 0);
        close(fd);
        close(cfd_data);
        show_results(sfd_cmd);
}

void chwkmd(const char *mode)
{
        extern int workmode;
        if (!mode) {
                printf("usage: chwkmd <{PASV|PORT}>\n");
                return;
        }
        if (!strcmp(mode, "PORT"))
                workmode = PORT;
        else if (!strcmp(mode, "PASV"))
                workmode = PASV;
        else
                printf("%s: ERROR work mode.\n", mode);
}

#include "shell.h"
void help(int argc, char *argv[])
{
        if (1 == argc) {
                printf("Commands may be abbreviated.  Commands are:\n\n");
                printf("!\tls\tcd\tpwd\tmkdir\trmdir\n");
                printf("rename\tdelete\tput\tget\tchwkmd\n");
                printf("help\tquit\texit\tbye\topen\n");
                return;
        }
        char *esc = "escape to the shell";
        char *ls = "list contents of remote directory";
        char *cd = "change remote working directory";
        char *pwd = "print working directory on remote machine";
        char *mkdir = "make directory on the remote machine";
        char *rmdir = "remove directory on the remote machine";
        char *rename = "rename file";
        char *delete = "delete remote file";
        char *put = "send one file";
        char *get = "receive file";
        char *chwkmd = "change work mode(PASV or PORT)";
        char *help = "print local help information";
        char *quit = "terminate ftp session and exit";
        char *exit = "terminate ftp session and exit";
        char *bye = "terminate ftp session and exit";
        char *open = "connect to remote ftp";

        char *cmdhelp[] = {esc, ls, cd, pwd, mkdir, rmdir, rename, delete,
                put, get, chwkmd, help, quit, exit, bye, open};
	char *support[] = {"!", "ls", "cd", "pwd", "mkdir", "rmdir",
                "rename", "delete", "put", "get", "chwkmd",
		"help", "quit", "exit", "bye", "open"};
        for (int i = 1; i < argc; ++i) {
                int pos = searchcmd(argv[i], countof(support), support);
                if (-1 == pos)
                        printf("?Invalid help command %s\n", argv[i]);
                else
                        printf("%8s\t\t%s\n", support[pos], cmdhelp[pos]);
        }
}
