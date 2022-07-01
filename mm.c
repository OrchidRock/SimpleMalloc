#include "mm.h"
#include "memlib.h"
#include <stdio.h>

/* Value Debug macros  */
#define DEBUG_INIT  	1
#define DEBUG_MALLOC    2
#define DEBUG_FREE	4

#ifndef DEBUG
#define DEBUG 0
#endif

/* Basic constants and macros */
#define WSIZE	   4
#define DSIZE	   8
#define CHUNKSZE  (0x1<<12) /* Extend heap by this amount (bytes) */

#define MAX(x,y) ((x)>(y)?(x):(y))

/* Pack a size and allocated field into a word */
#define PACK(size,alloc) ((size) | (alloc))

/* Read and write a word at address p*/
#define GET(p) 		(*(unsigned int*)(p))
#define PUT(p,val)	(*(unsigned int*)(p)=(val))

/*Read the size and allocated field from address p*/
#define GET_SIZE(p) 	(GET(p) & ~0x7)
#define GET_ALLOC(p)	(GET(p) & 0x1)

/* Given block ptr bp, computer address of its header and footer*/
#define HDRP(bp) 	((char*)(bp) - WSIZE)
#define FTRP(bp)	((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, computer address of next and previous blocks*/
#define NEXT_BLKP(bp)	((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)	((char*)(bp) - GET_SIZE(((char*)(bp)-DSIZE)))

static char* heap_listp ; /* The global varible that always point to
			   prologue clock*/


static void* coalesce(void* bp){
	size_t prev_alloc=GET_ALLOC(FTRP(PREV_BLKP(bp)));
	size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t size=GET_SIZE(HDRP(bp));

	if(prev_alloc && next_alloc)	
		return bp;
	else if(prev_alloc && ! next_alloc){
		size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
		PUT(HDRP(bp),PACK(size,0));
		PUT(FTRP(bp),PACK(size,0));
	}else if(!prev_alloc && next_alloc){
		size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(FTRP(bp),PACK(size,0));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		bp=PREV_BLKP(bp);
	}else{
		size+=GET_SIZE(FTRP(NEXT_BLKP(bp)))+
			GET_SIZE(HDRP(PREV_BLKP(bp)));
		PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
		PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
		bp=PREV_BLKP(bp);
	}
#if DEBUG
	debug("coalesce");
#endif

	return bp;
}
static void * extend_heap(size_t words){
	size_t size;
	char* bp=NULL;
	size=(words%2) ? (words+1)*WSIZE : words*WSIZE;
	if((long)(bp=mem_sbrk(size))==-1){
		return NULL;
	}
	
	/* Initialize free block header/footer and the epilogue header*/
	PUT(HDRP(bp),PACK(size,0)); /* Free block Header */
	PUT(FTRP(bp),PACK(size,0)); /* Free block footer */
	PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); /* New epilogue header*/
	
	/* Coalesce if previous block is free */
#if DEBUG
    printf("--extend_heap--\n");
#endif
	return coalesce(bp);

}
/*
 *Policy: First fit
 */
static void * find_fit(size_t size){
	char* tmp_head_listp=heap_listp;
	size_t asize;
	size_t alloc;
	while((asize=GET_SIZE(HDRP(NEXT_BLKP(tmp_head_listp))))!=0){
		if((alloc=GET_ALLOC(HDRP(NEXT_BLKP(tmp_head_listp))))){
			tmp_head_listp=NEXT_BLKP(tmp_head_listp);
			continue; /* Skip alloc block*/
		}

		/* Find a free block*/
		if(asize>=size){ /* Find a fit*/
			return NEXT_BLKP(tmp_head_listp);
		}else{
		    tmp_head_listp=NEXT_BLKP(tmp_head_listp);
        }
	}
	return NULL;
}

/*
 *place resuest clock and split free block.
 */
static void place(void* bp,size_t size){
	size_t asize=GET_SIZE(HDRP(bp));
	
    if(asize - size < DSIZE){
	    PUT(HDRP(bp),PACK(asize,1));
	    PUT(FTRP(bp),PACK(asize,1));
    }else{ /* split*/
	    PUT(HDRP(bp),PACK(size,1));
	    PUT(FTRP(bp),PACK(size,1));
		
        PUT(HDRP(NEXT_BLKP(bp)),PACK(asize-size,0));
		PUT(FTRP(NEXT_BLKP(bp)),PACK(asize-size,0));
	}
		
}

void debug(char* message){
	size_t asize;
	size_t alloc;
	char* tmp_head_listp=heap_listp;
	printf("\n-- Debug : %s --\n",message);
	while((asize=GET_SIZE(HDRP(tmp_head_listp)))!=0){
		alloc=GET_ALLOC(HDRP(tmp_head_listp));
		printf("%p: ",tmp_head_listp);
		printf("|%6ld|%ld|--PayLoad And Padding--|%6d|%d|--> \n",
				asize,alloc,GET_SIZE(FTRP(tmp_head_listp)),
				GET_ALLOC(FTRP(tmp_head_listp)));
		tmp_head_listp=NEXT_BLKP(tmp_head_listp);
	}
}



/*
 *Init. the start block( a word)
 *	the prologue block(header and footer)
 *	the epilogue block(only header)
 */
int mm_init(void){
	if((heap_listp=mem_sbrk(4*WSIZE))==(void*)-1){
		return -1;
	}
	PUT(heap_listp,0);	/* start block - Alignment padding */
	PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1)); /* Prologue header*/
	PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1)); /* Prologue footer*/
	PUT(heap_listp+(3*WSIZE),PACK(0,1)); /* Epilogue header*/
	
	heap_listp+=(2*WSIZE);
	
	if(extend_heap(CHUNKSZE/WSIZE)==NULL)
		return -1;
#if (DEBUG & DEBUG_INIT)
	debug("init");
#endif
	return 0;
		
}
void mm_free(void* bp){
	size_t size=GET_SIZE(HDRP(bp));
	PUT(HDRP(bp),PACK(size,0));
	PUT(FTRP(bp),PACK(size,0));
	
	coalesce(bp);
#if (DEBUG & DEBUG_FREE)
	debug("free");
#endif
}

void* mm_malloc(size_t size){
	size_t asize;
	char* bp=NULL;
	size_t extend_size;

	if(size<=0)
		return NULL;
	if(size <=DSIZE)
		asize=2*DSIZE;
	else
		asize=DSIZE *((size + (DSIZE) + (DSIZE-1))/DSIZE);
	/* Search the free list for a fit */
	if((bp=find_fit(asize))!=NULL){
		place(bp,asize); /* Use Placement policy and 
				  split policy*/
#if (DEBUG & DEBUG_MALLOC)
		debug("malloc");
#endif
		return bp;
	}
	/* No fit found. Get more memory and place the block*/
	extend_size=MAX(asize,CHUNKSZE);
	if((bp=extend_heap(extend_size/WSIZE))==NULL){
		return NULL;
	}
	place(bp,asize);

#if (DEBUG & DEBUG_MALLOC)
		debug("malloc");
#endif
	return bp;

}
