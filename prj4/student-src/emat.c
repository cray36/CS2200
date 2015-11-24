#include "statistics.h"

#define MEMORY_ACCESS_TIME      100 /* 100 ns */
#define DISK_ACCESS_TIME   10000000 /* 10 million ns = 10 ms */

double compute_emat() {
   /* FIX ME - Compute the average mean access time.  You should only need the
    * numbers contained in the following variables. You may or may not need to
    * use them all:
    *    count_pagefaults   - the number of page faults that occurred
    *                         (note: this _does_ include the unavoidable page
    *                                fault when a process is first brought into
    *                                memory upon starting)
    *    count_tlbhits      - the number of tlbhits that occurred
    *    count_writes       - the number of stores/writes that occurred
    *    count_reads        - the number of reads that occurred
    *
    * Any other values you might need are composites of the above values.  Some
    * of these computations have been done for you, in case you need them.
    */

   long int total_accesses = count_writes + count_reads; 
   long int tlb_misses     = total_accesses - count_tlbhits;
    /**
    * since tlb hit takes no time (according to the instruction),
    * access time is computed solely with tlb_misses
    *
    * everytime tlb is missed, there are 1 disk access and 2 memory access
    */

   return count_tlbhits * MEMORY_ACCESS_TIME + tlb_misses * (2 * MEMORY_ACCESS_TIME) + total_accesses * DISK_ACCESS_TIME;
}
