/* clone with bad stack argument */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg1, void *arg2);

int
main(int argc, char *argv[])
{
   ppid = getpid();
   void *stack, *p = malloc(PGSIZE * 2);
   assert(p != NULL);
   if((uint)p % PGSIZE == 0)
     stack = p + 4;
   else
     stack = p;

   assert(clone(worker, 0, 0, stack) == -1);
   
   printf(1, "TEST PASSED\n");
   free(p);
   exit();
}

void
worker(void *arg1, void *arg2) {
   exit();
}
