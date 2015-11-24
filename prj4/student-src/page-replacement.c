#include <stdlib.h>

#include "types.h"
#include "pagetable.h"
#include "global.h"
#include "process.h"

/*******************************************************************************
 * Finds a free physical frame. If none are available, uses a clock sweep
 * algorithm to find a used frame for eviction.
 *
 * @return The physical frame number of a free (or evictable) frame.
 */
pfn_t get_free_frame(void) {
   int i;
   pte_t *page_table;
   /* See if there are any free frames */
   for (i = 0; i < CPU_NUM_FRAMES; i++) {
      if (rlt[i].pcb == NULL) {
         return i;
      }
   }

       for (i = 0; i < CPU_NUM_FRAMES; i++) {
        vpn_t current_vpn = rlt[i].vpn;
        /*get to process's page table*/
        page_table = (rlt[i].pcb)->pagetable;
        /*if the frame is invalid*/
        if (!page_table[current_vpn].valid)
            /* return it */
            return i;
        else {
            /* if the frame is valid and not recently used */
            if (!page_table[current_vpn].used)
                /* return it */
                return i;
            else {
                /* give it another chance */
                page_table[current_vpn].used = 0;
            }
        }
    }

   /* FIX ME : Problem 5 */
   /* IMPLEMENT A CLOCK SWEEP ALGORITHM HERE */
   /* Note: Think of what kinds of frames can you return before you decide
      to evit one of the pages using the clock sweep and return that frame */

   /* If all else fails, return a random frame */
   return rand() % CPU_NUM_FRAMES;
}
