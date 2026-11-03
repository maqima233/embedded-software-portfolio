/*
 * 刷题线(第二轮)#55:动态数组扩容(二级指针+realloc)
 *
 * 题面(独立训练;用户只实现 USER_CODE 区内标注的函数,其余部分不得改动)
 *
 * 函数签名:
 *   int ensure_capacity(int **arr, int *cap, int need);
 *
 * 输入与范围:
 *   -- *arr 为堆数组首指针(可以为 NULL),*cap 为当前容量(以 int 个数计)
 *   -- need 为本次需要的元素个数;题目保证 need <= 1000000,容量与字节数
 *      计算都在此范围内,无溢出责任
 *
 * 输出与返回码:
 *   -- 保证 *arr 指向的数组容量装得下 need 个 int;成功返回 0
 *   -- 目标容量(写死):起点 base 为(*cap > 0 时取 *cap,否则取 4);
 *      目标容量是 base × 2^k 序列中第一个 >= need 的值。注意序列值不必是
 *      2 的幂:base 为 3 时序列是 3 -> 6 -> 12 ...
 *   -- *arr 为 NULL 时一律视为全新分配(忽略旧 *cap,base 取 4)
 *   -- 资源契约:每次进入扩容路径的调用,对 realloc 只做一次尝试——一次
 *      调用,请求大小为目标容量的字节数;不得拆成多次中间扩容
 *   -- 成功扩容时 *arr 与 *cap 更新为新数组与新容量
 *
 * 边界与非法输入(判定优先级按此顺序):
 *   1) arr 或 cap 为 NULL -> 返回 -1
 *   2) need <= 0 -> 返回 -1
 *   3) *cap < 0 -> 返回 -1(即使 *arr 为 NULL,负容量也是非法状态)
 *   4) *arr == NULL -> 走全新分配(忽略旧 *cap)
 *   5) need <= 当前容量 -> 快路径,返回 0,不调用 realloc、不动任何内存
 *   6) 其他 -> 扩容
 *
 * 失败保持:
 *   -- realloc 失败(返回 NULL)时返回 -1,且 *arr 与 *cap 必须保持调用
 *      前的原值,原数组内容完好(失败路径不得丢失原指针)
 *   -- 任何返回 -1 的路径都不得修改 *arr、*cap 指向的值与原数组内容
 *
 * 内存归属:
 *   -- 失败时旧内存仍归调用者所有,由调用者释放;成功时旧指针经由
 *      realloc 语义处理,新指针交回调用者,最终由调用者 free
 *   -- 本函数不调用 free;不分配额外内存
 *
 * 编译运行(项目根目录,GCC;当前 Windows x86-64):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror c-handwritten/algorithms/dyn-array-grow.c -o output/dyn-array-grow.exe
 *   ./output/dyn-array-grow.exe                全部用例
 *   ./output/dyn-array-grow.exe --case NAME    单用例
 * 验收:严格编译零诊断;教师冻结测试全部通过;教师本地 commit。
 * 建议 15-20 分钟,30 分钟硬止损。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ---- 测试用 realloc 拦截声明:只作用于用户核心区的 realloc 调用 ---- */
static void *t_realloc_impl(void *p, size_t n);
#define realloc(p, n) t_realloc_impl(p, n)

/* ======================= 用户核心区 ======================= */
/* USER_CODE_BEGIN */
int ensure_capacity(int **arr, int *cap, int need) {
    if (!arr || !cap || need <= 0 || *cap < 0) return -1;
    int base = (!*arr || *cap == 0) ? 4 : *cap;
    if (base >= need && *arr && *cap) return 0;
    while (base < need) base *= 2;
    int *new_arr = realloc(*arr, (size_t)base * sizeof **arr);
    if (!new_arr) return -1;
    *arr = new_arr;
    *cap = base;
    return 0;
}
/* USER_CODE_END */

/* ======================= 教师冻结测试区(2026-10-09 冻结) ======================= */
/* FROZEN_TESTS_BEGIN */
#undef realloc

/* ---- 测试侧块账本:以 uintptr_t 快照登记测试可见的堆块,防止悬垂指针比较 ---- */
#define BLK_MAX 16
typedef struct {
    uintptr_t addr;   /* 地址快照;绝不通过该字段解引用 */
    size_t bytes;
    int live;
} tblock_t;

static tblock_t g_blocks[BLK_MAX];
static int g_block_n;
static int g_armed_fail;      /* 1 表示下一次 realloc 注入失败 */
static int g_attempts;        /* 用户区调用 realloc 的次数 */
static size_t g_last_request; /* 最近一次 realloc 请求的字节数 */
static int g_bad_realloc;     /* realloc 收到非存活块指针的次数 */

static int blk_find(uintptr_t a) {
    for (int i = 0; i < g_block_n; ++i) {
        if (g_blocks[i].addr == a) return i;
    }
    return -1;
}

static void blk_register(void *p, size_t bytes) {
    if (p == NULL) return;
    int i = blk_find((uintptr_t)p);
    if (i < 0) {
        if (g_block_n >= BLK_MAX) { printf("FIXTURE FAIL: block ledger full\n"); return; }
        i = g_block_n++;
    }
    g_blocks[i].addr = (uintptr_t)p;
    g_blocks[i].bytes = bytes;
    g_blocks[i].live = 1;
}

static void blk_mark_freed(uintptr_t a) {
    int i = blk_find(a);
    if (i >= 0) g_blocks[i].live = 0;
}

static int blk_live(uintptr_t a) {
    int i = blk_find(a);
    return i >= 0 && g_blocks[i].live;
}

static size_t blk_bytes(uintptr_t a) {
    int i = blk_find(a);
    return i >= 0 ? g_blocks[i].bytes : 0;
}

static int blk_live_count(void) {
    int c = 0;
    for (int i = 0; i < g_block_n; ++i) {
        if (g_blocks[i].live) ++c;
    }
    return c;
}

static void blk_release_all(void) {
    for (int i = 0; i < g_block_n; ++i) {
        if (g_blocks[i].live) {
            free((void *)g_blocks[i].addr);
            g_blocks[i].live = 0;
        }
    }
    g_block_n = 0;
}

/* 解引用前的指针校验:必须是非 NULL 且当前存活的已登记块,且不小于 need 字节 */
static int blk_live_has(const char *name, const void *p, size_t need) {
    if (p == NULL) {
        printf("FAIL %s: pointer is NULL (lost or never set)\n", name);
        return 0;
    }
    if (!blk_live((uintptr_t)p)) {
        printf("FAIL %s: pointer is not a live test block (bogus/lost pointer; no dereference)\n", name);
        return 0;
    }
    if (blk_bytes((uintptr_t)p) < need) {
        printf("FAIL %s: live block smaller than expected (%zu < %zu bytes)\n",
               name, blk_bytes((uintptr_t)p), need);
        return 0;
    }
    return 1;
}

static void *t_realloc_impl(void *p, size_t n) {
    ++g_attempts;
    g_last_request = n;
    uintptr_t old_addr = (uintptr_t)p;   /* 用地址快照做账,不再事后使用 p */
    if (p != NULL && !blk_live(old_addr)) {
        ++g_bad_realloc;   /* 把非法指针挡在真实 realloc 之前 */
        return NULL;
    }
    if (g_armed_fail) {
        g_armed_fail = 0;
        return NULL;
    }
    void *rp = realloc(p, n);
    if (rp == NULL) return NULL;
    if (old_addr != 0) blk_mark_freed(old_addr);
    blk_register(rp, n);
    return rp;
}

static void reset_instr(void) {
    g_armed_fail = 0;
    g_attempts = 0;
    g_last_request = 0;
    g_bad_realloc = 0;
}

static int *make_block(int cap) {
    int *p = (int *)malloc((size_t)cap * sizeof(int));
    if (p == NULL) { printf("FIXTURE FAIL: OOM\n"); return NULL; }
    blk_register(p, (size_t)cap * sizeof(int));
    return p;
}

static int report(const char *name, int ok) {
    printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", name);
    return ok;
}

/* 用例收尾:核对存活块数量(多出即泄漏/丢失),然后安全释放全部登记块 */
static int finish(const char *name, int ok, int expect_live) {
    int live = blk_live_count();
    if (live != expect_live) {
        printf("FAIL %s: live block ledger; expected %d, got %d (leak or lost block)\n",
               name, expect_live, live);
        ok = 0;
    }
    blk_release_all();
    return report(name, ok);
}

static int check_bytes(const char *name, size_t want) {
    if (g_last_request != want) {
        printf("FAIL %s: last realloc request; expected %zu bytes, got %zu\n", name, want, g_last_request);
        return 0;
    }
    return 1;
}

static int check_core_flags(const char *name, int ok) {
    if (g_bad_realloc != 0) {
        printf("FAIL %s: realloc was called with a non-live pointer %d time(s)\n", name, g_bad_realloc);
        ok = 0;
    }
    return ok;
}

static int case_fresh_need1(void) {
    const char *name = "fresh-need1";
    reset_instr();
    int *arr = NULL;
    int cap = 0;
    printf("RUN %s arr=NULL cap=0 need=1\n", name);
    int rc = ensure_capacity(&arr, &cap, 1);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 4) { printf("FAIL %s: cap; expected 4, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: realloc attempts; expected 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)4 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)4 * sizeof(int))) {
        arr[0] = 10;
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fresh_need4(void) {
    const char *name = "fresh-need4";
    reset_instr();
    int *arr = NULL;
    int cap = 0;
    printf("RUN %s arr=NULL cap=0 need=4\n", name);
    int rc = ensure_capacity(&arr, &cap, 4);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 4) { printf("FAIL %s: cap; expected 4, got %d\n", name, cap); ok = 0; }
    ok = check_bytes(name, (size_t)4 * sizeof(int)) && ok;
    ok = blk_live_has(name, arr, (size_t)4 * sizeof(int)) && ok;
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fresh_need5(void) {
    const char *name = "fresh-need5";
    reset_instr();
    int *arr = NULL;
    int cap = 0;
    printf("RUN %s arr=NULL cap=0 need=5\n", name);
    int rc = ensure_capacity(&arr, &cap, 5);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 8) { printf("FAIL %s: cap; expected 8 (4 then double), got %d\n", name, cap); ok = 0; }
    ok = check_bytes(name, (size_t)8 * sizeof(int)) && ok;
    ok = blk_live_has(name, arr, (size_t)8 * sizeof(int)) && ok;
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fresh_stale_cap(void) {
    const char *name = "fresh-stale-cap-ignored";
    reset_instr();
    int *arr = NULL;
    int cap = 999;   /* 陈旧容量:arr 为 NULL 时必须忽略 */
    printf("RUN %s arr=NULL cap=999 need=5\n", name);
    int rc = ensure_capacity(&arr, &cap, 5);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 8) { printf("FAIL %s: cap; expected 8 (fresh path ignores stale cap), got %d\n", name, cap); ok = 0; }
    ok = check_bytes(name, (size_t)8 * sizeof(int)) && ok;
    ok = blk_live_has(name, arr, (size_t)8 * sizeof(int)) && ok;
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fast_path(void) {
    const char *name = "fast-path";
    reset_instr();
    int *arr = make_block(4);
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 4; ++i) arr[i] = 100 + i;
    int cap = 4;
    int *before = arr;
    printf("RUN %s arr=block(4) cap=4 need=3\n", name);
    int rc = ensure_capacity(&arr, &cap, 3);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (arr != before) { printf("FAIL %s: fast path must not move the pointer\n", name); ok = 0; }
    if (cap != 4) { printf("FAIL %s: cap changed; expected 4, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 0) { printf("FAIL %s: realloc called on fast path; attempts=%d\n", name, g_attempts); ok = 0; }
    if (arr == before && blk_live_has(name, arr, 4 * sizeof(int))) {
        for (int i = 0; i < 4; ++i) {
            if (arr[i] != 100 + i) { printf("FAIL %s: data[%d] changed; expected %d, got %d\n", name, i, 100 + i, arr[i]); ok = 0; break; }
        }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fast_path_exact(void) {
    const char *name = "fast-path-exact";
    reset_instr();
    int *arr = make_block(8);
    if (arr == NULL) { blk_release_all(); return 0; }
    arr[7] = 77;
    int cap = 8;
    int *before = arr;
    printf("RUN %s arr=block(8) cap=8 need=8\n", name);
    int rc = ensure_capacity(&arr, &cap, 8);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (arr != before || cap != 8) { printf("FAIL %s: exact-fit must be fast path (ptr/cap unchanged)\n", name); ok = 0; }
    if (g_attempts != 0) { printf("FAIL %s: realloc called; attempts=%d\n", name, g_attempts); ok = 0; }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_grow_4to8(void) {
    const char *name = "grow-4to8";
    reset_instr();
    int *arr = make_block(4);
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 4; ++i) arr[i] = i * 7 + 1;
    int cap = 4;
    printf("RUN %s arr=block(4) cap=4 need=5\n", name);
    int rc = ensure_capacity(&arr, &cap, 5);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 8) { printf("FAIL %s: cap; expected 8, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)8 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)8 * sizeof(int)) && cap == 8) {
        for (int i = 0; i < 4; ++i) {
            if (arr[i] != i * 7 + 1) { printf("FAIL %s: old data[%d]; expected %d, got %d\n", name, i, i * 7 + 1, arr[i]); ok = 0; break; }
        }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_grow_3to6(void) {
    const char *name = "grow-3to6";
    reset_instr();
    int *arr = make_block(3);   /* 非 2 的幂容量 */
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 3; ++i) arr[i] = 500 + i;
    int cap = 3;
    printf("RUN %s arr=block(3) cap=3 need=6\n", name);
    int rc = ensure_capacity(&arr, &cap, 6);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 6) { printf("FAIL %s: cap; expected 6 (3 doubled; not forced to a power of two), got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)6 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)6 * sizeof(int)) && cap == 6) {
        for (int i = 0; i < 3; ++i) {
            if (arr[i] != 500 + i) { printf("FAIL %s: old data[%d]; expected %d, got %d\n", name, i, 500 + i, arr[i]); ok = 0; break; }
        }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_grow_3to12(void) {
    const char *name = "grow-3to12";
    reset_instr();
    int *arr = make_block(3);
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 3; ++i) arr[i] = 700 + i;
    int cap = 3;
    printf("RUN %s arr=block(3) cap=3 need=12\n", name);
    int rc = ensure_capacity(&arr, &cap, 12);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 12) { printf("FAIL %s: cap; expected 12 (3->6->12), got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)12 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)12 * sizeof(int)) && cap == 12) {
        for (int i = 0; i < 3; ++i) {
            if (arr[i] != 700 + i) { printf("FAIL %s: old data[%d]; expected %d, got %d\n", name, i, 700 + i, arr[i]); ok = 0; break; }
        }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_grow_8to32(void) {
    const char *name = "grow-8to32";
    reset_instr();
    int *arr = make_block(8);
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 8; ++i) arr[i] = 900 - i;
    int cap = 8;
    printf("RUN %s arr=block(8) cap=8 need=17\n", name);
    int rc = ensure_capacity(&arr, &cap, 17);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 32) { printf("FAIL %s: cap; expected 32 (16 then 32), got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)32 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)32 * sizeof(int)) && cap == 32) {
        for (int i = 0; i < 8; ++i) {
            if (arr[i] != 900 - i) { printf("FAIL %s: old data[%d]; expected %d, got %d\n", name, i, 900 - i, arr[i]); ok = 0; break; }
        }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_need_cap_plus_one(void) {
    const char *name = "need-cap-plus-one";
    reset_instr();
    int *arr = make_block(8);
    if (arr == NULL) { blk_release_all(); return 0; }
    int cap = 8;
    printf("RUN %s arr=block(8) cap=8 need=9\n", name);
    int rc = ensure_capacity(&arr, &cap, 9);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 16) { printf("FAIL %s: cap; expected 16, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)16 * sizeof(int)) && ok;
    ok = blk_live_has(name, arr, (size_t)16 * sizeof(int)) && ok;
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_grow_big_jump(void) {
    const char *name = "grow-big-jump";
    reset_instr();
    int *arr = make_block(8);
    if (arr == NULL) { blk_release_all(); return 0; }
    int cap = 8;
    printf("RUN %s arr=block(8) cap=8 need=1000\n", name);
    int rc = ensure_capacity(&arr, &cap, 1000);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 1024) { printf("FAIL %s: cap; expected 1024 (one-jump to the doubling-sequence value), got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)1024 * sizeof(int)) && ok;
    ok = blk_live_has(name, arr, (size_t)1024 * sizeof(int)) && ok;
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_cap_zero_existing(void) {
    const char *name = "cap-zero-existing";
    reset_instr();
    int *arr = make_block(1);
    if (arr == NULL) { blk_release_all(); return 0; }
    arr[0] = 55;
    int cap = 0;
    printf("RUN %s arr=block(1) cap=0 need=2\n", name);
    int rc = ensure_capacity(&arr, &cap, 2);
    int ok = 1;
    if (rc != 0) { printf("FAIL %s: return; expected 0, got %d\n", name, rc); ok = 0; }
    if (cap != 4) { printf("FAIL %s: cap; expected 4 (first new capacity), got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_bytes(name, (size_t)4 * sizeof(int)) && ok;
    if (blk_live_has(name, arr, (size_t)4 * sizeof(int)) && cap == 4) {
        if (arr[0] != 55) { printf("FAIL %s: realloc must preserve old content\n", name); ok = 0; }
    } else {
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fail_grow(void) {
    const char *name = "fail-grow";
    reset_instr();
    int *arr = make_block(4);
    if (arr == NULL) { blk_release_all(); return 0; }
    for (int i = 0; i < 4; ++i) arr[i] = 31 + i;
    int cap = 4;
    int *before = arr;
    g_armed_fail = 1;
    printf("RUN %s arr=block(4) cap=4 need=100 (realloc injected to fail)\n", name);
    int rc = ensure_capacity(&arr, &cap, 100);
    int ok = 1;
    if (rc != -1) { printf("FAIL %s: return; expected -1 on realloc failure, got %d\n", name, rc); ok = 0; }
    if (arr != before) {
        printf("FAIL %s: pointer must stay at the original block on failure (no dereference)\n", name);
        ok = 0;
    } else if (!blk_live_has(name, before, 4 * sizeof(int))) {
        printf("FAIL %s: original block lost/not live after failed realloc\n", name);
        ok = 0;
    } else {
        for (int i = 0; i < 4; ++i) {
            if (before[i] != 31 + i) { printf("FAIL %s: original data damaged: data[%d]; expected %d, got %d\n", name, i, 31 + i, before[i]); ok = 0; break; }
        }
    }
    if (cap != 4) { printf("FAIL %s: cap must stay 4 on failure, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_fail_fresh(void) {
    const char *name = "fail-fresh";
    reset_instr();
    int *arr = NULL;
    int cap = 0;
    g_armed_fail = 1;
    printf("RUN %s arr=NULL cap=0 need=1 (realloc injected to fail)\n", name);
    int rc = ensure_capacity(&arr, &cap, 1);
    int ok = 1;
    if (rc != -1) { printf("FAIL %s: return; expected -1, got %d\n", name, rc); ok = 0; }
    if (arr != NULL) { printf("FAIL %s: *arr must stay NULL on failure\n", name); ok = 0; }
    if (cap != 0) { printf("FAIL %s: *cap must stay 0 on failure, got %d\n", name, cap); ok = 0; }
    if (g_attempts != 1) { printf("FAIL %s: attempts; expected exactly 1, got %d\n", name, g_attempts); ok = 0; }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 0);
}

static int case_append_seq(void) {
    const char *name = "append-seq";
    reset_instr();
    int *arr = NULL;
    int cap = 0;
    printf("RUN %s append 0..20 with growth checks\n", name);
    int ok = 1;
    int attempts_prev = 0;
    for (int i = 0; i < 21; ++i) {
        int need = i + 1;
        int rc = ensure_capacity(&arr, &cap, need);
        if (rc != 0) {
            printf("FAIL %s: append #%d return; expected 0, got %d\n", name, i, rc);
            ok = 0;
            break;
        }
        int want_cap = need <= 4 ? 4 : (need <= 8 ? 8 : (need <= 16 ? 16 : 32));
        if (cap != want_cap) {
            printf("FAIL %s: append #%d cap; expected %d, got %d\n", name, i, want_cap, cap);
            ok = 0;
            break;
        }
        if (!blk_live_has(name, arr, (size_t)cap * sizeof(int))) { ok = 0; break; }
        if (g_attempts > attempts_prev) {
            if (g_attempts != attempts_prev + 1) {
                printf("FAIL %s: append #%d growth; expected exactly 1 realloc attempt, got %d\n",
                       name, i, g_attempts - attempts_prev);
                ok = 0;
                break;
            }
            ok = check_bytes(name, (size_t)cap * sizeof(int)) && ok;
        }
        attempts_prev = g_attempts;
        for (int j = 0; j < i; ++j) {   /* 已写入的 0..i-1 必须原样保留 */
            if (arr[j] != j * 3 + 1) {
                printf("FAIL %s: append #%d data[%d]; expected %d, got %d\n", name, i, j, j * 3 + 1, arr[j]);
                ok = 0;
                break;
            }
        }
        if (!ok) break;
        arr[i] = i * 3 + 1;             /* 成功后写入新元素 */
    }
    if (g_attempts != 4) {
        printf("FAIL %s: total realloc calls; expected 4 (need 1,5,9,17), got %d\n", name, g_attempts);
        ok = 0;
    }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

static int case_invalid_inputs(void) {
    const char *name = "invalid-inputs";
    reset_instr();
    int ok = 1;
    printf("RUN %s NULL/need/negative-cap\n", name);
    int *arr = make_block(4);
    if (arr == NULL) { blk_release_all(); return 0; }
    arr[0] = 5;
    int cap = 4;
    int *keep = arr;
    int rc = ensure_capacity(NULL, &cap, 1);
    if (rc != -1) { printf("FAIL %s: arr=NULL; expected -1, got %d\n", name, rc); ok = 0; }
    rc = ensure_capacity(&arr, NULL, 1);
    if (rc != -1) { printf("FAIL %s: cap=NULL; expected -1, got %d\n", name, rc); ok = 0; }
    rc = ensure_capacity(&arr, &cap, 0);
    if (rc != -1) { printf("FAIL %s: need=0; expected -1, got %d\n", name, rc); ok = 0; }
    rc = ensure_capacity(&arr, &cap, -5);
    if (rc != -1) { printf("FAIL %s: need=-5; expected -1, got %d\n", name, rc); ok = 0; }
    int negcap = -3;
    int *nullarr = NULL;
    rc = ensure_capacity(&nullarr, &negcap, 5);
    if (rc != -1) { printf("FAIL %s: *cap=-3 (arr NULL); expected -1, got %d\n", name, rc); ok = 0; }
    if (nullarr != NULL || negcap != -3) { printf("FAIL %s: negative-cap path must not modify state\n", name); ok = 0; }
    rc = ensure_capacity(&arr, &cap, -1);
    if (rc != -1) { printf("FAIL %s: need=-1; expected -1, got %d\n", name, rc); ok = 0; }
    if (arr != keep || cap != 4) { printf("FAIL %s: state changed on invalid input\n", name); ok = 0; }
    if (arr == keep && blk_live_has(name, keep, 4 * sizeof(int))) {
        if (keep[0] != 5) { printf("FAIL %s: data changed on invalid input\n", name); ok = 0; }
    } else {
        ok = 0;
    }
    if (g_attempts != 0) { printf("FAIL %s: realloc must not be called on invalid input; attempts=%d\n", name, g_attempts); ok = 0; }
    ok = check_core_flags(name, ok);
    return finish(name, ok, 1);
}

int main(int argc, char **argv) {
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)&t_realloc_impl; /* 拦截器按需参与;确保零警告 */
    const char *sel = NULL;
    if (argc == 3 && strcmp(argv[1], "--case") == 0) {
        sel = argv[2];
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [--case NAME]\n", argv[0]);
        return 2;
    }
    int total = 0, passed = 0;
    /* --case NAME 接受每个用例 RUN 行打印的稳定标识(首个 token);
     * 输入细节(n=…/cap=…/need=…)跟在标识之后,不属于标识。 */
#define RUN(name, fn) do { if (sel == NULL || strcmp(sel, name) == 0) { ++total; passed += fn(); } } while (0)
    RUN("fresh-need1", case_fresh_need1);
    RUN("fresh-need4", case_fresh_need4);
    RUN("fresh-need5", case_fresh_need5);
    RUN("fresh-stale-cap-ignored", case_fresh_stale_cap);
    RUN("fast-path", case_fast_path);
    RUN("fast-path-exact", case_fast_path_exact);
    RUN("grow-4to8", case_grow_4to8);
    RUN("grow-3to6", case_grow_3to6);
    RUN("grow-3to12", case_grow_3to12);
    RUN("grow-8to32", case_grow_8to32);
    RUN("need-cap-plus-one", case_need_cap_plus_one);
    RUN("grow-big-jump", case_grow_big_jump);
    RUN("cap-zero-existing", case_cap_zero_existing);
    RUN("fail-grow", case_fail_grow);
    RUN("fail-fresh", case_fail_fresh);
    RUN("append-seq", case_append_seq);
    RUN("invalid-inputs", case_invalid_inputs);
#undef RUN
    if (total == 0) {
        fprintf(stderr, "Unknown case: %s\n", sel);
        return 2;
    }
    printf("%d/%d PASS\n", passed, total);
    return passed == total ? 0 : 1;
}
/* FROZEN_TESTS_END */
