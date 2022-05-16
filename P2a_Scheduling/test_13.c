#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

void 
spin(int fd) 
{
  while(1)
    printf(fd, "A");
}

int
main(int argc, char *argv[])
{
  int fd1 = open("P1.txt", O_CREATE | O_WRONLY);
  int fd2 = open("P2.txt", O_CREATE | O_WRONLY);
  if (fd1 < 0 || fd2 < 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  }

  int rc1 = fork();
  if (rc1 < 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (rc1 == 0) {
    settickets(getpid(), 5);
    sleep(2000);
    spin(fd1);  // infinite loop
  }
  
  // create the second child process after the first wakes up 
  sleep(2000);
  int rc2 = fork();
  if (rc2 < 0) {
    printf(1, "XV6_SCHEDULER\t FAILED\n");
    exit();
  } else if (rc2 == 0) {
    settickets(getpid(), 2);
    spin(fd2);
  }

  settickets(getpid(), 1000);
  sleep(100);
  kill(rc2);
  kill(rc1);
  wait();
  wait();

  struct stat f1, f2;
  fstat(fd1, &f1);
  fstat(fd2, &f2);
  printf(1, "%d - %d\n", f1.size, f2.size);
  if (f1.size > 2 * f2.size) 
    printf(1, "XV6_SCHEDULER\t SUCCESS\n");
  else
    printf(1, "XV6_SCHEDULER\t FAILED\n");

  close(fd1);
  close(fd2);
  unlink("P1.txt");
  unlink("P2.txt");
  exit();
}
