#ifndef _MM_H_
#define _MM_H_
#include <stddef.h>
extern int mm_init(void);
extern void* mm_malloc(size_t size);
extern void mm_free(void * ptr);

#endif
