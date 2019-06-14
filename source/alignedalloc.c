#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include "alignedalloc.h"

//The Functions Below are taken from
//https://stackoverflow.com/questions/1919183/how-to-allocate-and-free-aligned-memory-in-c

void *malloc_aligned(int alignment, int size)
{
    void *mem = malloc(size+alignment+sizeof(void*));
	if(mem == NULL)
	{
		return NULL;
	}
    void **ptr = (void**)((uintptr_t)(mem+alignment+sizeof(void*)) & ~(alignment-1));
    ptr[-1] = mem;
    return ptr;
}

void free_aligned(void *ptr)
{
	if(ptr != NULL)
	{
		free(((void**)ptr)[-1]);
	}
}