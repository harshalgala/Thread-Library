
/*
#include <stdio.h>
#include "mythread.h"
int n;
void t1(void * who)
{
  int i;
  printf("t%d start\n", *((int *)who));
  for (i = 0; i < n; i++) {
    printf("t%d yield\n", *((int *)who));
    MyThreadYield();
  }
  printf("t%d end\n", *((int *)who));
  MyThreadExit();
}
void t0(void * dummy)
{
  int zero = 0;
  int who = 1;
  MyThreadCreate(t1, (void *)&who);
  t1((void *)&zero);
}
int main(int argc, char *argv[])
{
  int who = 0;
   if (argc != 2)
    return -1;
  n = atoi(argv[1]); 
  MyThreadInit(t0, (void *)&who);
  printf("This should not be printed\n");
}
*/



#include <stdio.h>
#include "../mythread.h"


void t1(void *dummy) {
  printf("I am child\n");
  MyThreadExit();

}

void start_threads(void *dummy) {
  // Create a thread
  MyThread child =  MyThreadCreate(t1, NULL);
  printf("I am parent\n");
  // Wait for the child to finish
  MyThreadJoin(child);
  printf("Finished Waiting for child to exit\n");
  // Exit
  MyThreadExit();
}

int main(int argc, char **argv) {
  MyThreadInit(start_threads, NULL);
  printf("hello\n");
  return 0;
}



/*
#include <stdio.h>
#include "mythread.h"
int mode = 0;
void t0(void * n)
{
  MyThread T;
  int n1 = *(int *)n; 
  printf("t0 start %d\n", n1);
  int n2 = n1 -1 ;
  if (n1 > 0) {
    printf("t0 create\n");
    T = MyThreadCreate(t0, (void *)&n2);
    if (mode == 1)
      MyThreadYield();
    else if (mode == 2)
      MyThreadJoin(T);
  }
  printf("t0 end\n");
  MyThreadExit();
}
int main(int argc, char *argv[])
{
  // Count will be passed as an argument
  int count; 
  
  if (argc < 2 || argc > 3)
    return -1;
  count = atoi(argv[1]);
  if (argc == 3)
    mode = atoi(argv[2]);
  MyThreadInit(t0, (void *)&count);
}
*/
