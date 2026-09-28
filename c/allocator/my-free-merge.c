/* 在给定分配器基建上实现相邻空闲块合并与 realloc。 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HEAP_SIZE 1024

typedef struct block {
    size_t        size;   /* payload 字节数(不含 header) */
    struct block *next;   /* 空闲链表后继 */
} block_t;

static uint8_t  heap[HEAP_SIZE];
static block_t *free_list;

void heap_init(void) {
    block_t *blk = (block_t *)heap;
    blk->size = HEAP_SIZE - sizeof(block_t);
    blk->next = NULL;
    free_list = blk;
}

/* 给定基建：best-fit 分配，返回 payload 指针。 */

void *best_fit_malloc(size_t size) {
    block_t *cur = free_list;
    block_t *prev = NULL;
    block_t *best = NULL;
    block_t *best_prev = NULL;

    while (cur != NULL) {
        if (cur->size >= size && (best == NULL || cur->size < best->size)) {
            best = cur;
            best_prev = prev;
        }
        prev = cur;
        cur = cur->next;
    }
    if (best == NULL)
        return NULL;

    if (best->size >= size + 2 * sizeof(block_t)) {
        block_t *rest = (block_t *)((uint8_t *)best + sizeof(block_t) + size);
        rest->size = best->size - size - sizeof(block_t);
        rest->next = best->next;
        best->size = size;
        best->next = rest;
    }

    if (best_prev == NULL)
        free_list = best->next;
    else
        best_prev->next = best->next;

    return best + 1;
}

/* 本人实现：释放、相邻空闲块合并和 realloc。 */

void my_free(void *ptr) {
    if (!ptr) return;

    /* 回跳 header：payload 指针 -1 个 block_t = 块起始（方向 -1，不是 +1） */
    block_t *m = (block_t *)ptr - 1;

    /* ── 第一步：无条件头插 ──
     * 块先回到空闲集合。这一步不依赖任何邻居信息：
     * 链表上全是空闲块，刚 free 的这块当然是空闲块，头插永远合法。 */
    m->next = free_list;
    free_list = m;

    /* ── 第二步：合并（最多两次，不需要循环）──
     * 不变量 P：my_free 返回后链上无物理相邻空闲对；malloc 拆块不改变
     * 物理相邻关系（rest 是 best 的子区域），P 始终成立。
     * 因此 m 的空闲邻居最多左、右各一个 —— 每轮找任一邻居合并，
     * 一轮合并后（m 可能被左邻吞并而转移）另一侧最多还有一个，两轮封顶。 */
    for (int round = 0; round < 2; round++) {
        uint8_t *m_start = (uint8_t *)m;
        uint8_t *m_end   = m_start + sizeof(block_t) + m->size;

        block_t *cur  = free_list;
        block_t *prev = NULL;
        int merged = 0;

        while (cur != NULL) {
            uint8_t *c_start = (uint8_t *)cur;
            uint8_t *c_end   = c_start + sizeof(block_t) + cur->size;

            if (c_end == m_start) {
                /* 前邻吞 m：并块留在前邻的地址（物理规律：并块从左边块的地址开始） */
                cur->size += sizeof(block_t) + m->size;
                /* m 从链表消失。恒在链首，不需要判断：
                 * m 头插进来；若被左邻吞过（m 转移），转移后的块两侧无空闲
                 * 邻居（不变量 P），此分支不会再命中。 */
                free_list = m->next;
                m = cur;          /* 活块转移给前邻，继续用它找邻居 */
                merged = 1;
                break;
            }
            if (m_end == c_start) {
                /* m 吞后邻：并块留在 m 的地址，后邻从链表消失（摘链需要前驱） */
                m->size += sizeof(block_t) + cur->size;
                if (prev == NULL)
                    free_list = cur->next;
                else
                    prev->next = cur->next;
                merged = 1;
                break;
            }
            prev = cur;
            cur = cur->next;
        }

        if (!merged)
            break;                 /* 这一轮没有可合并的邻居，也不会有下一轮 */
    }
}

void *my_realloc(void *ptr, size_t size) {
    if (!ptr) return best_fit_malloc(size);
    if (!size) {
        my_free(ptr);
        return NULL;
    }
    void *newp = best_fit_malloc(size);
    if (!newp) return NULL;
    block_t *old = (block_t*)ptr - 1;
    size_t size_min = old->size < size ? old->size : size;
    memcpy(newp, ptr, size_min);
    my_free(ptr);
    return newp;
}

/* 给定测试用例。 */

static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) printf("PASS: %s\n", (name)); \
    else { printf("FAIL: %s\n", (name)); failures++; } \
} while (0)

static void dump_free_list(const char *tag) {
    printf("%-26s free_list:", tag);
    for (block_t *b = free_list; b != NULL; b = b->next)
        printf(" [%zu]", b->size);
    printf("\n");
}

static size_t list_length(void) {
    size_t n = 0;
    for (block_t *b = free_list; b != NULL; b = b->next)
        n++;
    return n;
}

/* 场景 1: 只 free 一块, 无邻居可合并 —— 块必须还在链上, 还能再分出去 */
void test_free_no_merge(void) {
    heap_init();
    void *a = best_fit_malloc(100);
    void *b = best_fit_malloc(50);
    void *c = best_fit_malloc(80);

    my_free(a);
    dump_free_list("after free(a)");
    CHECK(list_length() == 2, "no-merge: block stays on list (2 blocks)");
    CHECK(free_list->size == 100, "no-merge: freed block at list head, size 100");

    void *a2 = best_fit_malloc(100);
    CHECK(a2 == a, "no-merge: block not lost, malloc(100) returns a again");
    my_free(a2);
    my_free(c);
    my_free(b);
}

/* 场景 2: free A 再 free B, A/B 物理相邻 —— 必须并成一块 */
void test_merge_with_prev(void) {
    heap_init();
    void *a = best_fit_malloc(100);
    void *b = best_fit_malloc(50);
    void *c = best_fit_malloc(80);

    my_free(a);
    dump_free_list("after free(a)");
    my_free(b);
    dump_free_list("after free(b)");
    CHECK(list_length() == 2, "merge a+b: 2 blocks (not 3)");
    CHECK(free_list->size == 166, "merged size = 100+50+header = 166");

    void *big = best_fit_malloc(160);
    CHECK(big == a, "malloc(160) reuses merged block");
    my_free(big);
    my_free(c);
}

/* 场景 3: free A、free C, 最后 free 夹在中间的 B —— 两边同时合并 */
void test_merge_both_sides(void) {
    heap_init();
    void *a = best_fit_malloc(100);
    void *b = best_fit_malloc(50);
    void *c = best_fit_malloc(80);

    my_free(a);
    my_free(c);
    dump_free_list("after free(a),free(c)");
    my_free(b);
    dump_free_list("after free(b)");
    /* 注意: 三次拆块后堆里还剩 rest(730), 它物理紧贴 c —— c 被 free 时先吞 rest(826),
     * b 再被两侧吞 —— 最终全部并回一整块(1024-16=1008)。期望值随契约走, 不是 262。 */
    CHECK(list_length() == 1, "both-sides merge: 1 block left (whole heap)");
    CHECK(free_list->size == 1008, "merged size = whole heap = 1024-16 = 1008");

    void *big = best_fit_malloc(250);
    CHECK(big == a, "malloc(250) fits merged block");
    my_free(big);
}

/* 场景 4: my_free(NULL) 不崩溃 */
void test_free_null(void) {
    my_free(NULL);
    CHECK(1, "free(NULL) does not crash");
}

/* 场景 5: realloc 扩容, 前 64 字节内容必须保留 */
void test_realloc_grow(void) {
    heap_init();
    unsigned char *p = best_fit_malloc(64);
    for (int i = 0; i < 64; i++)
        p[i] = (unsigned char)i;

    unsigned char *q = my_realloc(p, 200);
    CHECK(q != NULL, "realloc grow: succeeds");
    if (q != NULL) {
        int ok = 1;
        for (int i = 0; i < 64; i++)
            if (q[i] != (unsigned char)i) ok = 0;
        CHECK(ok, "realloc grow: first 64 bytes preserved");
    }
    if (q != NULL) my_free(q);
}

/* 场景 6: realloc 缩容, 前 40 字节内容必须保留 */
void test_realloc_shrink(void) {
    heap_init();
    unsigned char *p = best_fit_malloc(100);
    for (int i = 0; i < 100; i++)
        p[i] = (unsigned char)(i + 1);

    unsigned char *q = my_realloc(p, 40);
    CHECK(q != NULL, "realloc shrink: succeeds");
    if (q != NULL) {
        int ok = 1;
        for (int i = 0; i < 40; i++)
            if (q[i] != (unsigned char)(i + 1)) ok = 0;
        CHECK(ok, "realloc shrink: first 40 bytes preserved");
    }
    if (q != NULL) my_free(q);
}

/* 场景 7: realloc 两个特例 */
void test_realloc_edge(void) {
    heap_init();
    void *p = my_realloc(NULL, 32);
    CHECK(p != NULL, "realloc(NULL, 32) acts as malloc(32)");
    if (p == NULL) return;

    void *r = my_realloc(p, 0);
    CHECK(r == NULL, "realloc(p, 0) returns NULL");

    void *again = best_fit_malloc(32);
    CHECK(again == p, "realloc(p, 0) actually freed the block");
    my_free(again);
}

int main(void) {
    printf("=== my_free merge and realloc tests ===\n");
    test_free_no_merge();
    test_merge_with_prev();
    test_merge_both_sides();
    test_free_null();
    test_realloc_grow();
    test_realloc_shrink();
    test_realloc_edge();
    printf(failures == 0 ? "\nALL PASS\n" : "\n%d FAILED\n", failures);
    return failures != 0;
}
