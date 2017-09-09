#include <stdio.h>
#include "../mythread.h"
void *sem;
int data = 0;

void t1() {
  printf("\nIn T1");
  MySemaphoreWait(sem);
  int b = data;
  b++;
  data = b;
  MySemaphoreSignal(sem);
  printf("\nIn T1: data = %d", data);
  MyThreadExit();
} 


void t0(void * dummy)
{
  sem = MySemaphoreInit(1);
  printf("t0 start\n");
  MyThread t = MyThreadCreate(t1, NULL);
  MySemaphoreWait(sem);
  int a = data;
  a = a + 1;
  data = a;
  //MyThreadJoin(t);
  MySemaphoreSignal(sem);
  printf("\nIn T0: data = %d", data);
  MyThreadExit();
}

int main()
{
  MyThreadInit(t0, NULL);
}
