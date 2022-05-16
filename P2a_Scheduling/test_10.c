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
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (rc == 0) {
    res = spin(100000000);
    printf(1, "%d\n", res);
    exit();
  } else {
    wait();
  }

  cpid = fork();
  if (cpid < 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (cpid == 0) {
    res = spin(100000000);
    printf(1, "%d\n", res);
    exit();
  } else {
    if (getpinfo(&pstat) != 0) {
      printf(1, "XV6_SCHEDULER\t FAILED\n");
    } else {
      int pidx = get_pstat_idx(&pstat, ppid);
      int cidx = get_pstat_idx(&pstat, cpid);
      if (pidx < 0 || cidx < 0) {
        printf(1, "XV6_SCHEDULER\t FAILED\n");
      } else {
        if (pstat.boostsleft[pidx] > 0 && pstat.boostsleft[cidx] == 0 
		&& pstat.runticks[cidx] == 0)
          printf(1, "XV6_SCHEDULER\t SUCCESS\n");
        else
          printf(1, "XV6_SCHEDULER\t FAILED\n");
      }
    }
  }
  
  wait();
  exit();
}
