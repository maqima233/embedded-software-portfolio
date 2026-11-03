/*
 * 刷题线(第二轮)#62:十六进制行格式化(字节转文本)
 *
 * 题面(独立训练;用户只实现 USER_CODE 区内标注的函数,其余部分不得改动)
 *
 * 函数签名:
 *   int hex_row(const unsigned char *data, int n, char *dst, int dstcap);
 *
 * 输入与范围:
 *   -- data 指向 n 个字节;dst 是容量为 dstcap 字节的可写缓冲区
 *   -- 声明上界:int n 与 int dstcap 都必须落在 0..1024(含端点);
 *      超出上界的输入一律非法(见"边界与非法输入")
 *   -- 前置条件:data 与 dst 指向的区域不得重叠(契约保证,函数无需检测)
 *
 * 输出与返回码(与 snprintf 同款截断契约):
 *   -- 把每个字节格式化为两位大写十六进制,字节之间用一个空格分隔,
 *      末尾无多余空格,整体以 '\0' 结尾;返回"本应写入的字符数"(不含
 *      '\0');n>0 时该值为 3*n-1,如 3 个字节 -> "AF 01 7F",返回 8
 *   -- 完整放得下(dstcap >= 需要字符数+1):全部写入,返回需要字符数
 *   -- 放不下:只写前 dstcap-1 个字符并保证 dst[dstcap-1]=='\0',返回
 *      值仍是需要字符数;调用者用 返回值 >= dstcap 判断截断
 *   -- n==0:合法,dst[0] 写 '\0',返回 0
 *
 * 边界与非法输入:
 *   -- data 为 NULL 且 n>0 -> 返回 -1;n<0 -> 返回 -1
 *   -- dst 为 NULL 或 dstcap <= 0 -> 返回 -1
 *   -- n > 1024 或 dstcap > 1024(含 INT_MAX)-> 返回 -1;这类越界输入
 *      必须在读取 data 之前就判定,返回 -1 时不得触碰 dst
 *
 * 写入范围与失败保持:
 *   -- 返回 -1 时 dst 一个字节都不写
 *   -- 成功/截断时只写 dst[0] 到 '\0' 所在下标,更后面的字节保持原样
 *
 * 内存与副作用:
 *   -- 不分配、不释放内存;不得修改 data 指向的内容
 *
 * 编译运行(项目根目录,GCC;当前 Windows x86-64):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror c-handwritten/algorithms/hex-row-format.c -o output/hex-row-format.exe
 *   ./output/hex-row-format.exe                全部用例
 *   ./output/hex-row-format.exe --case NAME    单用例
 * 验收:严格编译零诊断;教师冻结测试全部通过;教师本地 commit。
 * 建议 15-20 分钟,30 分钟硬止损。
 */
#include <stdio.h>
#include <string.h>

/* ======================= 用户核心区 ======================= */
/* USER_CODE_BEGIN */

char switch_to_16(unsigned char n)
{
    if (n < 10) return (char)('0' + n);
    return (char)('A' + (n - 10));
}

int hex_row(const unsigned char *data, int n, char *dst, int dstcap) {
    if (n > 1024 || dstcap > 1024) return -1;
    if (dstcap <= 0 || !dst) return -1;
    if (n == 0) {*dst = '\0'; return 0;}
    if (n < 0 || !data) return -1;
    int j = 0;
    char tmp[n * 3];
    for (int i = 0; i < n; i++)
    {
        unsigned char high = data[i] / 16;
        unsigned char low = data[i] % 16;
        tmp[j] = switch_to_16(high);
        tmp[j + 1] = switch_to_16(low);
        if (i < n - 1) tmp[j + 2] = ' ';
        j += 3;
    }
    int need = n * 3 >= dstcap ? dstcap : n * 3;
    int i;
    for (i = 0; i < need - 1; i++) dst[i] = tmp[i];
    dst[i] = '\0';
    return n * 3 - 1;
}
/* USER_CODE_END */

/* ======================= 教师冻结测试区(2026-10-09 冻结) ======================= */
/* FROZEN_TESTS_BEGIN */
#include <limits.h>

#define DSTBUF 4096

typedef struct {
    const char *name;
    unsigned char data[1040];  /* 覆盖 n=1025 越界探针,防测试侧被越读 */
    int n;
    int dstcap;
    int null_dst;
    int null_data;
    int want_rc;              /* 需要字符数;负值表示期望 -1 */
    const char *want_full;    /* 独立构造的完整期望串 */
} hex_case_t;

static const hex_case_t cases[] = {
    {"empty",        {0},                        0, 8,  0, 0,  0, ""},
    {"one-zero",     {0x00},                     1, 8,  0, 0,  2, "00"},
    {"one-ff",       {0xFF},                     1, 8,  0, 0,  2, "FF"},
    {"three-mixed",  {0xAF, 0x01, 0x7F},         3, 16, 0, 0,  8, "AF 01 7F"},
    {"exact-fit",    {0xAF, 0x01, 0x7F},         3, 9,  0, 0,  8, "AF 01 7F"},
    {"one-short",    {0xAF, 0x01, 0x7F},         3, 8,  0, 0,  8, "AF 01 7"},
    {"two-short",    {0xAF, 0x01, 0x7F},         3, 3,  0, 0,  8, "AF"},
    {"cap-two",      {0xAF, 0x01, 0x7F},         3, 2,  0, 0,  8, "A"},
    {"cap-one",      {0xAF, 0x01, 0x7F},         3, 1,  0, 0,  8, ""},
    {"cap-zero",     {0xAF},                     1, 0,  0, 0, -1, ""},
    {"null-dst",     {0xAF},                     1, 8,  1, 0, -1, ""},
    {"null-data",    {0},                        1, 8,  0, 1, -1, ""},
    {"null-data-n0", {0},                        0, 8,  0, 1,  0, ""},
    {"n-neg",        {0},                       -1, 8,  0, 0, -1, ""},
    {"n-1025",       {0xAA},                  1025, 16, 0, 0, -1, ""},
    {"n-intmax",     {0xAA},                INT_MAX, 16, 0, 0, -1, ""},
    {"cap-1025",     {0xAA},                     1, 1025, 0, 0, -1, ""},
    {"cap-intmax",   {0xAA},                     1, INT_MAX, 0, 0, -1, ""},
    {"full-range",   {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F},
                     16, 64, 0, 0, 47, "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F"},
    {"deadbeef",     {0xDE, 0xAD, 0xBE, 0xEF},   4, 16, 0, 0, 11, "DE AD BE EF"},
    {"uppercase",    {0xAB, 0xCD},               2, 8,  0, 0,  5, "AB CD"}
};

/* 独立构造完整期望串(教师侧;不调用被测函数),用于 fixtures 交叉核对 */
static void build_hex(const unsigned char *data, int n, char *out) {
    static const char digits[] = "0123456789ABCDEF";
    int pos = 0;
    for (int i = 0; i < n; ++i) {
        if (i > 0) out[pos++] = ' ';
        out[pos++] = digits[(data[i] >> 4) & 0x0F];
        out[pos++] = digits[data[i] & 0x0F];
    }
    out[pos] = '\0';
}

static int verify_fixtures(void) {
    int bad = 0;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        const hex_case_t *c = &cases[i];
        if (c->want_rc != -1 && !c->null_dst) {
            char full[64];
            build_hex(c->null_data ? (const unsigned char *)"" : c->data, c->n, full);
            int content_len = c->want_rc;
            if (content_len > c->dstcap - 1) content_len = c->dstcap - 1;
            if (strlen(c->want_full) != (size_t)content_len) {
                printf("FIXTURE FAIL %s: expected-prefix length %zu != content %d (full \"%s\")\n",
                       c->name, strlen(c->want_full), content_len, full);
                ++bad;
            } else if (strncmp(full, c->want_full, (size_t)content_len) != 0) {
                printf("FIXTURE FAIL %s: constructed \"%s\" prefix != frozen \"%s\"\n",
                       c->name, full, c->want_full);
                ++bad;
            }
            int want_rc = c->n == 0 ? 0 : 3 * c->n - 1;
            if (c->want_rc != want_rc) {
                printf("FIXTURE FAIL %s: want_rc %d != 3n-1 %d\n", c->name, c->want_rc, want_rc);
                ++bad;
            }
        }
    }
    printf("Teacher fixture review: %d/%zu PASS\n",
           bad == 0 ? (int)(sizeof cases / sizeof cases[0]) : 0, sizeof cases / sizeof cases[0]);
    return bad == 0 ? 0 : 1;
}

static int run_case(const hex_case_t *c) {
    char data_copy[1040];
    char dst[DSTBUF];
    memset(dst, 0x5A, sizeof dst);
    const unsigned char *data_eff = c->null_data ? NULL : c->data;
    memcpy(data_copy, c->data, sizeof data_copy);
    printf("RUN %s n=%d dstcap=%d%s\n", c->name, c->n, c->dstcap, c->null_dst ? " dst=(NULL)" : "");
    int got = hex_row(data_eff, c->n, c->null_dst ? NULL : dst, c->dstcap);
    int ok = 1;
    if (got != c->want_rc) {
        printf("FAIL %s: input n=%d dstcap=%d; expected rc=%d, got rc=%d\n",
               c->name, c->n, c->dstcap, c->want_rc, got);
        ok = 0;
    }
    if (memcmp(data_copy, c->data, sizeof data_copy) != 0) {
        printf("FAIL %s: input data modified (immutability)\n", c->name);
        ok = 0;
    }
    if (c->null_dst) {
        printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
        return ok;
    }
    if (c->want_rc < 0) {
        for (int i = 0; i < DSTBUF; ++i) {
            if ((unsigned char)dst[i] != 0x5A) {
                printf("FAIL %s: dst[%d] written on error path (want untouched)\n", c->name, i);
                ok = 0;
                break;
            }
        }
        printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
        return ok;
    }
    int content_len = c->want_rc;
    if (content_len > c->dstcap - 1) content_len = c->dstcap - 1;
    for (int i = 0; i < content_len; ++i) {
        if (dst[i] != c->want_full[i]) {
            printf("FAIL %s: content[%d]; expected '%c', got 0x%02X (first mismatch)\n",
                   c->name, i, c->want_full[i], (unsigned char)dst[i]);
            ok = 0;
            break;
        }
    }
    if (dst[content_len] != '\0') {
        printf("FAIL %s: terminator at [%d]; expected '\\0', got 0x%02X\n",
               c->name, content_len, (unsigned char)dst[content_len]);
        ok = 0;
    }
    for (int i = content_len + 1; i < DSTBUF; ++i) {
        if ((unsigned char)dst[i] != 0x5A) {
            printf("FAIL %s: extra write at dst[%d] (want untouched)\n", c->name, i);
            ok = 0;
            break;
        }
    }
    printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
    return ok;
}

/* 上界内(dstcap <= 1024)的两个边界成功用例(程序化构造,单独一例) */
static int run_boundary_1024(void) {
    unsigned char data[1024];
    char dst[DSTBUF];
    memset(data, 0xA5, sizeof data);
    int ok = 1;

    /* 用例 1:n=341,need=1022,dstcap=1023 —— 恰好完整放下 */
    memset(dst, 0x5A, sizeof dst);
    printf("RUN n-341-exact n=341 dstcap=1023\n");
    int got = hex_row(data, 341, dst, 1023);
    if (got != 1022) {
        printf("FAIL n-341-exact: expected rc=1022, got rc=%d\n", got);
        ok = 0;
    } else {
        for (int i = 0; i < 1022; ++i) {
            char wantc = (i % 3 == 0) ? 'A' : (i % 3 == 1) ? '5' : ' ';
            if (dst[i] != wantc) {
                printf("FAIL n-341-exact: dst[%d]; expected '%c', got 0x%02X\n",
                       i, wantc, (unsigned char)dst[i]);
                ok = 0;
                break;
            }
        }
        if (dst[1022] != '\0') {
            printf("FAIL n-341-exact: terminator; expected '\\0' at [1022], got 0x%02X\n",
                   (unsigned char)dst[1022]);
            ok = 0;
        }
        if ((unsigned char)dst[1023] != 0x5A) {
            printf("FAIL n-341-exact: wrote beyond dstcap at [1023]\n");
            ok = 0;
        }
    }

    /* 用例 2:n=1024(上界),need=3071,dstcap=1024 —— 截断,rc 仍是 3071 */
    memset(dst, 0x5A, sizeof dst);
    printf("RUN n-1024-trunc n=1024 dstcap=1024\n");
    int got2 = hex_row(data, 1024, dst, 1024);
    if (got2 != 3071) {
        printf("FAIL n-1024-trunc: expected rc=3071, got rc=%d\n", got2);
        ok = 0;
    } else {
        for (int i = 0; i < 1023; ++i) {
            char wantc = (i % 3 == 0) ? 'A' : (i % 3 == 1) ? '5' : ' ';
            if (dst[i] != wantc) {
                printf("FAIL n-1024-trunc: dst[%d]; expected '%c', got 0x%02X\n",
                       i, wantc, (unsigned char)dst[i]);
                ok = 0;
                break;
            }
        }
        if (dst[1023] != '\0') {
            printf("FAIL n-1024-trunc: terminator; expected '\\0' at [1023], got 0x%02X\n",
                   (unsigned char)dst[1023]);
            ok = 0;
        }
        if ((unsigned char)dst[1024] != 0x5A) {
            printf("FAIL n-1024-trunc: wrote beyond dstcap at [1024]\n");
            ok = 0;
        }
    }
    printf(ok ? "PASS n-1024-boundary\n" : "FAIL n-1024-boundary (see above)\n");
    return ok;
}

int main(int argc, char **argv) {
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    if (argc == 2 && strcmp(argv[1], "--verify-fixtures") == 0)
        return verify_fixtures();
    const char *sel = NULL;
    if (argc == 3 && strcmp(argv[1], "--case") == 0) {
        sel = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [--case NAME | --verify-fixtures]\n", argv[0]);
        return 2;
    }
    int total = 0, passed = 0;
    size_t n = sizeof cases / sizeof cases[0];
    for (size_t i = 0; i < n; ++i) {
        if (sel != NULL && strcmp(sel, cases[i].name) != 0) continue;
        ++total;
        passed += run_case(&cases[i]);
    }
    if (sel == NULL || strcmp(sel, "n-1024-full") == 0) {
        ++total;
        passed += run_boundary_1024();
    }
    if (total == 0) {
        fprintf(stderr, "Unknown case: %s\n", sel);
        return 2;
    }
    printf("%d/%d PASS\n", passed, total);
    return passed == total ? 0 : 1;
}
/* FROZEN_TESTS_END */
