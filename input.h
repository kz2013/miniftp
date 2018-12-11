#ifndef __INPUT_H_
#define __INPUT_H_

#include <sys/ioctl.h>

int getwinsize(int fd);
char *fgets_input(char *s, int size, FILE *stream, int prompt_len);

#endif  // __INPUT_H_
