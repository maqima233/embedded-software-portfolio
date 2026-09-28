#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BUFF_SIZE 8

typedef struct ring_buffer{
    char data[BUFF_SIZE];
    int head;
    int tail;
}RB;

void init(RB *rb){
    rb->head = 0;
    rb->tail = 0;
    for (int i = 0; i < BUFF_SIZE; i ++) {
        rb ->data[i] = 0;
    }
}

bool is_empty(RB *rb){
    return rb->head == rb->tail;
}

bool is_full(RB *rb){
    return (rb->head + 1) % BUFF_SIZE == rb->tail;
}

bool put(RB *rb,char byte){
    if (is_full(rb)) {
        return  false;
    }

    rb->data[rb->head] = byte;
    rb->head = (rb->head + 1) % BUFF_SIZE;
    return true;
}

bool read(RB *rb,char *out){
    if (is_empty(rb)) {
        return false;
    }

    *out = rb->data[rb->tail];
    rb->tail = (rb->tail + 1) % BUFF_SIZE;
    return true;
}

int available(RB *rb){
    return (rb->tail - rb->head + BUFF_SIZE - 1) % BUFF_SIZE;
}

int main(void){
    printf("测试环形缓冲区开始：\n");
    RB rb;
    init(&rb);
    char byte[8] = {'A','B','C','D','E','F','G','H'};
    int i;
    for(i = 0;i < 7;i++){
        put(&rb, byte[i]);
        printf("head = %d\n",rb.head);
    }
    char out;
    read(&rb, &out);
    printf("tail = %d,第一个数据是%c\n",rb.tail,out);
    printf("还可以写入%d\n",available(&rb));
    put(&rb,byte[i]);
    printf("还可以写入%d\n",available(&rb));
    printf("head = %d",rb.head);
    return 0;
}