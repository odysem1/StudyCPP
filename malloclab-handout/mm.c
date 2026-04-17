/*
-----------------------
version 0.2
TO-DO: implement find_fit, place function
and change readme style
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
#define WSIZE 4             /*Word and header/footer size (bytes)*/
#define DSIZE 8             /*Double word size (bytes)*/
#define CHUNKSIZE (1<<12)   /*Extend heap by this amount (bytes)*/

#define MAX(x, y) ((x) > (y) ? (x):(y))

/*Pack a size and allocated bit into a word*/
#define PACK(size, alloc) ((size) | (alloc))

/*Read and write a word at address p*/
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = val)

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


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*Address of the start of the heap*/
static char *heap_listp;


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
*/

static void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /*Case 1*/
    if (prev_alloc && next_alloc){
        return bp;
    }

    /*Case 2*/
    else if (prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    /*Case 3*/
    else if (!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    /*Case 4*/
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        bp = PREV_BLKP(bp);
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    return bp;  

}

/*
* find-fit: find allocatable block(first-fit)
*/
static void *find_fit(size_t asize){
    void *bp;
    
    /*heap_listp(start) to epiloge(size 0)*/
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }

    return NULL;    /*NO HIT*/
}

/*
* place: allocate the memory
*    if the needed size is smalller than the block, then
*    divide the block and allocate only the needed size
*/
static void place(void *bp, size_t asize){
    /*Check size of the current block*/
    size_t csize = GET_SIZE(HDRP(bp));
    if ((csize - asize) >= (2*DSIZE)){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
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

    /*Create the initial empty heap*/
    if ((heap_listp = (char *)mem_sbrk(4*WSIZE)) == (char *)-1)
        return -1;
    PUT(heap_listp, 0);                             /*Alignment padding*/
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    /*Prologue header*/
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    /*Prologue footer*/
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        /*Epilogue header*/
    heap_listp += (2*WSIZE);

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
    if (size <= DSIZE)
        asize = 2*DSIZE;
    else
        asize = DSIZE * ((size + DSIZE + (DSIZE-1)) / DSIZE);

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
    
    /*Same as malloc if bp is null*/
    if (bp == NULL) return mm_malloc(size);

    /*Free memory if size is 0*/
    if (size == 0){
        mm_free(bp);
        return NULL;
    }

    /*Allocate memory first*/
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /*Check old size, copy and free*/
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














