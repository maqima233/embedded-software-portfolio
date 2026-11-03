/*
 * 刷题线(第二轮)#53:最小栈(数组模拟,辅助数组同步)
 *
 * 题面(独立训练;用户只实现 USER_CODE 区内标注的四个函数,其余部分不得改动)
 *
 * 函数签名:
 *   void ms_init(minstack_t *s, int *data, int *mins, int cap);
 *   int  ms_push(minstack_t *s, int v);
 *   int  ms_pop(minstack_t *s, int *out);
 *   int  ms_min(const minstack_t *s, int *out);
 *
 * 结构 minstack_t(见下方定义,不得修改):
 *   -- data:主栈存储,容量 cap,调用者提供;ms_init 后 data[0..top-1] 为
 *     自底向上的栈内容
 *   -- mins:与 data 平行使用的辅助数组,容量 cap,调用者提供
 *   -- cap:容量(>=1);top:当前元素个数(0..cap)
 *
 * 输入与范围:
 *   -- 调用者保证:cap >= 1、s/data/mins/out 均非 NULL、data 与 mins 各
 *     有 cap 个 int 空间;函数不做契约外防御
 *   -- 元素值域为任意 int(含负数与 INT_MIN)
 *
 * 输出与返回码:
 *   -- ms_init:绑定存储,置空栈(top=0)
 *   -- ms_push:成功返回 0;满(top==cap)返回 -1
 *   -- ms_pop:成功返回 0 并把弹出的栈顶值写 *out;空返回 -1 且不碰 *out
 *   -- ms_min:成功返回 0 并把当前栈内最小值写 *out;空返回 -1 且不碰 *out
 *   -- 0/-1 是状态码,不携带业务值,不会与元素值混淆
 *   -- 例:依次 push 5,3,7,2 后 min 为 2;pop 掉 2 后 min 退回 3
 *
 * 边界与失败保持:
 *   -- 满栈 push 返回 -1 时不得改动栈状态与存储
 *   -- 空栈 pop/min 返回 -1 时不得写 *out
 *
 * 内存与副作用:
 *   -- 不分配、不释放内存;只使用调用者提供的 data 与 mins;栈内元素
 *      数据只写 data 的 [0, top) 区间
 *
 * 编译运行(项目根目录,GCC;当前 Windows x86-64):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror c-handwritten/algorithms/min-stack.c -o output/min-stack.exe
 *   ./output/min-stack.exe                全部用例
 *   ./output/min-stack.exe --case NAME    单用例
 * 验收:严格编译零诊断;教师冻结测试全部通过;教师本地 commit。
 * 建议 15-20 分钟,30 分钟硬止损。
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int *data;   /* 主栈存储,容量 cap,调用者提供 */
    int *mins;   /* 辅助数组,容量 cap,与 data 平行使用 */
    int  cap;    /* 容量 */
    int  top;    /* 当前元素个数,0..cap */
} minstack_t;

/* ======================= 用户核心区 ======================= */
/* USER_CODE_BEGIN */
void ms_init(minstack_t *s, int *data, int *mins, int cap) {
    s->data = data;
    s->mins = mins;
    s->cap = cap;
    s->top = 0;
}

int ms_push(minstack_t *s, int v) {
    if (s->cap == s->top) return -1;
    if (!s->top) s->mins[s->top] = v;
    else
    {
        if (v <= s->mins[s->top - 1]) s->mins[s->top] = v;
        else s->mins[s->top] = s->mins[s->top - 1];
    }
    s->data[s->top] = v;
    s->top++;
    return 0;
}

int ms_pop(minstack_t *s, int *out) {
    if (!s->top) return -1;
    *out = s->data[s->top - 1];
    s->top--;
    return 0;
}

int ms_min(const minstack_t *s, int *out) {
    if (!s->top) return -1;
    *out = s->mins[s->top - 1];
    return 0;
}
/* USER_CODE_END */

/* ======================= 教师冻结测试区(2026-10-09 冻结) ======================= */
/* FROZEN_TESTS_BEGIN */
#define OP_MAX 24

typedef struct {
    const char *name;
    int cap;
    char ops[OP_MAX];   /* 'p'=成功push 'P'=满栈push(期望-1) 'o'=成功pop 'O'=空pop(-1)
                           'm'=成功min 'M'=空min(-1) */
    int args[OP_MAX];   /* 'p':入栈值;'o'/'m':期望写出值(失败操作忽略) */
    int nops;
} ms_case_t;

static const ms_case_t cases[] = {
    {"empty-queries", 4, "OM", {0, 0}, 2},
    {"decreasing", 4,
     "pppp" "mo" "mo" "mo" "mo" "MO",
     {5, 3, 7, 2, 2, 2, 3, 7, 3, 3, 5, 5, 0, 0}, 14},
    {"increasing", 4,
     "pm" "pm" "pm",
     {1, 1, 2, 1, 3, 1}, 6},
    {"duplicate-min", 4,
     "pp" "m" "o" "m" "o" "M",
     {3, 3, 3, 3, 3, 3, 0}, 7},
    {"cap-one", 1,
     "p" "P" "m" "o" "O" "M",
     {42, 43, 42, 42, 0, 0}, 6},
    {"int-min", 4,
     "p" "m" "p" "m" "o" "m" "o" "O",
     {INT_MIN, INT_MIN, 0, INT_MIN, 0, INT_MIN, INT_MIN, 0}, 8},
    {"alternating", 2,
     "po" "po" "po",
     {1, 1, 2, 2, 3, 3}, 6},
    {"max-values", 4,
     "ppp" "m" "o" "m" "o" "m" "o" "O",
     {INT_MAX, INT_MIN, 0, INT_MIN, 0, INT_MIN, INT_MIN, INT_MAX, INT_MAX, 0}, 10},
    {"full-cap4", 4,
     "pppp" "P" "m" "o" "p" "m" "P" "o" "o" "o" "o" "O" "M",
     {1, 2, 3, 4, 9, 1, 4, 9, 1, 9, 9, 3, 2, 1, 0, 0}, 16},
    {"dup-min-cross", 4,
     "pppp" "mo" "mo" "mo" "mo" "OM",
     {2, 1, 1, 3, 1, 3, 1, 1, 1, 1, 2, 2, 0, 0}, 14}
};

static int run_ms_case(const ms_case_t *c) {
    int data[8];
    int mins[8];
    int sentinel_probe = -777111;
    for (int i = 0; i < 8; ++i) { data[i] = sentinel_probe; mins[i] = sentinel_probe; }
    minstack_t s;
    memset(&s, 0, sizeof s);
    int model[8];      /* 独立记录的栈内容(自底向上) */
    int model_top = 0;

    printf("RUN %s cap=%d ops=%.*s\n", c->name, c->cap, c->nops, c->ops);
    int ok = 1;
    ms_init(&s, data, mins, c->cap);
    if (s.data != data || s.mins != mins || s.cap != c->cap || s.top != 0) {
        printf("FAIL %s init: expected data/mins bound, cap=%d, top=0; got data-bound=%d mins-bound=%d cap=%d top=%d\n",
               c->name, c->cap, s.data == data, s.mins == mins, s.cap, s.top);
        ok = 0;
    }

    for (int i = 0; i < c->nops; ++i) {
        char op = c->ops[i];
        int arg = c->args[i];
        int got = 0, rc = 0;
        int out = sentinel_probe;
        switch (op) {
        case 'p':
            rc = ms_push(&s, arg);
            if (rc != 0) {
                printf("FAIL %s op#%d push(%d): expected rc=0, got rc=%d\n", c->name, i, arg, rc);
                ok = 0;
            } else {
                model[model_top++] = arg;
                if (s.top != model_top) {
                    printf("FAIL %s op#%d push(%d): expected top=%d, got top=%d\n",
                           c->name, i, arg, model_top, s.top);
                    ok = 0;
                }
                if (model_top > 0 && data[model_top - 1] != arg) {
                    printf("FAIL %s op#%d push(%d): expected data[%d]=%d, got %d\n",
                           c->name, i, arg, model_top - 1, arg, data[model_top - 1]);
                    ok = 0;
                }
            }
            break;
        case 'P':
            rc = ms_push(&s, arg);
            if (rc != -1) {
                printf("FAIL %s op#%d full-push(%d): expected rc=-1, got rc=%d\n", c->name, i, arg, rc);
                ok = 0;
            } else if (s.top != model_top) {
                printf("FAIL %s op#%d full-push(%d): state changed; expected top=%d, got top=%d\n",
                       c->name, i, arg, model_top, s.top);
                ok = 0;
            }
            break;
        case 'o':
            rc = ms_pop(&s, &out);
            if (rc != 0) {
                printf("FAIL %s op#%d pop: expected rc=0, got rc=%d\n", c->name, i, rc);
                ok = 0;
            } else {
                int want = model_top > 0 ? model[model_top - 1] : 0;
                if (model_top > 0) --model_top;
                if (out != want) {
                    printf("FAIL %s op#%d pop output: expected %d, got %d\n", c->name, i, want, out);
                    ok = 0;
                }
                if (s.top != model_top) {
                    printf("FAIL %s op#%d pop: expected top=%d, got top=%d\n", c->name, i, model_top, s.top);
                    ok = 0;
                }
            }
            break;
        case 'O':
            rc = ms_pop(&s, &out);
            if (rc != -1) {
                printf("FAIL %s op#%d empty-pop: expected rc=-1, got rc=%d\n", c->name, i, rc);
                ok = 0;
            } else if (out != sentinel_probe) {
                printf("FAIL %s op#%d empty-pop: *out must be untouched; expected sentinel %d, got %d\n",
                       c->name, i, sentinel_probe, out);
                ok = 0;
            }
            break;
        case 'm':
            got = ms_min(&s, &out);
            if (got != 0) {
                printf("FAIL %s op#%d min: expected rc=0, got rc=%d\n", c->name, i, got);
                ok = 0;
            } else if (out != arg) {
                printf("FAIL %s op#%d min output: expected %d, got %d\n", c->name, i, arg, out);
                ok = 0;
            }
            break;
        case 'M':
            got = ms_min(&s, &out);
            if (got != -1) {
                printf("FAIL %s op#%d empty-min: expected rc=-1, got rc=%d\n", c->name, i, got);
                ok = 0;
            } else if (out != sentinel_probe) {
                printf("FAIL %s op#%d empty-min: *out must be untouched; expected sentinel %d, got %d\n",
                       c->name, i, sentinel_probe, out);
                ok = 0;
            }
            break;
        default:
            printf("FAIL %s op#%d: bad fixture op '%c'\n", c->name, i, op);
            ok = 0;
            break;
        }
        if (s.data != data || s.mins != mins || s.cap != c->cap) {
            printf("FAIL %s op#%d: storage binding changed (data=%d mins=%d cap=%d)\n",
                   c->name, i, s.data == data, s.mins == mins, s.cap);
            ok = 0;
        }
        if (s.top < 0 || s.top > c->cap) {
            printf("FAIL %s op#%d: top out of range [0,%d]; got %d\n", c->name, i, c->cap, s.top);
            ok = 0;
        }
    }
    printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
    return ok;
}

int main(int argc, char **argv) {
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    const char *sel = NULL;
    if (argc == 3 && strcmp(argv[1], "--case") == 0) {
        sel = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [--case NAME]\n", argv[0]);
        return 2;
    }
    int total = 0, passed = 0;
    size_t n = sizeof cases / sizeof cases[0];
    for (size_t i = 0; i < n; ++i) {
        if (sel != NULL && strcmp(sel, cases[i].name) != 0) continue;
        ++total;
        passed += run_ms_case(&cases[i]);
    }
    if (total == 0) {
        fprintf(stderr, "Unknown case: %s\n", sel);
        return 2;
    }
    printf("%d/%d PASS\n", passed, total);
    return passed == total ? 0 : 1;
}
/* FROZEN_TESTS_END */
