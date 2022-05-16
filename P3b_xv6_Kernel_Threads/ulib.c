#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "mmu.h"
#include "param.h"


struct stack_struct {
  int inuse_flag;
  void *stack;
  void *align_stack;
  int id;
};

struct stack_struct stackInfo[NPROC];

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

int
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  void * stack;
  void *align_stack;
  stack = malloc(PGSIZE * 2);
  int excess = ((uint)(stack))%PGSIZE;
  align_stack = stack + PGSIZE - excess;
  int cid = clone(start_routine, arg1, arg2, align_stack);
  if(cid != -1) {
    for(int i = 0; i < NPROC; i++) {
      if(stackInfo[i].inuse_flag == 0) {
        stackInfo[i].inuse_flag = 1;
        stackInfo[i].id = cid;
        stackInfo[i].stack = stack;
        break;
      }
    }
  } else {
    free(stack);
  }
  return cid;
}

int 
thread_join()
{
  int pid;
  void *stack;
  pid = join(&stack);

  if(pid == -1) {
    return pid;
  } else {
    for(int i = 0; i < NPROC; i++) {
      if(stackInfo[i].inuse_flag == 1 && stackInfo[i].id == pid) {
        free(stackInfo[i].stack);
        stackInfo[i].id = -1;
        stackInfo[i].stack = NULL;
        stackInfo[i].inuse_flag = 0;
        break;
      }
    }
  }
  return pid;
}

void
lock_acquire(lock_t *lock) 
{
  int turn = faa(&lock->ticket, 1);
  while(lock->turn != turn);
}

void
lock_release(lock_t *lock) 
{
  faa(&lock->turn, 1);
}

void
lock_init(lock_t *lock) 
{
  lock->ticket = 0;
  lock->turn = 0;
}

