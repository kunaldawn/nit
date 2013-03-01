/*
	see copyright notice in squirrel.h
*/
#include "sqpcheader.h"

sq_user_malloc_func sq_user_malloc = 0;
sq_user_realloc_func sq_user_realloc = 0;
sq_user_free_func sq_user_free = 0;

void *sq_vm_malloc(SQUnsignedInteger size){	return sq_user_malloc(size); }

void *sq_vm_realloc(void *p, SQUnsignedInteger oldSize, SQUnsignedInteger size){ return sq_user_realloc(p, oldSize, size); }

void sq_vm_free(void *p, SQUnsignedInteger size){ sq_user_free(p, size); }
