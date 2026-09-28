/* 十六进制字符串解析练习。 */

#include <stdio.h>
#include <limits.h>
/* 按需补:#include <string.h> / <stdlib.h> / <stdint.h> */

int hex_to_int(const char *s) {
    const char *cur = s;
    const char *last = s;
    if (*cur == 0) return -1;
    if (*s == '0' && (*(s + 1) == 'x' || *(s + 1) == 'X')) {
        last = s + 2;
        cur = s + 2;
    }
    if (*last == 0) return -1;
    while ((*last >= '0' && *last <= '9') || (*last >= 'a' && *last <= 'f') || (*last >= 'A' && *last <= 'F')) last++;
    if (*last == '\0') {
        int result = 0;
        while(*cur != '\0') {
            if (*cur >= '0' && *cur <= '9') result = result * 16 + *cur - '0';
            else if ((*cur >= 'a' && *cur <= 'f')) result = result * 16 + *cur - 'a' + 10;
            else result = result * 16 + *cur - 'A' + 10;
            cur++;
        }
        return result;
    }
    else return -1;
}

typedef struct {
    const char *input;
    int expected;
} hex_test_case_t;

int main(void)
{
    const hex_test_case_t cases[] = {
        { "123",     291 },
        { "0x123",   291 },
        { "0X123",   291 },
        { "0x00123", 291 },
        { "00123",   291 },
        { "0xff",    255 },
        { "0XFF",    255 },
        { "0",         0 },
        { "",         -1 },
        { "0x",       -1 },
        { "0X",       -1 },
        { "0xxxx",    -1 },
        { "+=-=",     -1 },
        { "1x2",      -1 },
        { "12g",      -1 },
    };

    const size_t case_count = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < case_count; i++) {
        int actual = hex_to_int(cases[i].input);
        if (actual != cases[i].expected) {
            printf("FAIL input=\"%s\" expected=%d actual=%d\n",
                   cases[i].input, cases[i].expected, actual);
            return 1;
        }
    }

    printf("ALL PASS (%zu/%zu)\n", case_count, case_count);
    return 0;
}
