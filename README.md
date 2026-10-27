# Operating Systems — Assignment 9
<!-- Student Name: Ranveer Gupta -->
<!-- Roll Number: 2401MC20 -->
<!-- Assignment: 9 — Memory Management in xv6 (x86) -->

**Student:** Ranveer Gupta  
**Roll Number:** 2401MC20  
**Course:** Operating Systems  
**Assignment:** 9 — Page Tables, Lazy Allocation, and Page Replacement

## Overview

This assignment explores three memory-management concepts using the xv6 operating system for x86:

1. Inspecting the page-table mappings of a running process.
2. Implementing lazy memory allocation through page-fault handling.
3. Simulating FIFO and LRU page-replacement policies and comparing their performance.

The first two tasks involve changes to the xv6 kernel and the user-to-kernel system-call interface. The third task implements a user-space simulator that evaluates page replacement using a predefined reference string.

## 1. Process Page-Table Inspection

### Objective

Implement `vmprint()` to examine the calling process's page tables and display the virtual-to-physical mappings of valid user pages.

### Implementation

**Kernel-side page-table traversal — `vm.c`**

- Traverse the process page directory and its associated page tables.
- Use the existing xv6 page-table structures and `walkpgdir()` as a reference.
- Examine user-space addresses below `KERNBASE`, excluding the kernel portion of the address space.
- For each valid user mapping, display the virtual address, physical frame address, and the `P`, `W`, and `U` permission flags.
- Maintain counters for valid user pages and page-table pages.

**System-call interface**

The new system call is connected through the relevant kernel and user-interface files:

- `sysproc.c` — kernel-side system-call handler.
- `syscall.h` and `syscall.c` — system-call number and dispatch-table entry.
- `defs.h` — kernel function declaration, where required.
- `user.h` and `usys.S` — user-level declaration and system-call entry stub.

**Test program — `pttest.c`**

The test program performs the following sequence:

1. Calls `vmprint()` to record the initial user-space mappings.
2. Requests additional memory using `sbrk(4 * 4096)`.
3. Calls `vmprint()` again to inspect the updated mappings.

The output should demonstrate four additional valid user-page mappings, provided the allocation succeeds and the test is run under the assignment's expected allocation conditions.

## 2. Lazy Allocation and Page-Fault Handling

### Objective

Change the memory-allocation process so that increasing a process's size does not immediately allocate physical memory. Instead, physical pages are allocated when the process first accesses the corresponding virtual addresses.

### Kernel modifications

**A. Deferred allocation — `sysproc.c`**

Modify `sys_sbrk()` so that a positive increment updates the process's logical size without immediately allocating physical pages. The implementation must still handle invalid size changes and preserve correct return-value semantics.

**B. Support for unmapped pages — `vm.c`**

Update `deallocuvm()` and `copyuvm()` to handle virtual ranges containing pages that have not yet been physically allocated.

- `deallocuvm()` must safely skip absent page mappings while releasing allocated memory.
- `copyuvm()` must avoid treating a missing page as an automatic kernel error when it represents a legitimate lazy allocation.

These changes are necessary because a process's virtual address range may include pages that do not yet have physical frames.

**C. Page-fault handling — `trap.c`**

Handle the `T_PGFLT` trap for valid lazy-allocation accesses.

The intended sequence is:

1. Read the faulting virtual address using `rcr2()`.
2. Check that the address belongs to the process's valid allocated range.
3. Allocate a physical page with `kalloc()`.
4. Initialize the page to zero using `memset()`.
5. Map the page using `mappages()` with the appropriate `PTE_W` and `PTE_U` permissions.
6. Increment the current process's page-fault counter.
7. Resume execution if the mapping succeeds.

If the address is invalid, or allocation or mapping fails, handle the error safely rather than resuming with an invalid mapping. An invalid user access should terminate the offending process without unnecessarily terminating the entire system.

**D. Page-fault counter**

Add a counter to `struct proc` in `proc.h` and expose it through the `getfaults()` system call. The system-call number, dispatch entry, kernel handler, declaration, and user stub must all be added consistently.

### User-space verification — `faulttest.c`

The test program should:

1. Request `32 * 4096` bytes using `sbrk()`.
2. Write to exactly five distinct pages that have not previously been allocated physically.
3. Call `getfaults()` and print the number of handled page faults.
4. Attempt an access outside the process's valid address range in a separate test process.

**Expected observations**

- Five first-time page accesses should produce five handled page faults, assuming no additional lazy faults occur during the measured interval.
- The invalid-access test should terminate the offending process rather than crash the entire operating system.

The test must measure the counter at the appropriate point and ensure that unrelated page faults do not affect the expected count.

## 3. FIFO and LRU Page-Replacement Simulation

### Objective

Implement a user-space simulator named `pagerepl.c` to compare First-In-First-Out (FIFO) and Least Recently Used (LRU) page-replacement algorithms.

This is a simulation of page replacement rather than a modification of xv6's actual physical-page replacement mechanism.

### Reference string

The required reference sequence is:

`7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`

Run the simulation with **3, 4, and 5 frames**.

### Algorithm descriptions

**FIFO — First-In-First-Out**

When all frames are occupied and a page fault occurs, replace the page that entered memory earliest. A page hit does not change the replacement order.

**LRU — Least Recently Used**

When all frames are occupied and a page fault occurs, replace the page that has not been accessed for the longest time. A page hit updates the page's recent-use information.

### Program requirements

The simulator should:

- Accept the frame count and reference string as input, or provide a clear default for the prescribed test case.
- Process each reference in sequence.
- Display the current frame contents after each reference.
- Identify each reference as a hit or a page fault.
- Maintain separate fault and hit counters for FIFO and LRU.
- Calculate the hit ratio using:

  `Hit ratio = Number of hits / Total number of references`

- Print a comparison table for the three required frame counts.

### Expected result for three frames

| Algorithm | Expected page faults |
|---|---:|
| FIFO | 15 |
| LRU | 12 |

The results for four and five frames should be obtained by running the simulator. The final comparison table must contain the actual counts and hit ratios produced by the program.

## 4. Building and Running xv6

### Prerequisites

- xv6 source tree for x86.
- An x86-compatible cross-compiler and binutils.
- QEMU with the `qemu-system-i386` emulator.
- A compatible Unix-like development environment, such as Linux or macOS with the necessary toolchain installed.

The commands below assume that the repository's Makefile supports the `TOOLPREFIX` and `qemu-nox` options.

### Build

Clean the previous build artifacts:

```bash
make clean
```

Compile xv6 and the included user programs:

```bash
make TOOLPREFIX=x86_64-elf-
```

**Toolchain note:** The tool prefix must match the compiler installed on your machine and the architecture expected by your xv6 source tree. Traditional xv6 x86 builds commonly use an `i386-jos-elf-` or `i686-elf-` prefix; `x86_64-elf-` should be used only if the required tools and Makefile configuration support it.

### Run

Start xv6 in a terminal-based QEMU session:

```bash
make qemu-nox TOOLPREFIX=x86_64-elf-
```

After booting, run the user programs from the xv6 shell:

```text
pttest
faulttest
pagerepl
```

These commands assume that all three programs have been added to the user-program list in the Makefile and compiled successfully.

## 5. Files and Changes

The submission should include the relevant modified kernel files, system-call interface files, and user programs.

| File | Purpose |
|---|---|
| `vm.c` | Page-table inspection and support for absent mappings |
| `trap.c` | Lazy-allocation page-fault handling |
| `proc.h` | Per-process page-fault counter |
| `sysproc.c` | System-call handlers and deferred `sbrk()` allocation |
| `syscall.h` | System-call number definitions |
| `syscall.c` | System-call dispatch table |
| `defs.h` | Kernel function declarations, where needed |
| `user.h` | User-level system-call declarations |
| `usys.S` | User-to-kernel system-call stubs |
| `pttest.c` | Page-table inspection test |
| `faulttest.c` | Lazy-allocation and page-fault test |
| `pagerepl.c` | FIFO and LRU simulator |
| `Makefile` | Inclusion of the new user programs |

Add comments to the modified source code explaining the purpose of each significant change.

## 6. Test Evidence and Submission

Include terminal screenshots that demonstrate the results of each question.

- **Question 1:** Initial and subsequent page-table output, including the valid-page and page-table-page totals.
- **Question 2:** The five-page fault count and the separate invalid-access test showing that only the offending process is terminated.
- **Question 3:** Per-reference FIFO/LRU output and the comparison table for three, four, and five frames.

Also include a one-page report describing the kernel changes, the purpose of each test, and the results observed.

## 7. Summary

This assignment demonstrates three distinct aspects of memory management:

- Page tables provide the mappings and permission information used to translate virtual addresses into physical addresses.
- Lazy allocation postpones physical-memory allocation until a page is accessed, reducing unnecessary allocation.
- FIFO and LRU illustrate how replacement policies make different decisions about which resident page to evict when memory frames are full.

Correctness should be established through the actual xv6 build, test-program output, and screenshots included with the submission.
