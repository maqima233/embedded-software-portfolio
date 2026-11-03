/*
 * 刷题线(第二轮)#54:反转 32 位位序
 *
 * 题面(独立训练;用户只实现 USER_CODE 区内标注的函数,其余部分不得改动)
 *
 * 函数签名:
 *   uint32_t reverse_bits(uint32_t x);
 *
 * 输入与范围:
 *   -- x 为任意 32 位无符号值(0x00000000 ~ 0xFFFFFFFF)
 *
 * 输出与返回码:
 *   -- 把 x 的 32 个位完全镜像:第 0 位与第 31 位互换,第 1 位与第 30 位
 *      互换,依此类推;返回镜像后的值
 *   -- 例:0x00000001 -> 0x80000000;0x12345678 -> 0x1E6A2C48;
 *          0xAAAAAAAA -> 0x55555555
 *
 * 边界情况:
 *   -- 0 与 0xFFFFFFFF 都是位回文,返回自身
 *   -- 有些值反转后等于自身(如 0x80000001),返回自身
 *
 * 内存与副作用:
 *   -- 无内存操作;不得使用全局状态
 *
 * 方法约束(沿用原题,不改):
 *   -- 逐位镜像:一位一位从旧值取出搬进新值;
 *   -- 禁止借助"字节交换"捷径(即 bswap 一族的做法),禁止查表法,
 *      禁止 __builtin 内建函数;
 *   -- 全程只使用 uint32_t 运算。
 *
 * 编译运行(项目根目录,GCC;当前 Windows x86-64):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror c-handwritten/algorithms/reverse-bits.c -o output/reverse-bits.exe
 *   ./output/reverse-bits.exe                全部用例
 *   ./output/reverse-bits.exe --case NAME    单用例
 * 验收:严格编译零诊断;教师冻结测试全部通过;教师本地 commit。
 * 建议 15-20 分钟,30 分钟硬止损。
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* ======================= 用户核心区 ======================= */
/* USER_CODE_BEGIN */
uint32_t reverse_bits(uint32_t x) {
    uint32_t result = 0;
    for (uint32_t i = 0; i < 32; i++)
    {
        result = result << 1;
        result |= x & UINT32_C(1);
        x = x >> 1;
    }
    return result;
}
/* USER_CODE_END */

/* ======================= 教师冻结测试区(2026-10-09 冻结) ======================= */
/* FROZEN_TESTS_BEGIN */
typedef struct {
    const char *name;
    uint32_t x;
    uint32_t want;
} bits_case_t;

static const bits_case_t cases[] = {
    {"zero",          0x00000000u, 0x00000000u},
    {"all-ones",      0xFFFFFFFFu, 0xFFFFFFFFu},
    {"lowest-bit",    0x00000001u, 0x80000000u},
    {"highest-bit",   0x80000000u, 0x00000001u},
    {"second-bit",    0x00000002u, 0x40000000u},
    {"mixed-1",       0x12345678u, 0x1E6A2C48u},
    {"alt-aa",        0xAAAAAAAAu, 0x55555555u},
    {"alt-55",        0x55555555u, 0xAAAAAAAAu},
    {"low-half",      0x0000FFFFu, 0xFFFF0000u},
    {"high-half",     0xFFFF0000u, 0x0000FFFFu},
    {"bit-palindrome",0x80000001u, 0x80000001u},
    {"mid-byte",      0x0000FF00u, 0x00FF0000u},
    {"deadbeef",      0xDEADBEEFu, 0xF77DB57Bu},
    {"one-nibble",    0x0000000Fu, 0xF0000000u},
    {"top-nibble",    0xF0000000u, 0x0000000Fu}
};

static int verify_fixtures(void) {
    int bad = 0;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        /* 独立位循环重算,只用于夹具自检(不调用被测函数) */
        uint32_t r = 0u;
        for (int b = 0; b < 32; ++b) {
            if ((cases[i].x >> b) & 1u) r |= (uint32_t)1u << (31 - b);
        }
        if (r != cases[i].want) {
            printf("FIXTURE FAIL %s: x=0x%08X recomputed 0x%08X != frozen 0x%08X\n",
                   cases[i].name, cases[i].x, r, cases[i].want);
            ++bad;
        }
    }
    printf("Teacher fixture review: %d/%zu PASS\n",
           bad == 0 ? (int)(sizeof cases / sizeof cases[0]) : 0,
           sizeof cases / sizeof cases[0]);
    return bad == 0 ? 0 : 1;
}

static int run_case(const bits_case_t *c) {
    printf("RUN %s x=0x%08X\n", c->name, c->x);
    int ok = 1;
    uint32_t got = reverse_bits(c->x);
    if (got != c->want) {
        printf("FAIL %s: input x=0x%08X; expected 0x%08X, got 0x%08X\n",
               c->name, c->x, c->want, got);
        ok = 0;
    }
    uint32_t twice = reverse_bits(got);
    if (twice != c->x) {
        printf("FAIL %s: double reverse; expected 0x%08X, got 0x%08X\n", c->name, c->x, twice);
        ok = 0;
    }
    printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
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
    if (total == 0) {
        fprintf(stderr, "Unknown case: %s\n", sel);
        return 2;
    }
    printf("%d/%d PASS\n", passed, total);
    return passed == total ? 0 : 1;
}
/* FROZEN_TESTS_END */
