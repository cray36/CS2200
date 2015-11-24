#include <stdlib.h>
#include <stdio.h>
#include "tlb.h"
#include "pagetable.h"
#include "global.h" /* for tlb_size */
#include "statistics.h"

/*******************************************************************************
 * Looks up an address in the TLB. If no entry is found, attempts to access the
 * current page table via cpu_pagetable_lookup().
 *
 * @param vpn The virtual page number to lookup.
 * @param write If the access is a write, this is 1. Otherwise, it is 0.
 * @return The physical frame number of the page we are accessing.
 */
pfn_t tlb_lookup(vpn_t vpn, int write) {
   pfn_t pfn;
    int i;
    int foundPosition = -1;
    tlbe_t *current_entry_ptr;

   /* 
    * FIX ME : Step 6
    */

   /* 
    * Search the TLB for the given VPN. Make sure to increment count_tlbhits if
    * it was a hit!
    */
    for (i = 0; i < tlb_size && foundPosition < 0; i++) {
        current_entry_ptr = tlb + i;
        if (current_entry_ptr->valid && current_entry_ptr->vpn == vpn) {
            foundPosition = i;
        }
	
    }

    if (foundPosition < 0)
    {
        /*cpu_pagetable_lookup();*/
        /* If it does not exist (it was not a hit), call the page table reader */
        pfn = pagetable_lookup(vpn, write);

        /*
         * Replace an entry in the TLB if we missed. Pick invalid entries first,
         * then do a clock-sweep to find a victim.
         */
        for (i = 0; i < tlb_size && foundPosition < 0; i++) {
            if (!tlb[i].valid) {
                foundPosition = i;
            }
            else {
                if (1 == tlb[i].used) {
                    tlb[i].used = 0;
                }
                else {
                    foundPosition = i;
                }
            }
        }
    }
    else {
        /***
            if found, increase tlb hits
        */
        count_tlbhits++;
    }
    /*****
    * If all the tlb entry is used, then evict the first one
    */
    if (foundPosition < 0) {
        foundPosition = 0;
    }
    pfn = tlb[foundPosition].pfn;
    tlb[foundPosition].used = 1;
    tlb[foundPosition].vpn = vpn;
    tlb[foundPosition].dirty = write;
    tlb[foundPosition].valid = 1;
   /*
    * Perform TLB house keeping. This means marking the found TLB entry as
    * accessed and if we had a write, dirty. We also need to update the page
    * table in memory with the same data.
    *
    * We'll assume that this write is scheduled and the CPU doesn't actually
    * have to wait for it to finish (there wouldn't be much point to a TLB if
    * we didn't!).
    */

   return pfn;
}

