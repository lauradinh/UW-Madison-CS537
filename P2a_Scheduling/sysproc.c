#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

extern uint rseed;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  myproc()->tick_sleep = n;                 //CHANGED
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    // myproc()->boosts_left++;
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int 
sys_settickets(void) 
{
  int n_tickets;
  int proc_pid;
  if(argint(0, &proc_pid) < 0){
    return -1;
  }else if(argint(1, &n_tickets) < 0){
    return -1;
  } else if (proc_pid < 0) {
    return -1;
  } else if (n_tickets <= 0) {
    return -1;
  }else{
    set_tickets(proc_pid, n_tickets);
    return 0;
  }

}

int
sys_srand(void)
{
  int seed;
  if(argint(0, &seed) < 0) {
    return -1;
  }
  rseed = (uint)seed;
  return 0;

}

int
sys_getpinfo(void)
{
  struct pstat* pstat;

  if(argptr(0, (void*)&pstat, sizeof(*pstat)) < 0) {
    return -1;
  }
  
  struct pstat* new_pstat;
  new_pstat = set_pstat(pstat);
  for(int i = 0; i < NPROC; i++) {
    pstat->pid[i] = new_pstat->pid[i];
    pstat->inuse[i] = new_pstat->inuse[i];
    pstat->runticks[i] = new_pstat->runticks[i];
    pstat->tickets[i] = new_pstat->tickets[i];
    pstat->boostsleft[i] = new_pstat->boostsleft[i];

  }
  return 0;
}