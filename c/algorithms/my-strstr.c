/* 朴素子串查找：返回最左侧匹配的起始地址。 */

#include <stddef.h>
#include <stdio.h>
/* 按需补:#include <string.h> / <limits.h> / <stdlib.h> / <stdint.h> */

char *my_strstr(const char *haystack, const char *needle) {
    if (!needle || !haystack) return NULL;
    if (!*needle) return (char *)haystack;
    const char *a = haystack;
    const char *b = needle;
    while (*a) {
        if (*a == *b) {
            const char *a_hold = a;
            const char *b_hold = b;
            while (*a == *b && *b && *a) {
                a++;
                b++;
            }
            if (!*b) return (char *)a_hold;
            a = a_hold;
            b = b_hold;
        }
        a++;
    }
    return NULL;
}

typedef struct {
    const char *haystack;
    const char *needle;
    ptrdiff_t expected_offset;
} test_case_t;

int main(void) {
    static const test_case_t cases[] = {
        {"a",       "a",      0},
        {"abb",     "a",      0},
        {"aaaaabc", "abc",    4},
        {"aaaabcccc", "abc",  3},
        {"aaaa",    "aaaaaa", -1},
        {"aaaa",    "aa",     0},
        {NULL,       NULL,      -1},
        {"aaaa",    "",       0},
        {"abc",     "abd",   -1},

        /* NULL 输入约定。 */
        {NULL,       "a",      -1},
        {"a",       NULL,     -1},
    };

    size_t passed = 0;
    const size_t case_count = sizeof cases / sizeof cases[0];

    for (size_t i = 0; i < case_count; i++) {
        char *actual = my_strstr(cases[i].haystack, cases[i].needle);
        ptrdiff_t actual_offset = -1;

        if (actual != NULL && cases[i].haystack != NULL) {
            actual_offset = actual - cases[i].haystack;
        }

        const int expected_is_null = cases[i].expected_offset < 0;
        const int ok = expected_is_null
                           ? actual == NULL
                           : actual != NULL &&
                                 actual_offset == cases[i].expected_offset;
        printf("case %zu: expected=%td actual=%td %s\n",
               i + 1,
               cases[i].expected_offset,
               actual_offset,
               ok ? "PASS" : "FAIL");
        if (ok) {
            passed++;
        }
    }

    printf("%zu/%zu PASS\n", passed, case_count);
    return passed == case_count ? 0 : 1;
}
