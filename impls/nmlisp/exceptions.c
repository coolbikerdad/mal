#include "exceptions.h"
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <gc.h>

jmp_buf *__bufs;
size_t __buf_count = 0;
exception_t __last_exception;

void exception_init() {
	__bufs = (jmp_buf*) GC_malloc(0);
	__last_exception.code = 0;
	__last_exception.msg = (char*) GC_malloc(0);
}

jmp_buf *exception_newbuf() {
	__buf_count++;
	__bufs = (jmp_buf*) 
		     GC_realloc((void*) __bufs, sizeof(jmp_buf) * __buf_count);
	return &__bufs[__buf_count-1];
}

void exception_endbuf() {
	__buf_count--;
	__bufs = (jmp_buf*) 
		     GC_realloc((void*) __bufs, sizeof(jmp_buf) * __buf_count);
}

void throw_exception(const char *msg, node *value, int code) {
	__last_exception.code = code;
	if (!__buf_count) {
		fprintf(stderr, "Uncaught exception code %d, message: %s\n", 
				code, msg);
		exception_end();
		raise(SIGSEGV);
		/* NOTREACHED */
	}
	GC_free((void*) __last_exception.msg);
	__last_exception.msg = (char*) 
						   GC_malloc( sizeof(char)*(strlen(msg)+1) );
	strcpy(__last_exception.msg, msg);
	__last_exception.value = value;
	longjmp(__bufs[__buf_count-1], code);
}

const exception_t exception_get_last(){
	return __last_exception;
}

void exception_end() {
	__buf_count = 0;
	__last_exception.code = 0;
	GC_free((void*) __last_exception.msg);
	GC_free((void*) __bufs);
}