#include <string.h>
#include <ctype.h>
#include "strkit.h"

char *strtrim(char *s)
{
        int first = 0, last = strlen(s);
        if (s == NULL || first == last)
                return s;
        for (; first < last && isspace(*(s + first)); first++);

        if (first == last) {     // 待修剪字符串全部由空白符构成
                *s = '\0';
                return s;
        }
        // 经过循环，first和last分别表示第一个和最后一个非空白字符的位置
        for (last--; first < last && isspace(*(s + last)); last--);

        *(s + (++last)) = '\0';
        return memmove(s, s + first, last - first + 1);
}
