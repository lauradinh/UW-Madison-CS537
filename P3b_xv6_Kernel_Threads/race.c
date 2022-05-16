/* test race condition, atomically consume global variable A to to produce B */
#include "types.h"
#include "user.h"
#include "x86.h"
#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

lock_t lock;
int a, b;
int ppid;
int pid1, pid2, pid3, pid4;

uint running_flag;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void consume(void *arg1, void *arg2);

int
main(int argc, char *argv[])
{
  lock_init(&lock);
  int i;
  ppid = getpid();

  lock_acquire(&lock);
  running_flag = 1;
  lock_release(&lock);
  assert((pid1 = thread_create(consume, NULL, NULL)) > 0);
  assert((pid2 = thread_create(consume, NULL, NULL)) > 0);
  assert((pid3 = thread_create(consume, NULL, NULL)) > 0);
  assert((pid4 = thread_create(consume, NULL, NULL)) > 0);
  
  
  for(i = 0; i < 50; i++) {
    a+=10;
    //printf(1, "produce in main\n");
    sleep(10);
  }
  
  lock_acquire(&lock);
  sleep(2000);
  running_flag = 0;
  sleep(100);
  lock_release(&lock);
  
  
  for(i = 0; i < 4; i++) {
    int join_pid = thread_join();
    assert(join_pid > 0);
    //printf(1, "joined pid %d\n", i);
  }
  
  //printf(1, "b is %d\n", b);
  if(b == 500)
    printf(1, "TEST PASSED\n");
  exit();
}

void
consume(void *arg1, void *arg2) {
  //printf(1, "in consume\n");
  sleep(100);
  while(running_flag) {
    //printf(1, "in consume\n");
    lock_acquire(&lock);
    if (a > 0) {
      a--;
      //printf(1, "consumed 1\n");
      b++;
    }
    lock_release(&lock);
  }
  //printf(1, "out of consume\n");
  exit();
}
