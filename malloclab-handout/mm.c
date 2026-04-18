/*
-----------------------
version 0.4.1
TO-DO: Change to best-fit
-----------------------
*/



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "SeokSoo",
    /* First member's full name */
    "Hyunseung Shin",
    /* First member's email address */
    "seoksoo@kaist.ac.kr",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};


/* Basic constants and macros*/
#define WSIZE 8             /*Word and header/footer size (bytes)*/
#define DSIZE 16             /*Double word size (bytes)*/
#define CHUNKSIZE (1<<12)   /*Extend heap by this amount (bytes)*/

#define MAX(x, y) ((x) > (y) ? (x):(y))

/*Pack a size and allocated bit into a word*/
#define PACK(size, alloc) ((size) | (alloc))

/*Read and write a word at address p*/
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = val)

/*Read and write a address at address p - 64bit issue*/
#define GET_ADDR(p)      (*(void **)(p))
#define PUT_ADDR(p, val) (*(void **)(p) = val)

/*Read the size and allocated fields form address p*/
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/*Given block ptr bp, compute address of its header and footer*/
#define HDRP(bp)    ((char *)(bp) - WSIZE)
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/*Given block ptr bp, compute address of next and previous blocks*/
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/*Macros for implementing doubly linked list*/
#define GET_NEXT_FREE(bp) (GET_ADDR((char *)bp + WSIZE))
#define GET_PREV_FREE(bp) (GET_ADDR(bp))
#define SET_NEXT_FREE(bp, ptr) (PUT_ADDR((char *)(bp) + WSIZE, ptr))
#define SET_PREV_FREE(bp, ptr) (PUT_ADDR(bp, ptr))


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*Address of the start of the heap and usable memory*/
static char *heap_listp;
static char *st_heap_start;

/*MARCOs for segregated free list*/
#define LIST_LIMIT 20
#define GET_SEG_LIST(idx) (void **)((char *)(heap_listp) + ((idx) * WSIZE))



/*
* Helper functions - Free block list management
*/

static void insertNode(void *bp);
static void removeNode(void *bp);
int get_index(size_t size);
size_t get_size(size_t size);

/*
* get_size - make size aligned
*/
size_t get_size(size_t size){
    size_t asize;

    if (size <= DSIZE){
        asize = 2*DSIZE;
    }else{
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    return asize;
}

/*
* get_index - find index of which list to put based on size
*/
int get_index(size_t size){
    
    if (size <= 16) return 0;
    if (size <= 32) return 1;
    if (size <= 48) return 2;
    if (size <= 64) return 3;
    if (size <= 80) return 4;
    if (size <= 96) return 5;
    if (size <= 112) return 6;
    if (size <= 128) return 7;
    if (size <= 256) return 8;
    if (size <= 448) return 9;
    if (size <= 512) return 10;
    if (size <= 1024) return 11;
    if (size <= 2048) return 12;
    if (size <= 4096) return 13;
    if (size <= 8192) return 14;
    if (size <= 16384) return 15;
    if (size <= 32768) return 16;
    if (size <= 65536) return 17;
    if (size <= 131072) return 18;

    return 19;
}

/*
* insertNode - put Free block in list of free blocks with appropriate size
*/
static void insertNode(void *bp){
    int idx = get_index(GET_SIZE(HDRP(bp)));
    void **root_addr = GET_SEG_LIST(idx);
    void *free_listp = *root_addr;
    SET_NEXT_FREE(bp, free_listp);
    SET_PREV_FREE(bp, NULL);

    if (free_listp != NULL){
        SET_PREV_FREE(free_listp, bp);
    }
    PUT_ADDR(root_addr, bp);
}

/*
* removeNode - remove Free block in free block list of idx
*/
static void removeNode(void *bp){
    int idx = get_index(GET_SIZE(HDRP(bp)));
    void *root_addr = GET_SEG_LIST(idx);

    void *next_bp = GET_NEXT_FREE(bp);
    void *prev_bp = GET_PREV_FREE(bp);

    if (prev_bp == NULL){
        PUT_ADDR(root_addr, next_bp);
    }else{
        SET_NEXT_FREE(prev_bp, next_bp);
    }

    if (next_bp != NULL){
        SET_PREV_FREE(next_bp, prev_bp);
    }
}


/*
* Helper functions - Heap management
*/
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

/*
* extend_heap - Extends heap by calling mem_sbrk
*/
static void *extend_heap(size_t words){

    char *bp;
    size_t size;

    /*Allocate an even number of words to maintain alignment*/
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /*Initialize free block header/footer and epilogue header*/
    PUT(HDRP(bp), PACK(size, 0));   /*Free header*/
    PUT(FTRP(bp), PACK(size, 0));   /*Free footer*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /*New epilogue header*/

    /*Coalesce if the previous block was free*/
    return coalesce(bp);
}

/*
* coalesce - connects the adjacent free blocks
*   Also Removes previous free block and add new free block on
*   free block list
*/

static void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /*case 1: prev and next allocated, do nothing*/

    /*Case 2*/
    if (prev_alloc && !next_alloc){
        removeNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    /*Case 3*/
    else if (!prev_alloc && next_alloc){
        removeNode(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /*Case 4*/
    else if (!prev_alloc && !next_alloc){
        removeNode(PREV_BLKP(bp));
        removeNode(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    insertNode(bp);
    return bp;  

}

/*
* find-fit: find allocatable block(first-fit)
*/
static void *find_fit(size_t asize){
    void *bp;
    int idx = get_index(asize);

    /*start from least-needed idx*/
    while (idx < LIST_LIMIT){
        bp = (void *)GET_ADDR(GET_SEG_LIST(idx));
        
        while(bp != NULL){
            if (asize <= GET_SIZE(HDRP(bp))){
                return bp;
            }
            bp = GET_NEXT_FREE(bp);
        }
        idx++;
    }

    return NULL;    /*NO HIT*/
}

/*
* place: allocate the memory
*    if the needed size is smalller than the block, then
*    divide the block and allocate only the needed size
*    Also removes & adds new free block
*/
static void place(void *bp, size_t asize){
    /*Check size of the current block*/
    size_t csize = GET_SIZE(HDRP(bp));
    removeNode(bp);
    if ((csize - asize) >= (2*DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        insertNode(bp);
    }
    else{
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

/*--------------------------------------------*/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
    size_t list_space = LIST_LIMIT * WSIZE;

    /*Create the initial empty heap*/
    if ((heap_listp = (char *)mem_sbrk(list_space + (4*WSIZE))) == (char *)-1)
        return -1;

    for (int i = 0; i < LIST_LIMIT; i++){
        PUT_ADDR(heap_listp + (i*WSIZE), NULL);
    }

    st_heap_start = heap_listp + list_space;

    
    PUT(st_heap_start, 0);                             /*Alignment padding*/
    PUT(st_heap_start + (1*WSIZE), PACK(DSIZE, 1));    /*Prologue header*/
    PUT(st_heap_start + (2*WSIZE), PACK(DSIZE, 1));    /*Prologue footer*/
    PUT(st_heap_start + (3*WSIZE), PACK(0, 1));        /*Epilogue header*/
    st_heap_start += (2*WSIZE);

    /*Extend the empty heap with a free block of CHUNKSIZE bytes*/
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size){

    size_t asize; /*Adjusted block size*/
    size_t extendsize; /*Amount to extend heap if no fit*/
    char *bp;

    /*Ignore meaningless requests*/
    if (size == 0)
        return NULL;

    /*Adjust block size to include overhead and alignmnet reqs*/
    asize = get_size(size);

    /*Search the free list for a fit*/
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }

    /*If no fit is found, Get more memory and place block*/
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Free a block at pointer bp
 *      puts Block in a free list
 */

void mm_free(void *bp){

    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - reallocate the memory in given size
 */

void *mm_realloc(void *bp, size_t size){

    void *oldptr = bp;
    void *newptr;
    size_t copySize;

    /*variables for next_block empty check*/
    size_t old_size = GET_SIZE(HDRP(bp));
    void *next_bp = NEXT_BLKP(bp);
    size_t next_size = GET_SIZE(HDRP(next_bp));
    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t asize = get_size(size);
    size_t total_size = old_size + next_size;

    /*Same as malloc if bp is null*/
    if (bp == NULL) return mm_malloc(size);

    /*Free memory if size is 0*/
    if (size == 0){
        mm_free(bp);
        return NULL;
    }

    /*if size is big enough*/
    if (old_size >= asize){
        return bp;
    }

    /*If next block is empty*/
    if(!next_alloc && (total_size >= asize)){
        removeNode(next_bp);
        if (total_size - asize >= (2*DSIZE)){
            PUT(HDRP(bp), PACK(asize, 1));
            PUT(FTRP(bp), PACK(asize, 1));
            void *remain_bp = NEXT_BLKP(bp);
            PUT(HDRP(remain_bp), PACK(total_size - asize, 0));
            PUT(FTRP(remain_bp), PACK(total_size - asize, 0));
            coalesce(remain_bp);
        }else{
            PUT(HDRP(bp), PACK(total_size, 1));
            PUT(FTRP(bp), PACK(total_size, 1));
        }
        return bp;
    }
    
    /*if bp is the last block*/
    else if(next_size == 0){
        size_t extend_size = asize - old_size;
        if (extend_heap(extend_size / WSIZE) == NULL)
            return NULL;
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        return bp;
    }

    /*Allocate memory first*/
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /*Check old size, copy and free*/
    copySize = old_size - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














