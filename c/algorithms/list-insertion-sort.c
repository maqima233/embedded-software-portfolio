/*
 * 刷题线(第二轮)#58:链表插入排序(稳定)
 *
 * 题面(独立训练;用户只实现 USER_CODE 区内标注的函数,其余部分不得改动)
 *
 * 函数签名:
 *   node_t *list_insertion_sort(node_t *head);
 *
 * 输入与范围:
 *   -- 输入为单链表头指针,节点值任意 int(含负数与 INT_MIN/INT_MAX);
 *      节点类型见下方定义,不得修改
 *   -- head 可以为 NULL(空链)
 *   -- 链表必须是无环单链表(契约保证;遍历都要有界终止)
 *
 * 输出与返回码:
 *   -- 用插入排序把链表按 val 升序重排,返回新头指针
 *   -- 稳定性:值相等的节点保持原相对顺序(身份不变)
 *   -- 例:4->2->4->1 -> 1->2->4->4(两个 4 的先后与原序一致)
 *
 * 边界情况:
 *   -- head 为 NULL -> 返回 NULL;单节点原样返回
 *   -- 已序/逆序/全部相同值都要正确;重排后每个节点恰好出现一次
 *
 * 内存归属:
 *   -- 只许修改 next 指针完成重排;不得修改任何节点的 val;
 *   -- 不得新建节点、不得 free 节点、不得分配任何内存
 *
 * 编译运行(项目根目录,GCC;当前 Windows x86-64):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror c-handwritten/algorithms/list-insertion-sort.c -o output/list-insertion-sort.exe
 *   ./output/list-insertion-sort.exe                全部用例
 *   ./output/list-insertion-sort.exe --case NAME    单用例
 * Ubuntu(在本文件所在目录):
 *   gcc -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror list-insertion-sort.c -o list-insertion-sort
 *   ./list-insertion-sort                         全部用例
 *   ./list-insertion-sort --case NAME             单用例
 * 验收:严格编译零诊断;教师冻结测试全部通过;教师本地 commit。
 * 建议 15-20 分钟,30 分钟硬止损。
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct node {
    int val;
    struct node *next;
} node_t;

/* ---- 测试拦截声明:只统计用户核心区的 malloc/free 调用(禁止使用) ---- */
static void *t_malloc_impl(size_t n);
static void t_free_impl(void *p);
#define malloc(n) t_malloc_impl(n)
#define free(p) t_free_impl(p)

/* ======================= 用户核心区 ======================= */
/* USER_CODE_BEGIN */
node_t *list_insertion_sort(node_t *head) {
    if (!head) return NULL;
    node_t dummy;
    dummy.next = head;
    node_t *tail = head;
    node_t *cur = tail->next;
    while (cur)
    {
        if (tail->val <= cur->val) tail = tail->next;
        else
        {
            node_t *prev = &dummy;
            while (prev->next->val <= cur->val) prev = prev->next;
            tail->next = cur->next;
            cur->next = prev->next;
            prev->next = cur;
        }
        cur = tail->next;
    }
    return dummy.next;
}
/* USER_CODE_END */

/* ======================= 教师冻结测试区(2026-10-09 冻结) ======================= */
/* FROZEN_TESTS_BEGIN */
#undef malloc
#undef free

static int g_user_allocs;
static int g_user_frees;

static void *t_malloc_impl(size_t n) {
    ++g_user_allocs;
    return malloc(n);   /* 转发但记录(本契约禁止,用于判违规) */
}

static void t_free_impl(void *p) {
    ++g_user_frees;     /* 不真正释放:节点归测试所有,避免违规后双 free */
    (void)p;
}

#define REG_MAX 32
static node_t *g_nodes[REG_MAX];
static int g_node_val[REG_MAX];
static int g_reg_n;

static void reset_registry(void) {
    g_reg_n = 0;
    g_user_allocs = 0;
    g_user_frees = 0;
    for (int i = 0; i < REG_MAX; ++i) { g_nodes[i] = NULL; g_node_val[i] = 0; }
}

static node_t *t_node_new(int v) {
    if (g_reg_n >= REG_MAX) { printf("FIXTURE FAIL: registry full\n"); return NULL; }
    node_t *n = (node_t *)malloc(sizeof *n);
    if (n == NULL) { printf("FIXTURE FAIL: OOM\n"); return NULL; }
    n->val = v;
    n->next = NULL;
    g_nodes[g_reg_n] = n;
    g_node_val[g_reg_n] = v;
    ++g_reg_n;
    return n;
}

static node_t *build_list(const int *vals, int n) {
    node_t *head = NULL;
    node_t **tail = &head;
    for (int i = 0; i < n; ++i) {
        node_t *nd = t_node_new(vals[i]);
        if (nd == NULL) return head;
        *tail = nd;
        tail = &nd->next;
    }
    return head;
}

static int registry_index(const node_t *p) {
    for (int i = 0; i < g_reg_n; ++i) {
        if (g_nodes[i] == p) return i;
    }
    return -1;
}

static void free_all_nodes(void) {
    for (int i = 0; i < g_reg_n; ++i) free(g_nodes[i]);
    reset_registry();
}

typedef struct {
    const char *name;
    int vals[12];
    int n;
} sort_case_t;

static const sort_case_t cases[] = {
    {"sort-empty",      {0},                                 0},
    {"sort-single",     {42},                                1},
    {"sort-random",     {4, 2, 4, 1},                        4},
    {"sort-sorted",     {1, 2, 3, 4, 5},                     5},
    {"sort-reverse",    {5, 4, 3, 2, 1},                     5},
    {"sort-all-equal",  {7, 7, 7},                           3},
    {"sort-negatives",  {-5, 3, -10, 0, -5},                 5},
    {"sort-interleave", {2, -2, 1, -1, 0},                   5},
    {"sort-pairs",      {1, 2, 1, 2, 1},                     5},
    {"sort-extremes",   {INT_MIN, INT_MAX, 0, INT_MIN, INT_MAX}, 5},
    {"sort-two-equal",  {3, 3},                              2},
    {"sort-desc-pairs", {9, 9, 8, 8, 7},                     5}
};

/* 独立稳定排序(oracle):对身份下标做稳定插入排序,不调用被测函数 */
static void stable_expected_order(const sort_case_t *c, int *order) {
    for (int i = 0; i < c->n; ++i) order[i] = i;
    for (int i = 1; i < c->n; ++i) {
        int key = order[i];
        int j = i - 1;
        while (j >= 0 && c->vals[order[j]] > c->vals[key]) {
            order[j + 1] = order[j];
            --j;
        }
        order[j + 1] = key;
    }
}

static int run_sort_case(const sort_case_t *c) {
    reset_registry();
    node_t *head = c->n > 0 ? build_list(c->vals, c->n) : NULL;
    printf("RUN %s n=%d\n", c->name, c->n);
    int ok = 1;

    int want_order[12];
    stable_expected_order(c, want_order);

    node_t *got = list_insertion_sort(head);

    int seq_idx[16];
    int seq_n = 0;
    int seen[REG_MAX];
    memset(seen, 0, sizeof seen);
    node_t *cur = got;
    int bad_walk = 0;
    while (cur != NULL && !bad_walk) {
        int idx = registry_index(cur);
        if (idx < 0) {
            printf("FAIL %s: list contains a pointer outside test-allocated nodes (step %d)\n", c->name, seq_n);
            bad_walk = 1;
            break;
        }
        if (seen[idx]) {
            printf("FAIL %s: cycle or duplicate node (build-index %d value=%d)\n", c->name, idx, g_node_val[idx]);
            bad_walk = 1;
            break;
        }
        if (seq_n >= 16) {
            printf("FAIL %s: traversal exceeded bound (cycle suspected)\n", c->name);
            bad_walk = 1;
            break;
        }
        seen[idx] = 1;
        seq_idx[seq_n++] = idx;
        if (cur->val != g_node_val[idx]) {
            printf("FAIL %s: node value modified (build-index %d); expected %d, got %d\n",
                   c->name, idx, g_node_val[idx], cur->val);
            ok = 0;
        }
        cur = cur->next;
    }
    if (bad_walk) ok = 0;

    if (!bad_walk) {
        if (seq_n != c->n) {
            printf("FAIL %s: node count; expected %d, got %d\n", c->name, c->n, seq_n);
            ok = 0;
        }
        for (int i = 0; i < (seq_n < c->n ? seq_n : c->n); ++i) {
            if (seq_idx[i] != want_order[i]) {
                printf("FAIL %s: sorted order #%d identity; expected build-index %d (val=%d), got build-index %d (val=%d)\n",
                       c->name, i, want_order[i], c->vals[want_order[i]], seq_idx[i], g_node_val[seq_idx[i]]);
                ok = 0;
                break;
            }
        }
        if (c->n > 1) {
            int sorted = 1;
            for (int i = 1; i < seq_n; ++i) {
                if (g_node_val[seq_idx[i - 1]] > g_node_val[seq_idx[i]]) sorted = 0;
            }
            if (!sorted) {
                printf("FAIL %s: values not in non-decreasing order\n", c->name);
                ok = 0;
            }
        }
        if (got == NULL && c->n != 0) {
            printf("FAIL %s: head; expected non-NULL (n=%d), got NULL\n", c->name, c->n);
            ok = 0;
        }
    }

    if (g_user_allocs != 0) {
        printf("FAIL %s: allocation forbidden; user section called malloc %d time(s)\n", c->name, g_user_allocs);
        ok = 0;
    }
    if (g_user_frees != 0) {
        printf("FAIL %s: free forbidden; user section called free %d time(s)\n", c->name, g_user_frees);
        ok = 0;
    }

    printf(ok ? "PASS %s\n" : "FAIL %s (see above)\n", c->name);
    free_all_nodes();
    return ok;
}

int main(int argc, char **argv) {
    (void)setvbuf(stdout, NULL, _IONBF, 0);
    (void)&t_malloc_impl;
    (void)&t_free_impl;
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
        passed += run_sort_case(&cases[i]);
    }
    if (total == 0) {
        fprintf(stderr, "Unknown case: %s\n", sel);
        return 2;
    }
    printf("%d/%d PASS\n", passed, total);
    return passed == total ? 0 : 1;
}
/* FROZEN_TESTS_END */
