/*
 * CS3375 Computer Architecture
 * Course Project
 * Cache Simulator Design and Development
 * FALL 2017
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cachesim.h"

/*for plru*/

    uint64_t tree[WAY_SIZE*2-1] = {0};

int main(int argc, char *argv[])
{
    FILE *f = fopen("replacement_tests.txt","a");
    if (argc != 3) {
        printf("Usage: %s <direct> <trace file name>\n", argv[0]);
        return 1;
    }

#ifdef DBG
    printf("BLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
    printf("%d-WAY\n", WAY_SIZE);
    printf("CACHE SIZE = %d Bytes\n", CACHE_SIZE);
    printf("NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
    printf("NUMBER OF SETS = %d\n", NUM_SETS);
    printf("\n");
#endif

    char* trace_file_name = argv[2];
    struct direct_mapped_cache d_cache;
    char mem_request[20];
    uint64_t address;
    FILE *fp;


    /* Initialization */
    for (int i=0; i<NUM_BLOCKS; i++) {
        d_cache.valid_field[i] = 0;
        d_cache.dirty_field[i] = 0;
        d_cache.tag_field[i] = 0;
    }
    d_cache.hits = 0;
    d_cache.misses = 0;


    for (int i =WAY_SIZE-1;i<WAY_SIZE*2-1;i++){
        tree[i]=i-(WAY_SIZE-1);
    }
    for (int i =0;i<WAY_SIZE*2-1;i++){
        printf("tree[%d]=%d\n",i,tree[i]);
    }
    printf("\n\n");


    /* Opening the memory trace file */
    fp = fopen(trace_file_name, "r");



    if (strncmp(argv[1], "direct", 6)==0) { /* Simulating direct-mapped cache */
        /* Read the memory request address and access the cache */
        while (fgets(mem_request, 20, fp)!= NULL) {
            address = convert_address(mem_request);
            direct_mapped_cache_access(&d_cache, address);
        }
        /*Print out the results*/
        printf("\n==================================\n");
        printf("Cache type:    Direct-Mapped Cache\n");
        printf("==================================\n");
        printf("Cache Hits:    %d\n", d_cache.hits);
        printf("Cache Misses:  %d\n", d_cache.misses);
        printf("Hit Rate:      %f\n", (float)d_cache.hits/(float)(d_cache.hits+d_cache.misses)); //print hit rate
        printf("Miss Rate:     %f\n", (float)d_cache.misses/(float)(d_cache.hits+d_cache.misses)); // print miss rate
        printf("\n");
    }




    else if (strncmp(argv[1], "full", 4)==0) { /* Simulating fully-associative cache */
        /* Read the memory request address and access the cache */
        while (fgets(mem_request, 20, fp)!= NULL) {
            address = convert_address(mem_request);
            fully_associative_cache_access(&d_cache, address);
        }
        /*Print out the results*/
        printf("\n======================================\n");
        printf("Cache type:    Fully/N-WAY SET Associative Cache\n");
        printf("======================================\n");
        printf("Cache Hits:    %d\n", d_cache.hits);
        printf("Cache Misses:  %d\n", d_cache.misses);
        printf("Hit Rate:      %f\n", (float)d_cache.hits/(float)(d_cache.hits+d_cache.misses));
        printf("Miss Rate:     %f\n", (float)d_cache.misses/(float)(d_cache.hits+d_cache.misses));
        printf("\n");
    }
    fprintf(f,"======================================\n"); //write tests to file
    fprintf(f,"BLOCK SIZE = %d Bytes\n", BLOCK_SIZE);
    fprintf(f,"%d-WAY\n", WAY_SIZE);
    fprintf(f,"CACHE SIZE = %d Bytes\n", CACHE_SIZE);
    fprintf(f,"NUMBER OF BLOCKS = %d\n", NUM_BLOCKS);
    fprintf(f,"NUMBER OF SETS = %d\n", NUM_SETS);
    fprintf(f,"Cache Hits:    %d\n", d_cache.hits);
    fprintf(f,"Cache Misses:  %d\n", d_cache.misses);
    fprintf(f,"Hit Rate:      %f\n", (float)d_cache.hits/(float)(d_cache.hits+d_cache.misses));
    fprintf(f,"Miss Rate:     %f\n", (float)d_cache.misses/(float)(d_cache.hits+d_cache.misses));
    fprintf(f,"======================================\n\n\n\n\n");

    fclose(fp);
    fclose(f);

    return 0;
}

uint64_t convert_address(char memory_addr[])
/* Converts the physical 32-bit address in the trace file to the "binary" \\
 * (a uint64 that can have bitwise operations on it) */
{
    uint64_t binary = 0;
    int i = 0;

    while (memory_addr[i] != '\n') {
        if (memory_addr[i] <= '9' && memory_addr[i] >= '0') {
            binary = (binary*16) + (memory_addr[i] - '0');
        } else {
            if(memory_addr[i] == 'a' || memory_addr[i] == 'A') {
                binary = (binary*16) + 10;
            }
            if(memory_addr[i] == 'b' || memory_addr[i] == 'B') {
                binary = (binary*16) + 11;
            }
            if(memory_addr[i] == 'c' || memory_addr[i] == 'C') {
                binary = (binary*16) + 12;
            }
            if(memory_addr[i] == 'd' || memory_addr[i] == 'D') {
                binary = (binary*16) + 13;
            }
            if(memory_addr[i] == 'e' || memory_addr[i] == 'E') {
                binary = (binary*16) + 14;
            }
            if(memory_addr[i] == 'f' || memory_addr[i] == 'F') {
                binary = (binary*16) + 15;
            }
        }
        i++;
    }

#ifdef DBG
    printf("%s converted to %llu\n", memory_addr, binary);
#endif
    return binary;
}

void direct_mapped_cache_access(struct direct_mapped_cache *cache, uint64_t address)
{
    uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
    uint64_t index = block_addr % NUM_BLOCKS;
    uint64_t tag = block_addr >> (unsigned)log2(NUM_BLOCKS);

#ifdef DBG
    printf("Memory address: %llu, Block address: %llu, Index: %llu, Tag: %llu ", address, block_addr, index, tag);
#endif

    if (cache->valid_field[index] && cache->tag_field[index] == tag) { /* Cache hit */
        cache->hits += 1;
#ifdef DBG
        printf("Hit!\n");
#endif
    } else {
        /* Cache miss */
        cache->misses += 1;
#ifdef DBG
        printf("Miss!\n");
#endif
        if (cache->valid_field[index] && cache->dirty_field[index]) {
            /* Write the cache block back to memory */
        }
        cache->tag_field[index] = tag;
        cache->valid_field[index] = 1;
        cache->dirty_field[index] = 0;
    }
}


void fully_associative_cache_access(struct direct_mapped_cache *cache, uint64_t address)//also works for n way set associative
{
    uint64_t j;
    uint64_t block_addr = address >> (unsigned)log2(BLOCK_SIZE);
    uint64_t index = block_addr % NUM_BLOCKS;
    uint64_t tag = block_addr >> (unsigned)log2(NUM_BLOCKS);
    bool hit = false;
    bool updated = false;


#ifdef DBG
    printf("Memory address: %llu, Block address: %llu, Set: %llu, Index: %llu, Tag: %llu ", address, block_addr, block_addr%NUM_SETS, index, tag);
#endif


//PLRU

                /*for (j=0; j<(WAY_SIZE-1);){
                    //printf("j=%llu\n",j);
                    if (tree[j]==0){
                        //printf("\tthis\n");
                        tree[j]=1;
                        //printf("\t%llu\n",tree[j]);
                        j=j*2+1;
                    }
                    else{
                        //printf("\telse\n");
                        tree[j]=0;
                        //printf("\t%llu\n",tree[j]);
                        j=j*2+2;
                    }
                }*/
                //printf("***NEXT***\n");


    //printf("set: %llu\n",block_addr%NUM_SETS,tag);
    for (uint64_t i = block_addr%NUM_SETS; i<NUM_BLOCKS;i+=NUM_SETS){

        //printf("%llu\n\tvalid: %d\n",(i-(block_addr%NUM_SETS))/NUM_SETS,cache->valid_field[i]);
        //printf("valid field checked: %llu\n",i);
        if (cache->valid_field[i] && cache->tag_field[i] == tag) { /* Cache hit */

            cache->hits += 1;
#ifdef DBG
        printf("Hit!\n");
#endif
            hit=true;
            break;
        }
    }

         if (!hit) {
        /* Cache miss */
            cache->misses += 1;
#ifdef DBG
        printf("Miss!\n");
#endif

            //RR

            uint64_t r=rand()%WAY_SIZE;
            cache->valid_field[r*NUM_SETS+(block_addr%NUM_SETS)]=1;
            cache->tag_field[r*NUM_SETS+(block_addr%NUM_SETS)]=tag;


            //NRU

            /*for (uint64_t i = block_addr%NUM_SETS; i<NUM_BLOCKS;i+=NUM_SETS) {
                if (!cache->valid_field[i]){
                    cache->tag_field[i] = tag;
                    cache->valid_field[i] = 1;
                    updated=true;
                    break;
                }
            }
            if (!updated){



                for (uint64_t i = block_addr%NUM_SETS; i<NUM_BLOCKS;i+=NUM_SETS){
                    cache->valid_field[i] = 0;
                }
                cache->tag_field[0]=tag;
                cache->valid_field[0]=1;
            }*/




            //PLRU
                /*for (j=0; j<(WAY_SIZE-1);){
                    //printf("miss \n");
                    if (tree[j]==0){
                        j=j*2+1;
                    }
                    else{
                        j=j*2+2;
                    }
                    //printf("tree[%d]=%d\n",j,tree[j]);
                }

                //printf("valid field replaced: %llu\n",tree[j]*NUM_SETS+(block_addr%NUM_SETS));
                cache->valid_field[tree[j]*NUM_SETS+(block_addr%NUM_SETS)]=1;
                cache->tag_field[tree[j]*NUM_SETS+(block_addr%NUM_SETS)]=tag;*/





            /*if (cache->valid_field[index] && cache->dirty_field[index]) {
             Write the cache block back to memory
            }

            cache->tag_field[index] = tag;
            cache->valid_field[index] = 1;
            cache->dirty_field[index] = 0;*/
    }
}

