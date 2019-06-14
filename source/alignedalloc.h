#ifndef ALIGNEDALLOC_H
#define ALIGNEDALLOC_H

void *malloc_aligned(int alignment, int size);

void free_aligned(void *addr);

#endif