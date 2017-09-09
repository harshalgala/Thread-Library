#include <stdio.h>
#include <stdlib.h>
#include "../mythread.h"

int n;

void t1(void * who)
{
  int i;

  printf("t%d start\n", *(int *)who);
  for (i = 0; i < n; i++) {
    printf("t%d yield\n", *(int *)who);
    MyThreadYield();
  }
  printf("t%d end\n", *(int *)who);
  MyThreadExit();
}

void t0(void * dummy)
{
  int zero = 0;
  int one = 1;
  MyThreadCreate(t1, (void *)&one);
  t1((void *)&zero);
}

int main(int argc, char *argv[])
{
  int zero = 0;
  if (argc != 2)
    return -1;
  n = atoi(argv[1]);
  MyThreadInit(t0, (void *)&zero);
}