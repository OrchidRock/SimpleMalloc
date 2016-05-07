#ifndef _MEMLIB_H_
#define _MEMLIB_H_

#define MAX_HEAP (((long)0x1)<<32)

extern void mem_init(void);
extern void* mem_sbrk(int);

#endif
