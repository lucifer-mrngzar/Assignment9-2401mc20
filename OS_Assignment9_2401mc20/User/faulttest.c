//Ranveer Gupta 
//2401mc20
//Assignment 9
//Q2

#include "types.h"
#include "stat.h"
#include "user.h"

// Assignment 9, Question 2(c):
// Write a user program faulttest.c that calls sbrk(32 * 4096),
// writes to exactly 5 different pages, and prints the number of page faults handled
// (return it through a small system call getfaults()). The result must be 5.
// Also show that accessing an address above the allocated size kills only that process.

int
main(int argc, char *argv[])
{
  printf(1, "==================================================\n");
  printf(1, "  faulttest: Question 2 Lazy Allocation Test\n");
  printf(1, "==================================================\n\n");

  int faults_init = getfaults();
  printf(1, "Initial page faults handled for process: %d\n", faults_init);

  printf(1, "\nCalling sbrk(32 * 4096) to reserve 32 pages lazily...\n");
  char *ptr = sbrk(32 * 4096);
  if(ptr == (char*)-1){
    printf(1, "faulttest: sbrk failed!\n");
    exit();
  }
  printf(1, "sbrk returned base address: 0x%x\n", (uint)ptr);

  int faults_after_sbrk = getfaults();
  printf(1, "Page faults immediately after sbrk: %d (expected 0, lazy allocation)\n", faults_after_sbrk);

  // Write to exactly 5 distinct pages: 0, 4, 11, 22, 31
  int pages_to_write[5] = {0, 4, 11, 22, 31};
  printf(1, "\nWriting to 5 different pages: page 0, page 4, page 11, page 22, page 31...\n");
  for(int i = 0; i < 5; i++){
    int page_num = pages_to_write[i];
    uint addr = (uint)(ptr + page_num * 4096);
    *(ptr + page_num * 4096) = 'A' + i;
    printf(1, "  Wrote to page %d at address 0x%x -> current fault count: %d\n",
           page_num, addr, getfaults());
  }

  int faults_after_writes = getfaults();
  printf(1, "\nTotal page faults handled: %d (Expected: 5)\n", faults_after_writes);
  if(faults_after_writes == 5){
    printf(1, "SUCCESS: Exactly 5 page faults were handled by the kernel!\n");
  } else {
    printf(1, "FAILURE: Fault count mismatch!\n");
  }

  printf(1, "\nTesting accessing an address above the allocated size in a child process...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "faulttest: fork failed!\n");
    exit();
  }

  if(pid == 0){
    // Child process: attempt to access an address beyond curproc->sz
    // allocated size is ptr + 32 * 4096.
    char *illegal_addr = ptr + 32 * 4096 + 4096;
    printf(1, "Child [pid %d]: attempting to write to address above allocated size (0x%x)...\n",
           getpid(), (uint)illegal_addr);
    *illegal_addr = 'X'; // Should trigger unhandled page fault and kernel kills child
    printf(1, "ERROR: Child did not get killed!\n");
    exit();
  }

  // Parent process
  wait();
  printf(1, "Parent: Child process terminated by kernel due to out-of-bounds page fault.\n");
  printf(1, "Parent: Survived successfully! Only the violating process was killed.\n");
  printf(1, "==================================================\n");
  exit();
}
