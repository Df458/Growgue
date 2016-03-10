#include "queue.h"
#include "stdio.h"
#include "stdlib.h"

queue* queue_make()
{
    queue* s = malloc(sizeof(queue));
    s->front_node = 0;
    s->back_node = 0;
    return s;
}

void queue_enqueue(queue* s, void* data)
{
    struct queue_node* node = malloc(sizeof(struct queue_node));
    /* node->next_node = s->front_node; */
    if(!s->front_node)
        s->front_node = node;
    else
        s->back_node->next_node = node;
    s->back_node = node;
    node->next_node = 0;
    node->data = data;
}

void* queue_dequeue(queue* s, uint8_t clean)
{
    if(queue_empty(s))
        return 0;
    void* data = s->front_node->data;
    struct queue_node* temp = s->front_node;
    if(s->back_node == s->front_node) {
        s->back_node = 0;
        s->front_node = 0;
    }
    s->front_node = temp->next_node;
    free(temp);
    if(data && clean) {
        free(data);
        data = NULL;
    }
    return data;
}

uint8_t queue_empty(queue* s)
{
    if(!s->front_node)
        return 1;
    return 0;
}

void* queue_peek(queue* s)
{
    void* data = 0;
    if(s->front_node)
        data = s->front_node->data;
    return data;
}

void queue_destroy(queue* s)
{
    while(!queue_empty(s)) {
        queue_dequeue(s, 1);
    }
    free(s);
}
