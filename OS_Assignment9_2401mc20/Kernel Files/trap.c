//Ranveer Gupta
//2401MC20

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  // Assignment 9, Question 2(b):
  // Handle page faults for lazy allocation.
  case T_PGFLT: {
    uint va = rcr2();
    struct proc *curproc = myproc();
    if(curproc != 0 && va < curproc->sz && va < KERNBASE){
      // Check if page is already present (e.g. protection violation on stack guard page)
      pte_t *pte = walkpgdir(curproc->pgdir, (void*)va, 0);
      if(pte != 0 && (*pte & PTE_P)){
        cprintf("pid %d %s: protection page fault at addr 0x%x--kill proc\n",
                curproc->pid, curproc->name, va);
        curproc->killed = 1;
        break;
      }
      // Allocate a zeroed page with kalloc() and memset()
      char *mem = kalloc();
      if(mem == 0){
        cprintf("pid %d %s: lazy alloc out of memory--kill proc\n",
                curproc->pid, curproc->name);
        curproc->killed = 1;
        break;
      }
      memset(mem, 0, PGSIZE);
      // Map it with mappages() using PTE_W | PTE_U
      if(mappages(curproc->pgdir, (char*)PGROUNDDOWN(va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0){
        cprintf("pid %d %s: lazy alloc mappages failed--kill proc\n",
                curproc->pid, curproc->name);
        kfree(mem);
        curproc->killed = 1;
        break;
      }
      // Count the fault in the new field of struct proc
      curproc->pgfaults++;
      // Resume the process
      break;
    } else {
      // Otherwise print the pid and address and kill the process.
      cprintf("pid %d %s: page fault at addr 0x%x--kill proc\n",
              curproc ? curproc->pid : -1,
              curproc ? curproc->name : "",
              va);
      if(curproc)
        curproc->killed = 1;
      break;
    }
  }

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
