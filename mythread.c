#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "queue.h"
#include "mythread.h"

// Stack size of 8KB
#define STACK_SIZE (8 * 1024)

// This will be used to mint the thread Id (monotonically increasing)
static int nextThreadId = 1;

// Bookkeeping strutcure for each thread
typedef struct thread_context {
  // This is the monotonically increasing threadId for ths thread
  int tid;
  // This is the parent thread id who created this thread
  int parentId;
  // Actual context of this thread
  ucontext_t *context;
  // Whether parent is blocked on this thread's exit or not
  // When we do thread join, we set this flag to let the child know that parent is blocked on its exit
  int parentBlockedOnMyExit;
  // This is set when thread enters the blocked queue to denote whether the thread is waiting for all children or not
  int waitingForAllChildren;
}THREAD_CONTEXT;

typedef struct semaphore {
  // Current value of the semaphore, it is set by the init
  int value;
  // Threads (tids) waiting on this semaphore
  QUEUE *blocked_queue;
}SEMAPHORE;

// Queue of all the currently ready threads
// We assume that head is the one that is running at the moment
QUEUE *ready_queue;

// Queue of blocked threads
// When thread call join, it is blocked and placed in this thread
QUEUE *blocked_queue;

ucontext_t *init_context;

// Finds the node in the queue which matches the tid
NODE *find(QUEUE *queue, int tid) {
  NODE *current = queue -> head;
  int currentId = -1;
  while(current != NULL) {
    currentId = *((int *)current -> data);
    if(currentId == tid)
      return current;
    current = current -> next;
  }
  return NULL;
}

THREAD_CONTEXT *get_ready_thread(int tid) {
  NODE *node = find(ready_queue, tid);
  if(node == NULL)
    return NULL;
  return (THREAD_CONTEXT *)node -> data;
}

THREAD_CONTEXT *get_blocked_thread(int tid) {
  NODE *node = find(blocked_queue, tid);
  if(node == NULL)
    return NULL;
  return (THREAD_CONTEXT *)node -> data;
}

// Count the number of threads in ready queue which has a parent id = parentId
int count_ready_children(int parentId) {
  int count = 0;
  NODE *current = ready_queue -> head;
  while(current != NULL) {
    THREAD_CONTEXT *context = (THREAD_CONTEXT *)current -> data;
    if(context -> parentId == parentId)
      count++;
    current = current -> next;
  }
  return count;
}

int count_blocked_children(int parentId) {
  int count = 0;
  NODE *current = blocked_queue -> head;
  while(current != NULL) {
    THREAD_CONTEXT *context = (THREAD_CONTEXT *)current -> data;
    if(context -> parentId == parentId)
      count++;
    current = current -> next;
  }
  return count;
}

/* Purely debug related functions */

void dump_ready_queue() {
  NODE *current = ready_queue->head;
  printf("READY  ");
  while(current != NULL) {
    THREAD_CONTEXT *context = (THREAD_CONTEXT *)current->data;
    printf("--%d(%d) [%d] --", context -> tid, context -> parentId, context -> parentBlockedOnMyExit);
    //dump_thread_context(context);
    current = current -> next;
  }
  printf("\n");
}

void dump_blocked_queue() {
  NODE *current = blocked_queue->head;
  printf("BLOCKED  ");
  while(current != NULL) {
    THREAD_CONTEXT *context = (THREAD_CONTEXT *)current->data;
    printf("--%d(%d) [%d] --", context -> tid, context -> parentId, context -> waitingForAllChildren);
    current = current -> next;
  }
  printf("\n");
}


// Starts executing the front of the ready queue and until it relinquishes
void start() {
  // We cannot start if the queue is empty
  if(is_empty(ready_queue))
    return;
  // get the thread context at the head of the queue
  THREAD_CONTEXT *context = (THREAD_CONTEXT *)peek(ready_queue);
  //printf("Starting thread with id: %d\n", context->tid);
  setcontext(context->context);
}

void MyThreadInit(void(*start_funct)(void *), void *args) {
  THREAD_CONTEXT *runnable_thread_context = NULL;

  init_context = (ucontext_t *)malloc(sizeof(ucontext_t));

  // Initialize the two queues
  ready_queue = init_queue();
  blocked_queue = init_queue();

  // Create the master thread
  MyThreadCreate(start_funct, args);

  // Start executing the master thread
  //start();
  runnable_thread_context = (THREAD_CONTEXT *)peek(ready_queue);
  swapcontext(init_context, runnable_thread_context -> context);
}


// Creates the thread with starting function start_func which takes a single void * argument
MyThread MyThreadCreate(void(*start_funct)(void *), void *args) {
  // Create thread context, this will bne used for book-keeping
  THREAD_CONTEXT *thread_context = (THREAD_CONTEXT *)malloc(sizeof(THREAD_CONTEXT));
  thread_context->tid = nextThreadId;
  thread_context->context = (ucontext_t *)malloc(sizeof(ucontext_t));

  // Setting the parent
  // If the readyy queue is empty, this is the first thread and it does not have any parent
  // set it to 0
  // Otherwise set it to the tid of the head
  if(is_empty(ready_queue))
    thread_context->parentId = 0;
  else {
    thread_context->parentId = ((THREAD_CONTEXT *)ready_queue->head->data)->tid;
  }

  // Parent is not blocked .. no one called thread join on newly created thread yet
  thread_context -> parentBlockedOnMyExit = 0;
  thread_context -> waitingForAllChildren = 0;

  // Get the current context
  getcontext(thread_context->context);

  // However we need to make some changes to the context of this thread
  // Set the next thing to run to null, nothing else will run once this function finishes
  if(is_empty(ready_queue))
    thread_context->context->uc_link = init_context;
  else
    thread_context->context->uc_link = NULL;
  thread_context->context->uc_stack.ss_sp = malloc(STACK_SIZE);
  thread_context->context->uc_stack.ss_size = STACK_SIZE;
  thread_context->context->uc_stack.ss_flags = 0;

  // make a new context (we are attaching the function with the context, so that when we switch to that context we start executing)
  makecontext(thread_context->context, (void (*) (void)) start_funct, 1, args);

  // Add this context to the tail of the queue
  enqueue(ready_queue, (void *)thread_context);
  //dump_ready_queue();
  find(ready_queue, 1);

  printf("--------------- thread created -----------------\n");
  dump_ready_queue();
  dump_blocked_queue();
  printf("------------------------------------------------\n");

  // Increment the next thread id
  nextThreadId++;

  // Return the threadId
  return (void *)&thread_context->tid;
}

void MyThreadYield(void) {
  // If the ready queue has only one thread, there is nothing to yield to
  if(ready_queue -> head == ready_queue -> tail)
    return;

  // This will move head to tail
  move_head_to_tail(ready_queue);

  // Swap context between the head element and tail element
  swapcontext(((THREAD_CONTEXT *)ready_queue->tail->data)->context, ((THREAD_CONTEXT *)ready_queue->head->data)->context);
}

void MyThreadExit(void) {
  // Grab the thread context of the exiting thread
  THREAD_CONTEXT *exiting_thread_context = (THREAD_CONTEXT *)dequeue(ready_queue);

  // This will hold the context of the thread that will be woken up
  THREAD_CONTEXT *waking_up_thread_context = NULL;

  NODE *queue_node;
  NODE *blocked_on_queue_node;
  int *tid_to_awake;
  //printf("Thread %d exiting\n", exiting_thread_context->tid);

  if(exiting_thread_context -> parentBlockedOnMyExit) {
    // Find the parent in the blocked queue
    queue_node = find(blocked_queue, exiting_thread_context -> parentId);
    // Grab the context and move it from blocking queue to ready queue
    waking_up_thread_context = (THREAD_CONTEXT *)queue_node -> data;
    // If it is not waiting for all the children, it should be awaken
    // However if it is waiting for all the children, we should not find any 
    if(!waking_up_thread_context -> waitingForAllChildren || 
        (count_ready_children(waking_up_thread_context->tid) == 0) && (count_blocked_children(waking_up_thread_context->tid) == 0)) {
      enqueue(ready_queue, queue_node -> data);
      remove_node(blocked_queue, queue_node);
    }
  }

  printf("----------------- thread exit %d -----------\n", exiting_thread_context -> tid);
  dump_ready_queue();
  dump_blocked_queue();
  printf("---------------------------------------------\n");

  // Lets free some data structure
  free(exiting_thread_context->context->uc_stack.ss_sp);  
  free(exiting_thread_context->context);
  free(exiting_thread_context);


  // Start the next thread since this thread exited
  start();

}

void blockCurrentThread() {
  THREAD_CONTEXT *next_runnable_thread_context = (THREAD_CONTEXT *)peek(ready_queue);
  THREAD_CONTEXT *blocking_thread_context = (THREAD_CONTEXT *)blocked_queue -> tail -> data;
  swapcontext(blocking_thread_context->context, next_runnable_thread_context->context);
}

void MyThreadJoinAll(void) {
  // Get the tid of the thread which will be blocking (currently running tid)
  THREAD_CONTEXT *running_thread_context = (THREAD_CONTEXT *)ready_queue -> head -> data;
  int my_tid = running_thread_context -> tid;

  // Look at all the children in the ready queue and set parentBlocking to true
  NODE *current = ready_queue -> head;
  while(current != NULL) {
    THREAD_CONTEXT *context = (THREAD_CONTEXT *)current -> data;
    if(context -> parentId == my_tid)
      context -> parentBlockedOnMyExit = 1;
    current = current -> next;
  }

  // Move the thread context from ready queue to blocked queue
  dequeue_enqueue(ready_queue, blocked_queue);

  // Denote that the job that we just moved to blocked_queue will be waiting for all the children to exit (not just one)
  THREAD_CONTEXT *blocking_thread_context = (THREAD_CONTEXT *)blocked_queue -> tail -> data;
  blocking_thread_context -> waitingForAllChildren = 1;

  printf("---------------- thread join all %d ------------- \n", blocking_thread_context -> tid);
  dump_ready_queue();
  dump_blocked_queue();
  printf("-------------------------------------------------\n");

  blockCurrentThread();
}


int MyThreadJoin(MyThread thread) {
  int tid_to_block_on = *(int *)thread;             // This is the tid of the child on which the caller will block
  NODE *queue_node = NULL;                          // Temporary node to hold a queue node

  // Contexst of the caller, this thread will get blockled as a result of join
  THREAD_CONTEXT *thread_to_block_on_context = NULL;

  // This is the next runnable thread from the ready_queue, it will start runnig 
  THREAD_CONTEXT *next_runnable_thread_context = NULL;

  // The thread at the head is running and it will be blocked, grab the context
  // we dont dequeue as this thread may not be eligible for blocking (we need to check few conditions first)
  THREAD_CONTEXT *blocking_thread_context = (THREAD_CONTEXT *)peek(ready_queue);

  //printf("attempting thread join: %d on %d\n", blocking_thread_context->tid, tid_to_block_on);

  // You cannot block on yourself
  if(tid_to_block_on == blocking_thread_context->tid) {
    printf("ERROR: Attempting to join on itself\n");
    return -1;
  }

  thread_to_block_on_context = get_ready_thread(tid_to_block_on);

  // If we did not find the thread to block, it may have already exited.
  // Do nothing
  if(thread_to_block_on_context == NULL) {
    printf("ERROR: Did not find a thread to join on\n");
    return -1;
  }

  // A thread can only block on its immediate children
  if(thread_to_block_on_context -> parentId != blocking_thread_context -> tid) {
    printf("A thread can only block on its immediate children");
    return -1;
  }

  // Tell the children that parent is blocking on its exit
  thread_to_block_on_context -> parentBlockedOnMyExit = 1;

  // We are ready for moving the thread to blocking queue
  dequeue_enqueue(ready_queue, blocked_queue);

  printf("---------------- thread join %d on %d ------------- \n", blocking_thread_context -> tid, tid_to_block_on);
  dump_ready_queue();
  dump_blocked_queue();
  printf("-------------------------------------------------\n");

  // Block the current thread
  blockCurrentThread();
}

// get the running thread's id
int getTid() {
  THREAD_CONTEXT *current_thread = (THREAD_CONTEXT *)peek(ready_queue);
  return current_thread -> tid;
}

/**
SEMAPHORE FUNCTIONS
**/
MySemaphore MySemaphoreInit(int initialValue) {

  // Initial value must be non-negative
  if(initialValue < 0)
    return NULL;

  SEMAPHORE *sem = (SEMAPHORE *)malloc(sizeof(SEMAPHORE));
  sem -> value = initialValue;
  sem -> blocked_queue = init_queue();
  return (MySemaphore)sem;
}

void MySemaphoreSignal(MySemaphore semaphore) {
  SEMAPHORE *sem = (SEMAPHORE *)semaphore;
  int *tid_ptr;
  int tid;
  NODE *queue_node;
  THREAD_CONTEXT *waking_up_thread_context;

  // First increment the value
  (sem -> value)++;

  // If the semaphore value is still negative or 0, wake up one of the thraeds
  if(sem -> value <= 0) {
    // Figure out the tid of the thread to be woken up
    tid_ptr = (int *)dequeue(sem -> blocked_queue);
    tid = *tid_ptr;

    // Locate the thread context of the thread which will be awaken
    queue_node = find(blocked_queue, tid);
    if(queue_node == NULL)
      return;

    // Grab its thread context
    waking_up_thread_context = (THREAD_CONTEXT *)queue_node -> data;

    // Move this context to the ready queue
    enqueue(ready_queue, (void *)waking_up_thread_context);

    // Remove the node from the blocking queue
    remove_node(blocked_queue, queue_node);
  }
}

void MySemaphoreWait(MySemaphore semaphore) {
  SEMAPHORE *sem = (SEMAPHORE *)semaphore;
  int *tid_ptr = NULL;  // This will capture the tid of the running thread

  // When we wait on the semaphore we need to decrement the value
  (sem -> value)--;

  // If the value is negative, the calling thread will block
  if(sem->value < 0) {
    // Get the tid of the current thread id
    tid_ptr = (int *)malloc(sizeof(int));
    *tid_ptr = getTid();

    // Add this tid to the list of blocked threads
    enqueue(sem -> blocked_queue, (void *)tid_ptr);

    // move the currently running thread to blocked queue
    // And then block the current thread
    dequeue_enqueue(ready_queue, blocked_queue);
    blockCurrentThread();
  }
}

int MySemaphoreDestroy(MySemaphore semaphore) {
  SEMAPHORE *sem = (SEMAPHORE *)semaphore;

  // If there are threads still waiting dfail this call
  if(!is_empty(sem -> blocked_queue))
    return -1;

  free(sem -> blocked_queue);
  free(sem);

  return 0;
}
