#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

NODE *create_node(void *data) {
  NODE *node = (NODE *)malloc(sizeof(NODE));
  node -> data = data;
  node -> next = NULL;
  node -> prev = NULL;
  return node;
}

int is_empty(QUEUE *queue) {
  return (queue -> head == NULL && queue -> tail == NULL);
}

QUEUE *init_queue() {
  QUEUE *queue = (QUEUE *)malloc(sizeof(QUEUE));
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

void enqueue(QUEUE *queue, void *data) {
  // Create the node
  NODE *node = create_node(data);

  // If the queue is empty
  if(is_empty(queue)) {
    queue -> head = queue -> tail = node;
    return;
  }

  // Add the node to the end of the list
  // Link the last element of the current queue and the new element with eacbh other
  // current tail's next will point to the node
  // node's previous will point to the current tail
  queue -> tail -> next = node;
  node -> prev = queue -> tail;

  // Update the tail pointer
  queue -> tail = node;
}

void *dequeue(QUEUE *queue) {
  void *data = NULL;
  NODE *temp;
  
  // If the queue is empty return null
  if(is_empty(queue))
    return NULL;


  // If the queue is made of only one element
  if(queue -> head == queue -> tail) {
    data = queue -> head -> data;
    free(queue -> head);
    queue -> head = NULL;
    queue -> tail = NULL;
    return data;
  }

  data = queue -> head -> data;
  temp = queue -> head;
  queue -> head -> next -> prev = NULL;
  queue -> head = queue -> head -> next;
  free(temp);
  return data;
}

void *peek(QUEUE *queue) {
  if(queue -> head == NULL && queue -> tail == NULL)
    return NULL;

  return queue -> head -> data;
}

int get_size(QUEUE *queue) {
  int count = 0;
  NODE *current = queue -> head;

  while(current != NULL) {
    count++;
    current = current -> next;
  }

  return count;
}

void move_head_to_tail(QUEUE *queue) {
  NODE *temp;
  // If this is an empty queue or a single element queue, there is nothing to move
  if(queue -> head == queue -> tail)
    return;

  // This is not a single element queue, we will need to move it
  // First grab the reference to this
  temp = queue -> head;

  // Now change the head to point to the second element and make second element's previous null
  queue -> head -> next -> prev = NULL;
  queue -> head = queue -> head -> next;

  // temp will have previous null and next something
  // reverse this
  temp -> next = NULL;
  temp -> prev = queue -> tail;

  // Change the tail, the node that it points to has its next null .. change that to point to the new element
  queue -> tail -> next = temp;
  queue -> tail = temp;

  // We are done 
}

void remove_node(QUEUE *queue, NODE *node) {
  void *ret;
  // If this is a null queue, do nothing
  if(is_empty(queue))
    return;

  // If this is a single element queue and we need to remove the element
  if((queue -> head == queue -> tail) && (queue -> tail == node)) {
    queue -> head = NULL;
    queue -> tail = NULL;
    free(node);
    return;
  }

  // If the element is at the head
  if(queue -> head == node) {
    queue -> head = node -> next;
    node -> next = NULL;
    free(node);
    return;
  }

  // If thelement is at the tail fo the queue
  if(queue -> tail == node) {
    // Queue's tail points to the node's previous element
    queue -> tail = node -> prev;
    // Node's previous element's next will point to nothing since this is the last node now
    node -> prev -> next = NULL;
    // Node's previous element points to nothing
    node -> prev = NULL;
    free(node);
    return;
  }

  // The node is somewhere in the middle
  // we need to adjust the pointers
  node -> prev -> next = node -> next;
  node -> next -> prev = node -> prev;
  node -> prev = NULL;
  node -> next = NULL;
  free(node);
  return; 
}

void dequeue_enqueue(QUEUE *source, QUEUE *dest) {
  void *data = NULL;
  // You cannot remove anything from an empty list
  if(is_empty(source)) return;

  // First dequeue
  data = dequeue(source);

  // Then enqueue
  enqueue(dest, data);
}
