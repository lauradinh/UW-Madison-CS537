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
  int nchild = 5;
  int cpid[nchild];
  struct pstat pstat;

  int res;
  for (int i = 0; i < nchild; i++) {
    cpid[i] = fork();
    if (cpid[i] < 0) {
      printf(1, "XV6_SCHEDULER\t FAILED\n");
      exit();
    } else if (cpid[i] == 0) {
      settickets(getpid(), 1 << (nchild - i));
      res = spin(2147483647);
      printf(1, "%d\n", res);
      exit();
    }
  }
 
  sleep(1000);
  getpinfo(&pstat);
  for (int i = 0; i < nchild; i++) 
    kill(cpid[i]);
  for (int i = 0; i < nchild; i++)
    wait();

  int cidx[nchild];
  for (int i = 0; i < nchild; i++)
    cidx[i] = get_pstat_idx(&pstat, cpid[i]);

  for (int i = 1; i < nchild; i++) {
    if (cidx[i-1] < 0 || cidx[i] < 0) {
      printf(1, "XV6_SCHEDULER\t FAILED\n");
      exit();
    }
    if (pstat.runticks[cidx[i-1]] < pstat.runticks[cidx[i]] * 1.4) {
      printf(1, "XV6_SCHEDULER\t FAILED\n");
      exit();
    }
  }

  printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  exit();
}


