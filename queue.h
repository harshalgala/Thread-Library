#ifndef QUEUE_H
#define QUEUE_H

// This is a queue node
// Each node contains a data pointer which points to a generic blob of memory
// It also has next and previous pointers which point to the next and previous data in the queue
typedef struct node {
  void *data;
  struct node *next;
  struct node *prev;
}NODE;

// This is the queue itself
// Queue is defined by head and tail
// head: This is where dequeue operation happens
// tail: This is where enqueue operations happen
typedef struct queue {
  NODE *head;
  NODE *tail;
}QUEUE;

// initializes the queue by allocating the queue struture
// initialliy head and tail are pointint to NULL denoting an empty queue
QUEUE *init_queue();

// Whether the queue is empty or not
int is_empty(QUEUE *queue);

// Adds data to tail of the queue
void enqueue(QUEUE *queue, void *data);

// Returns the data from the front of the queue and changes the queue structure
void *dequeue(QUEUE *queue);

// Gets the data at the front of the queue without changing the queue struture
void *peek(QUEUE *queue);

// gets the size of the queue
int get_size(QUEUE *queue);

// Moves the node at the head to the tail
void move_head_to_tail(QUEUE *queue);

// Remove the node pointed
void remove_node(QUEUE *queue, NODE *node);

// Move the node from one queue to another (dequeue at the source queue, and enqueue at the destination queue)
void dequeue_enqueue(QUEUE *source, QUEUE *dest);

#endif
