#include "types.h"
#include "user.h"
#include "pstat.h"

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
  int tickets = 10;
  int ppid = getpid();
  struct pstat pstat1, pstat2;

  if (getpinfo(&pstat1) != 0) {
    printf(1, "1: XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  int pidx = get_pstat_idx(&pstat1, ppid);
  if (pidx < 0 || pstat1.tickets[pidx] != 1) {
    printf(1, "%d - %d\n", pidx, pstat1.tickets[pidx]);
    printf(1, "2: XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  if (settickets(ppid, tickets) != 0) {
    printf(1, "3: XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  int rc = fork();
  if (rc < 0) {
    printf(1, "4: XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (rc == 0) {
    int cpid = getpid();
    if (getpinfo(&pstat2) != 0) {
      printf(1, "5: XV6_SCHEDULER\tFAILED\n");
      exit();
    }

    int cidx = get_pstat_idx(&pstat2, cpid);
    if (cidx >= 0 && pstat2.tickets[cidx] == tickets)
      printf(1, "XV6_SCHEDULER\t SUCCESS\n");
    else
      printf(1, "6: XV6_SCUEDULER\t FAILED\n");

    exit(); 
  }

  wait();
  exit();
}
