#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
/* 1 KB 静态内存池：best-fit、分裂、释放与复用。 */

#define SIZE 1024
uint8_t header[SIZE];

typedef struct block{
    size_t size;
    bool is_free;
    struct block *next;
} block_t;

block_t *free_list;

void heap_init(void){
    block_t *block = (block_t*)(header + 0);
    block->size = SIZE - sizeof(block_t);
    block->is_free = true;
    block->next = NULL;
    free_list = block;
}

void *best_fit_malloc(size_t size) {
    block_t *cur = free_list;
    block_t *best = NULL;
    block_t *prev = free_list;
    while (cur) {
        if (cur->size >= size && (best == NULL || best->size > cur->size)) {
            best = cur;
        }
        cur = cur->next;
    }
    if (!best) return NULL;
    if (best->size < size) return NULL;
    if (best->size - size >= 2 * sizeof(block_t)) {
        block_t *rest = (block_t*)((uint8_t*)best + size + sizeof(block_t));
        rest->size = best->size - size - sizeof(block_t);
        rest->next = best->next;
        rest->is_free = true;
        best->size = size;
        best->next = rest;
        best->is_free = false;
    }
    else {
        best->is_free = false;
    }
    if (best == free_list) {
        free_list = best->next;
    }
    else {
        while (prev->next->is_free) prev = prev->next;
        prev->next = best->next;
    }
    return best + 1;
}

void my_free(void *ptr){
    if (ptr == NULL) return;
    block_t *metadata = (block_t*)ptr - 1;
    metadata->is_free = true;
    metadata->next = free_list;
    free_list = metadata;
}


static int failures = 0;

#define CHECK(cond, name) do { \
    if (cond) printf("PASS: %s\n", name); \
    else { printf("FAIL: %s\n", name); failures++; } \
} while (0)

static void dump_free_list(const char *tag){
    printf("%s free_list:", tag);
    for (block_t *b = free_list; b; b = b->next)
        printf(" [%zu%s]", b->size, b->is_free ? "" : "!F");
    printf("\n");
}

/* 基本分配:三块地址不重叠 */
void test_basic(void){
    heap_init();
    void *p1 = best_fit_malloc(200);
    void *p2 = best_fit_malloc(300);
    void *p3 = best_fit_malloc(400);
    printf("p1=%p p2=%p p3=%p\n", p1, p2, p3);
    CHECK(p1 != NULL && p2 != NULL && p3 != NULL, "基本分配三块成功");
    CHECK(p1 != p2 && p2 != p3 && p1 != p3, "三块地址互不重叠");
    dump_free_list("基本分配后");
    my_free(p1); my_free(p2); my_free(p3);
}

/* split 边界:请求 990 不回绕,之后还能分 10 */
void test_split_boundary(void){
    heap_init();
    void *big = best_fit_malloc(990);
    CHECK(big != NULL, "990 分配成功(剩余不足整块,整块给)");
    dump_free_list("990 后");
    void *small = best_fit_malloc(10);
    CHECK(small == NULL, "剩余不足,10 分配失败(NULL)");
    my_free(big);
}

/* 释放复用:同大小重新分配拿到同一块 */
void test_reuse(void){
    heap_init();
    void *p1 = best_fit_malloc(300);
    my_free(p1);
    void *p2 = best_fit_malloc(300);
    CHECK(p1 == p2, "释放后复用同一地址");
    my_free(p2);
}

/* best-fit 语义:碎片场景 {100, 700},请求 150 应选中 700 */
void test_best_fit_semantics(void){
    heap_init();
    void *p1 = best_fit_malloc(100);   /* 切出 100,剩余大块回链 */
    void *p2 = best_fit_malloc(700);   /* 切出 700,剩余小片回链 */
    my_free(p1);                       /* 头插:链上 {100, 小片} */
    dump_free_list("释放 100 后");
    void *p3 = best_fit_malloc(150);   /* 100 不够,应选小片 */
    CHECK(p3 != NULL, "150 请求在有足够块时不能返回 NULL");
    if (p3) {
        block_t *meta = (block_t*)p3 - 1;
        printf("p3 size=%zu\n", meta->size);
        CHECK(meta->size >= 150, "选中的块够大");
    }
    my_free(p2); my_free(p3);
}

/* 无足够空间返回 NULL */
void test_no_memory(void){
    heap_init();
    void *p = best_fit_malloc(5000);
    CHECK(p == NULL, "超堆大小返回 NULL");
}

int main(void){
    test_basic();
    test_split_boundary();
    test_reuse();
    test_best_fit_semantics();
    test_no_memory();
    my_free(NULL);
    printf(failures == 0 ? "\n全部通过\n" : "\n%d 个失败\n", failures);
    return failures != 0;
}
