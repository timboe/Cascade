#include <stdlib.h>
#include <stdarg.h>
#include "pdxalloc.h"

PlaydateAPI* pd = NULL;

static void (*pdxlogf)(const char*, ...);

void setPDPtr(PlaydateAPI* p) {
	pd = p;
	pdxlogf = p->system->logToConsole;
}

void* pdxalloc(size_t count, size_t size) {
	void* retval = pd->system->realloc(NULL, count * size);
	memset(retval, 0, size * count);
	return retval;
}

void pdxfree(void* ptr) {
	if (ptr) pd->system->realloc(ptr, 0);
}

void* pdxrealloc(void* ptr, size_t size) {
	return pd->system->realloc(ptr, size);
}

void pdxlog(char* msg) {
	pd->system->logToConsole(msg);
}
