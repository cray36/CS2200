#include "cachesim.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

u_int64_t c, s, b;
int global_clock;

typedef struct {
    int dirty;
    int access_time;
    int tag;
    u_char* data;
} BLOCK;

typedef struct {
    BLOCK* blocks;
} SET;

SET* cache;

u_int64_t get_index(u_int64_t addr) {
    addr >>= b;
    int index_mask = ~(-1 << s); 
    return addr & index_mask;
}


u_int64_t get_tag(u_int64_t addr) {
    addr >>= (b+s); 
    int tag_mask = (1 << (64-b-s)) -1;  //00001000 - 1 = 00000111
    return addr & tag_mask;
}

/**
 * Subroutint for initializing your cache with the paramters.
 * You may add and initialize any global variables that you might need
 *
 * @param C The total size of your cache is 2^size bytes
 * @param S The total number of sets in your cache are 2^S
 * @param B The size of your block is 2^B bytes
 */
void cache_init(uint64_t C, uint64_t S, uint64_t B) {
    global_clock = 0;
    c = C; s = S; b = B;
    int blocks_per_set = 1 << (c - b - s);
    int num_sets = 1 << s;
    int num_bytes_per_block = 1 << b;
    cache = (SET*)(malloc(num_sets* sizeof(SET)));
    if(NULL == cache) {
        // handle error
    }
    else {
	int i;
        for(i=0; i<num_sets; i++) {
            cache[i].blocks = (BLOCK*)(malloc(blocks_per_set* sizeof(BLOCK)));
            if (NULL == cache[i].blocks) {
                //handle error
            }
            else {
		int j;
                for(j= 0; j<blocks_per_set; j++){
		    cache[i].blocks[j].dirty = 0;
                    cache[i].blocks[j].tag = 0;
		    cache[i].blocks[j].access_time = 0;
                    cache[i].blocks[j].data = (u_char*)(malloc(sizeof(u_char)*num_bytes_per_block));
                }
            }
        }
    }
}

/**
 * Subroutine that simulates one cache event at a time.
 * @param rw The type of access, READ or WRITE
 * @param address The address that is being accessed
 * @param stats The struct that you are supposed to store the stats in
 */
void cache_access (char rw, uint64_t address, struct cache_stats_t *stats) {
    
    global_clock++;
    stats->accesses++;
    
    int blocks_per_set = 1 << (c - b - s);
    int num_sets = 1 << s;
    int num_bytes_per_block = 1 << b;
    
    u_int64_t index = get_index(address);
    u_int64_t tag = get_tag(address);
    SET set = cache[index];
    
    int lru_index = 0;
    int lru_accesstime = global_clock;
    
    if(rw == READ) {
        stats->reads++;
        int miss = 1;
	int i;
        for(i=0; i<blocks_per_set; i++) {
            if (set.blocks[i].access_time < lru_accesstime) {
                lru_accesstime = set.blocks[i].access_time;
                lru_index = i;
            }
            if (set.blocks[i].tag == tag) {
                miss = 0;
                set.blocks[i].access_time = global_clock;
            }
        }
        if (miss) {
            stats->read_misses++;
            set.blocks[lru_index].tag = tag;
            set.blocks[lru_index].access_time = global_clock;
            if (set.blocks[lru_index].dirty) {
		
                stats->write_backs++;
            }
	    set.blocks[lru_index].dirty =0;
        }
    }
    else if (rw == WRITE) {
        stats->writes++;
        int miss = 1;
	int i;
        for( i=0; i<blocks_per_set; i++) {
            if (set.blocks[i].access_time < lru_accesstime) {
                lru_accesstime = set.blocks[i].access_time;
                lru_index = i;
            }
            if (set.blocks[i].tag == tag) {
                miss = 0;
                set.blocks[i].access_time = global_clock;
                set.blocks[i].dirty = 1;
            }
        }
        if (miss) {
            stats->write_misses++;
            set.blocks[lru_index].tag = tag;
            set.blocks[lru_index].access_time = global_clock;
	    if (set.blocks[lru_index].dirty) {
                stats->write_backs++;
            }
            set.blocks[lru_index].dirty = 1;
	    
        }
    }
    else {
        // handle error
    }
}

/**
 * Subroutine for cleaning up memory operations and doing any calculations
 *
 */
void cache_cleanup (struct cache_stats_t *stats) {
    
    stats->misses = stats->read_misses + stats->write_misses;
    stats->miss_rate = stats->misses/((float)stats->accesses);
    stats->avg_access_time = stats->access_time + stats->miss_rate*stats->miss_penalty;
    
    //Clean up
    int blocks_per_set = 1 << (c - b - s);
    int num_sets = 1 << s;
    int i;
    for(i= 0; i<num_sets; i++) {
        SET set = cache[i];
	int j;
        for ( j = 0; j<blocks_per_set; j++) {
            free(set.blocks[j].data);
        }
        free(set.blocks);
    }
    free(cache);
}

