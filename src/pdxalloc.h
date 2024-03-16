#ifndef PDXALLOC_H
#define PDXALLOC_H

#include "pd_api.h"

extern PlaydateAPI* pd;

void setPDPtr(PlaydateAPI* p);
void* pdxalloc(size_t count, size_t size);
void pdxfree(void *ptr);
void* pdxrealloc(void *ptr, size_t size);
void pdxlog(char *msg);

#endif // PDXALLOC_H