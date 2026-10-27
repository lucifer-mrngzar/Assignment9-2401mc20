//Ranveer Gupta
//2401MC20

#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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

  // Assignment 9, Question 2(a):
  // When n > 0, lazy allocation: only increase myproc()->sz, do not allocate physical memory.
  if(n > 0){
    myproc()->sz += n;
  } else if(n < 0){
    // When n < 0, deallocate memory.
    if(growproc(n) < 0)
      return -1;
  }
  return addr;
}

// Assignment 9, Question 1:
// System call vmprint prints the user page table mappings.
int
sys_vmprint(void)
{
  return vmprint();
}

// Assignment 9, Question 2:
// System call getfaults returns the number of page faults handled for the calling process.
int
sys_getfaults(void)
{
  return myproc()->pgfaults;
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
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
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
