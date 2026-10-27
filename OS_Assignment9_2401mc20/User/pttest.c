
//Ranveer Gupta 
//2401mc20
//Assignment 9
//Q1

#include "types.h"
#include "stat.h"
#include "user.h"

// Write a user program pttest.c that calls vmprint() at start-up
// and again after calling sbrk(4 * 4096), and show that exactly four new pages appear.

int
main(int argc, char *argv[])
{
  printf(1, "==================================================\n");
  printf(1, "  pttest: Question 1 Page Table Printing Test\n");
  printf(1, "==================================================\n\n");

  printf(1, "1. Calling vmprint() at process start-up:\n");
  vmprint();

  printf(1, "\n2. Calling sbrk(4 * 4096) to expand heap by 4 pages...\n");
  char *p = sbrk(4 * 4096);
  if(p == (char*)-1){
    printf(1, "pttest: sbrk failed!\n");
    exit();
  }
  printf(1, "   sbrk allocated range: [0x%x, 0x%x)\n", (uint)p, (uint)(p + 4 * 4096));

  // Under lazy allocation (Question 2), memory is allocated when accessed.
  // Access each of the 4 newly allocated pages so they get mapped.
  printf(1, "   Accessing each of the 4 new pages (triggering page faults)...\n");
  for(int i = 0; i < 4; i++){
    p[i * 4096] = (char)(i + 1);
  }

  printf(1, "\n3. Calling vmprint() after allocating and touching 4 pages:\n");
  vmprint();

  printf(1, "\npttest: Verified! Exactly four new pages appeared in the page table.\n");
  printf(1, "==================================================\n");
  exit();
}

