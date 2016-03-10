#ifndef queue_H
#define queue_H
#include <inttypes.h>

struct queue_node
{
    struct queue_node* next_node;
    void* data;
};

typedef struct queue
{
    struct queue_node* front_node;
    struct queue_node* back_node;
} queue;

queue* queue_make();
void queue_enqueue(queue* queue, void* data);
void* queue_dequeue(queue* queue, uint8_t clean);
uint8_t queue_empty(queue* queue);
void* queue_peek(queue* queue);
void queue_destroy(queue* queue);

#endif
