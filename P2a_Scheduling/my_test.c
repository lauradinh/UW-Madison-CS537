#include "types.h"
#include "user.h"
#include "pstat.h"

int 
spin(int iters) 
{
  int i = 0, j = 0;
  while (i < iters) {
    j += i * j + 1;
    i++;
  }
  return j;
}

int
get_pstat_idx(struct pstat* pstat, int pid)
{
  for (int i = 0; i < NPROC; i++) {
    if (pstat->pid[i] == pid)
      return i;
  }
  return -1;
}

int
main(int argc, char *argv[])
{
  int ppid = getpid(), cpid;
  struct pstat pstat;

  int res;
  int rc = fork();
  if (rc < 0) {
    printf(1, "Test 1: XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (rc == 0) {
    cpid = getpid();
    res = spin(100000000);
    write(-1, "", res);

    if (getpinfo(&pstat) != 0) {
      printf(1, "Test 2: XV6_SCHEDULER\t FAILED\n");
    } else {
      int pidx = get_pstat_idx(&pstat, ppid);
      int cidx = get_pstat_idx(&pstat, cpid);
      if (pidx < 0 || cidx < 0) {
        printf(1, "Test 3: XV6_SCHEDULER\t FAILED\n");
      } else {
        if (pstat.boostsleft[pidx] <= pstat.runticks[cidx] 
              && pstat.boostsleft[pidx] >= pstat.runticks[cidx] - 2){
          printf(1, "XV6_SCHEDULER\t SUCCESS\n");
          printf(1, "parent boosts: %d, child run ticks: %d\n", pstat.boostsleft[pidx], pstat.runticks[cidx]);
        } else{
          printf(1, "parent boosts: %d, child run ticks: %d\n", pstat.boostsleft[pidx], pstat.runticks[cidx]);
          printf(1, "Test 4: XV6_SCHEDULER\t FAILED\n");
        }
      }
    }
  } else {
    wait();
  }

  exit();
}




// #include "types.h"
// #include "user.h"
// #include "pstat.h"

// int 
// spin(int iters) 
// {
//   int i = 0, j = 0;
//   while (i < iters) {
//     j += i * j + 1;
//     i++;
//   }
//   return j;
// }

// int
// get_pstat_idx(struct pstat* pstat, int pid)
// {
//   for (int i = 0; i < NPROC; i++) {
//     if (pstat->pid[i] == pid)
//       return i;
//   }
//   return -1;
// }

// int
// main(int argc, char *argv[])
// {
//   int ppid = getpid(), cpid;
//   struct pstat pstat;

//   int res;
//   int rc = fork();
//   if (rc < 0) {
//     printf(1, "Test 1: XV6_SCHEDULER\t FAILED\n");
//     exit();
//   } else if (rc == 0) {
//     res = spin(100000000);
//     printf(1, "%d\n", res);
//     exit();
//   } else {
//     wait();
//   }

//   cpid = fork();
//   if (cpid < 0) {
//     printf(1, "Test 2: XV6_SCHEDULER\t FAILED\n");
//     exit();
//   } else if (cpid == 0) {
//     res = spin(100000000);
//     printf(1, "%d\n", res);
//     exit();
//   } else {
//     if (getpinfo(&pstat) != 0) {
//       printf(1, "Test 3: XV6_SCHEDULER\t FAILED\n");
//     } else {
//       int pidx = get_pstat_idx(&pstat, ppid);
//       int cidx = get_pstat_idx(&pstat, cpid);
//       if (pidx < 0 || cidx < 0) {
//         printf(1, "Test 4: XV6_SCHEDULER\t FAILED\n");
//       } else {
//         if (pstat.boostsleft[pidx] > 0 && pstat.boostsleft[cidx] == 0 
// 		&& pstat.runticks[cidx] == 0)
//           printf(1, "XV6_SCHEDULER\t SUCCESS\n");
//         else
//           printf(1, "Test 5: XV6_SCHEDULER\t FAILED\n");
//       }
//     }
//   }
  
//   wait();
//   exit();
// }




////////THIS IS THE NEXT WORKING VERSION OF THE CODE


// #include "types.h"
// #include "defs.h"
// #include "param.h"
// #include "memlayout.h"
// #include "mmu.h"
// #include "x86.h"
// #include "proc.h"
// #include "spinlock.h"
// #include "pstat.h"

// #define RAND_MAX 0x7fffffff
// uint rseed = 0;

// // https://rosettacode.org/wiki/Linear_congruential_generator
// uint rand() {
//     return rseed = (rseed * 1103515245 + 12345) & RAND_MAX;
// }

// struct {
//   struct spinlock lock;
//   struct proc proc[NPROC];
//   struct pstat pstat;
// } ptable;

// static struct proc *initproc;

// int nextpid = 1;
// extern void forkret(void);
// extern void trapret(void);

// static void wakeup1(void *chan);

// struct proc 
// *hold_lottery(int total_tickets) 
// {
//     struct proc *p;
//     if (total_tickets <= 0){
//         cprintf("this function should only be called when at least 1 process is RUNNABLE");
//         return 0;
//     }
  
//     uint random_number = rand()%total_tickets;    // This number is between 0->4 billion
//     uint winner_ticket_number = random_number; // Ensure that it is less than total number of tickets.
    
//     // pick the winning process from ptable.
//     int counter = 0;
//     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//       if(p->state != RUNNABLE)
//         continue;

//       //cprintf("boosts_left: %d\n", p->boosts_left);           // DELETE: PRINT STATEMENT TO CHECK IF BOOSTS LEFT IS GETTING INCREMENT
//       if(p->boosts_left > 0){
//         cprintf("boosts_incremented\n");                        // DELETE: PRINT STATEMENT --> checked. boosts is getting incremented so that isn't the problem
//         counter += (p->num_tickets * 2);
//         p->boosts_left--;
//       }                                           //WASN'T ACCOUNTING FOR THE NUMBER OF BOOSTED TICKETS IN THE COUNTER VARIABLE
//       else{
//         counter += p->num_tickets;
//       }
//       if(counter > winner_ticket_number){
//         // return winner.
//         return p;
//       }
//     }
//     return 0;
// }

// int
// calc_total(void) 
// {
//   struct proc* p;
//   int total_tickets = 0;
//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->state == RUNNABLE) {
//       if(p->boosts_left > 0) {
//         total_tickets += (p->num_tickets * 2);
//         //p->boosts_left--;                             // Don't decrement here because you need the boosted number of tickets for the counter
//       } 
//       else {
//         total_tickets += p->num_tickets;
//       }
//     }
//   }
//   return total_tickets;
// }

// void
// pinit(void)
// {
//   initlock(&ptable.lock, "ptable");
// }

// // Must be called with interrupts disabled
// int
// cpuid() {
//   return mycpu()-cpus;
// }

// // Must be called with interrupts disabled to avoid the caller being
// // rescheduled between reading lapicid and running through the loop.
// struct cpu*
// mycpu(void)
// {
//   int apicid, i;
  
//   if(readeflags()&FL_IF)
//     panic("mycpu called with interrupts enabled\n");
  
//   apicid = lapicid();
//   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
//   // a reverse map, or reserve a register to store &cpus[i].
//   for (i = 0; i < ncpu; ++i) {
//     if (cpus[i].apicid == apicid)
//       return &cpus[i];
//   }
//   panic("unknown apicid\n");
// }

// // Disable interrupts so that we are not rescheduled
// // while reading proc from the cpu structure
// struct proc*
// myproc(void) {
//   struct cpu *c;
//   struct proc *p;
//   pushcli();
//   c = mycpu();
//   p = c->proc;
//   popcli();
//   return p;
// }

// //PAGEBREAK: 32
// // Look in the process table for an UNUSED proc.
// // If found, change state to EMBRYO and initialize
// // state required to run in the kernel.
// // Otherwise return 0.
// static struct proc*
// allocproc(void)
// {
//   struct proc *p;
//   char *sp;

//   acquire(&ptable.lock);

//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
//     if(p->state == UNUSED)
//       goto found;

//   release(&ptable.lock);
//   return 0;

// found:
//   p->state = EMBRYO;
//   p->pid = nextpid++;
//   // CHANGED: init boosts
//   // p->boosts_left = 0; 
//   // p->ticks = 0;
//   release(&ptable.lock);

//   // Allocate kernel stack.
//   if((p->kstack = kalloc()) == 0){
//     p->state = UNUSED;
//     return 0;
//   }
//   sp = p->kstack + KSTACKSIZE;

//   // Leave room for trap frame.
//   sp -= sizeof *p->tf;
//   p->tf = (struct trapframe*)sp;

//   // Set up new context to start executing at forkret,
//   // which returns to trapret.
//   sp -= 4;
//   *(uint*)sp = (uint)trapret;

//   sp -= sizeof *p->context;
//   p->context = (struct context*)sp;
//   memset(p->context, 0, sizeof *p->context);
//   p->context->eip = (uint)forkret;

//   return p;
// }

// //PAGEBREAK: 32
// // Set up first user process.
// void
// userinit(void)
// {
//   struct proc *p;
//   extern char _binary_initcode_start[], _binary_initcode_size[];

//   p = allocproc();
  
//   initproc = p;
//   if((p->pgdir = setupkvm()) == 0)
//     panic("userinit: out of memory?");
//   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
//   p->sz = PGSIZE;
//   p->num_tickets = 1;
//   p->ticks = 0;
//   p->boosts_left = 0;
//   memset(p->tf, 0, sizeof(*p->tf));
//   p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
//   p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
//   p->tf->es = p->tf->ds;
//   p->tf->ss = p->tf->ds;
//   p->tf->eflags = FL_IF;
//   p->tf->esp = PGSIZE;
//   p->tf->eip = 0;  // beginning of initcode.S

//   safestrcpy(p->name, "initcode", sizeof(p->name));
//   p->cwd = namei("/");

//   // this assignment to p->state lets other cores
//   // run this process. the acquire forces the above
//   // writes to be visible, and the lock is also needed
//   // because the assignment might not be atomic.
//   acquire(&ptable.lock);

//   p->state = RUNNABLE;

//   release(&ptable.lock);
// }

// // Grow current process's memory by n bytes.
// // Return 0 on success, -1 on failure.
// int
// growproc(int n)
// {
//   uint sz;
//   struct proc *curproc = myproc();

//   sz = curproc->sz;
//   if(n > 0){
//     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
//       return -1;
//   } else if(n < 0){
//     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
//       return -1;
//   }
//   curproc->sz = sz;
//   switchuvm(curproc);
//   return 0;
// }

// // Create a new process copying p as the parent.
// // Sets up stack to return as if from system call.
// // Caller must set state of returned proc to RUNNABLE.
// int
// fork(void)
// {
//   int i, pid;
//   struct proc *np;
//   struct proc *curproc = myproc();

//   // Allocate process.
//   if((np = allocproc()) == 0){
//     return -1;
//   }

//   // Copy process state from proc.
//   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
//     kfree(np->kstack);
//     np->kstack = 0;
//     np->state = UNUSED;
//     return -1;
//   }
//   np->sz = curproc->sz;
//   np->parent = curproc;
//   np->num_tickets = curproc->num_tickets;
//   *np->tf = *curproc->tf;
//   np->ticks = 0;
//   np->boosts_left = 0;
//   // Clear %eax so that fork returns 0 in the child.
//   np->tf->eax = 0;

//   for(i = 0; i < NOFILE; i++)
//     if(curproc->ofile[i])
//       np->ofile[i] = filedup(curproc->ofile[i]);
//   np->cwd = idup(curproc->cwd);

//   safestrcpy(np->name, curproc->name, sizeof(curproc->name));

//   pid = np->pid;

//   acquire(&ptable.lock);

//   np->state = RUNNABLE;

//   release(&ptable.lock);

//   return pid;
// }

// // Exit the current process.  Does not return.
// // An exited process remains in the zombie state
// // until its parent calls wait() to find out it exited.
// void
// exit(void)
// {
//   struct proc *curproc = myproc();
//   struct proc *p;
//   int fd;

//   if(curproc == initproc)
//     panic("init exiting");

//   // Close all open files.
//   for(fd = 0; fd < NOFILE; fd++){
//     if(curproc->ofile[fd]){
//       fileclose(curproc->ofile[fd]);
//       curproc->ofile[fd] = 0;
//     }
//   }

//   begin_op();
//   iput(curproc->cwd);
//   end_op();
//   curproc->cwd = 0;

//   acquire(&ptable.lock);

//   // Parent might be sleeping in wait().
//   wakeup1(curproc->parent);

//   // Pass abandoned children to init.
//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->parent == curproc){
//       p->parent = initproc;
//       if(p->state == ZOMBIE)
//         wakeup1(initproc);
//     }
//   }

//   // Jump into the scheduler, never to return.
//   curproc->state = ZOMBIE;
//   sched();
//   panic("zombie exit");
// }

// // Wait for a child process to exit and return its pid.
// // Return -1 if this process has no children.
// int
// wait(void)
// {
//   struct proc *p;
//   int havekids, pid;
//   struct proc *curproc = myproc();
  
//   acquire(&ptable.lock);
//   for(;;){
//     // Scan through table looking for exited children.
//     havekids = 0;
//     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//       if(p->parent != curproc)
//         continue;
//       havekids = 1;
//       if(p->state == ZOMBIE){
//         // Found one.
//         pid = p->pid;
//         kfree(p->kstack);
//         p->kstack = 0;
//         freevm(p->pgdir);
//         p->pid = 0;
//         p->parent = 0;
//         p->name[0] = 0;
//         p->killed = 0;
//         p->state = UNUSED;
//         release(&ptable.lock);
//         return pid;
//       }
//     }

//     // No point waiting if we don't have any children.
//     if(!havekids || curproc->killed){
//       release(&ptable.lock);
//       return -1;
//     }

//     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
//     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
//   }
// }

// //PAGEBREAK: 42
// // Per-CPU process scheduler.
// // Each CPU calls scheduler() after setting itself up.
// // Scheduler never returns.  It loops, doing:
// //  - choose a process to run
// //  - swtch to start running that process
// //  - eventually that process transfers control
// //      via swtch back to the scheduler.
// void
// scheduler(void)
// {
//   struct proc *p;
//   struct cpu *c = mycpu();
//   c->proc = 0;
//   int total_tickets = 0;
//   for(;;){
//     // Enable interrupts on this processor.
//     sti();

//     // Loop over process table looking for process to run.
//     acquire(&ptable.lock);
//     // CHANGED: reset total_tickets
//     total_tickets = 0;
//     total_tickets = calc_total();
//     if(total_tickets == 0){
//       release(&ptable.lock);
//       continue;
//     }
//     p = hold_lottery(total_tickets);
//     // Switch to chosen process.  It is the process's job
//     // to release ptable.lock and then reacquire it
//     // before jumping back to us.
//     c->proc = p;
//     switchuvm(p);
//     p->state = RUNNING;
//     //const int tick_start = ticks;       // CHANGED: added beginning of ticks
//     p->ticks++;
//     swtch(&(c->scheduler), p->context);
//     switchkvm();
//     //p->ticks += ticks - tick_start;     // CHANGED: add number of ticks run for
//     // Process is done running for now.
//     // It should have changed its p->state before coming back.
//     c->proc = 0;
//     release(&ptable.lock);
//     }
// }

// // Enter scheduler.  Must hold only ptable.lock
// // and have changed proc->state. Saves and restores
// // intena because intena is a property of this
// // kernel thread, not this CPU. It should
// // be proc->intena and proc->ncli, but that would
// // break in the few places where a lock is held but
// // there's no process.
// void
// sched(void)
// {
//   int intena;
//   struct proc *p = myproc();

//   if(!holding(&ptable.lock))
//     panic("sched ptable.lock");
//   if(mycpu()->ncli != 1)
//     panic("sched locks");
//   if(p->state == RUNNING)
//     panic("sched running");
//   if(readeflags()&FL_IF)
//     panic("sched interruptible");
//   intena = mycpu()->intena;
//   swtch(&p->context, mycpu()->scheduler);
//   mycpu()->intena = intena;
// }

// // Give up the CPU for one scheduling round.
// void
// yield(void)
// {
//   acquire(&ptable.lock);  //DOC: yieldlock
//   myproc()->state = RUNNABLE;
//   sched();
//   release(&ptable.lock);
// }

// // A fork child's very first scheduling by scheduler()
// // will swtch here.  "Return" to user space.
// void
// forkret(void)
// {
//   static int first = 1;
//   // Still holding ptable.lock from scheduler.
//   release(&ptable.lock);

//   if (first) {
//     // Some initialization functions must be run in the context
//     // of a regular process (e.g., they call sleep), and thus cannot
//     // be run from main().
//     first = 0;
//     iinit(ROOTDEV);
//     initlog(ROOTDEV);
//   }

//   // Return to "caller", actually trapret (see allocproc).
// }

// // Atomically release lock and sleep on chan.
// // Reacquires lock when awakened.
// void
// sleep(void *chan, struct spinlock *lk)
// {
//   struct proc *p = myproc();
  
//   if(p == 0)
//     panic("sleep");

//   if(lk == 0)
//     panic("sleep without lk");

//   // Must acquire ptable.lock in order to
//   // change p->state and then call sched.
//   // Once we hold ptable.lock, we can be
//   // guaranteed that we won't miss any wakeup
//   // (wakeup runs with ptable.lock locked),
//   // so it's okay to release lk.
//   if(lk != &ptable.lock){  //DOC: sleeplock0
//     acquire(&ptable.lock);  //DOC: sleeplock1
//     release(lk);
//   }
//   // Go to sleep.
//   p->chan = chan;
//   p->state = SLEEPING;
//   p->sleep_flag = 0;

//   sched();

//   // Tidy up.
//   p->chan = 0;

//   // Reacquire original lock.
//   if(lk != &ptable.lock){  //DOC: sleeplock2
//     release(&ptable.lock);
//     acquire(lk);
//   }
// }

// //PAGEBREAK!
// // Wake up all processes sleeping on chan.
// // The ptable lock must be held.
// static void
// wakeup1(void *chan)
// {
//   struct proc *p;

//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->state == SLEEPING && chan == p->chan){
//       if(chan == &ticks && p->tick_sleep == p->sleep_flag) {
//         p->state = RUNNABLE;
//       }
//       else if(chan != &ticks){
//       //p->boosts_left++;
//         p->state = RUNNABLE;
//       }
//       else if(chan == &ticks){
//         // cprintf("Working\n");
//         // cprintf("sleep_flag : %d\n", p->sleep_flag);
//         // cprintf("tick_sleep : %d\n", p->tick_sleep);
//         p->sleep_flag++;
//         p->boosts_left++;

//       }
//       // if(chan != &ticks || (p->tick_sleep == p->sleep_flag && p->tick_sleep != 0)) {
//       //   p->state = RUNNABLE;
//       // }
//       // else{
//       //   cprintf("Working\n");
//       //   cprintf("sleep_flag : %d\n", p->sleep_flag);
//       //   cprintf("tick_sleep : %d\n", p->tick_sleep);
//       //   p->sleep_flag++;
//       //   p->boosts_left++;

//       // }
//       // if(p->tick_sleep > p->sleep_flag && p->tick_sleep != 0){
//       //   //p->boosts_left++;
//       //   p->sleep_flag++;
//       // } theory is that somewhere, there is an off calculation -> total number being wrong and then trying to access a process that does not e
//       // else{
//         // p->boosts_left += 1;
//         // p->state = RUNNABLE;
//       // }
//     }
//   }
// }

// // Wake up all processes sleeping on chan.
// void
// wakeup(void *chan)
// {
//   acquire(&ptable.lock);
//   wakeup1(chan);
//   release(&ptable.lock);
// }

// // Kill the process with the given pid.
// // Process won't exit until it returns
// // to user space (see trap in trap.c).
// int
// kill(int pid)
// {
//   struct proc *p;

//   acquire(&ptable.lock);
//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->pid == pid){
//       p->killed = 1;
//       // Wake process from sleep if necessary.
//       if(p->state == SLEEPING)
//         p->state = RUNNABLE;
//       release(&ptable.lock);
//       return 0;
//     }
//   }
//   release(&ptable.lock);
//   return -1;
// }

// //PAGEBREAK: 36
// // Print a process listing to console.  For debugging.
// // Runs when user types ^P on console.
// // No lock to avoid wedging a stuck machine further.
// void
// procdump(void)
// {
//   static char *states[] = {
//   [UNUSED]    "unused",
//   [EMBRYO]    "embryo",
//   [SLEEPING]  "sleep ",
//   [RUNNABLE]  "runble",
//   [RUNNING]   "run   ",
//   [ZOMBIE]    "zombie"
//   };
//   int i;
//   struct proc *p;
//   char *state;
//   uint pc[10];

//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->state == UNUSED)
//       continue;
//     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
//       state = states[p->state];
//     else
//       state = "???";
//     cprintf("%d %s %s", p->pid, state, p->name);
//     if(p->state == SLEEPING){
//       getcallerpcs((uint*)p->context->ebp+2, pc);
//       for(i=0; i<10 && pc[i] != 0; i++)
//         cprintf(" %p", pc[i]);
//     }
//     cprintf("\n");
//   }
// }

// struct pstat*
// set_pstat(struct pstat* pstat)
// {
//   acquire(&ptable.lock);
//   pstat = &ptable.pstat;
//   struct proc *p;
//   int index = 0;
//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->state != UNUSED){
//       pstat->inuse[index] = 1;
//     }else{
//       pstat->inuse[index] = 0;
//     }
//     pstat->pid[index] = p->pid;
//     pstat->tickets[index] = p->num_tickets;
//     pstat->runticks[index] = p->ticks;
//     pstat->boostsleft[index] = p->boosts_left;
//     index++;
//   }
//   release(&ptable.lock);
//   return pstat;
// }

// void
// set_tickets(int pid, int ticket)
// {
//   acquire(&ptable.lock);
//   struct proc *p;
//   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
//     if(p->pid == pid) {
//       p->num_tickets = ticket;
//     }
//   }
//   release(&ptable.lock);
// }