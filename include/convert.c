#include <stddef.h>
#include "kstructs.h"

#define _ASMDEFINE(sym, val) asm volatile \
	("\n-> " #sym " %0 \n" : : "i" (val))
#define ASMDEFINE(s, m) \
	_ASMDEFINE(s.m, offsetof(s, m));

typedef  struct Task TS;

void myStruct_defineOffsets() 
{
	ASMDEFINE(TS, rax);
	ASMDEFINE(TS, rbx);
	ASMDEFINE(TS, rcx);
	ASMDEFINE(TS, rdx);
	ASMDEFINE(TS, rbp);
	ASMDEFINE(TS, rsi);
	ASMDEFINE(TS, rdi);
	ASMDEFINE(TS, rsp);
	ASMDEFINE(TS, r8);
	ASMDEFINE(TS, r9);
	ASMDEFINE(TS, r10);
	ASMDEFINE(TS, r11);
	ASMDEFINE(TS, r12);
	ASMDEFINE(TS, r13);
	ASMDEFINE(TS, r14);
	ASMDEFINE(TS, r15);
	ASMDEFINE(TS, rflags);
	ASMDEFINE(TS, cr3);
	ASMDEFINE(TS, firstdata);
	ASMDEFINE(TS, firstfreemem);
	ASMDEFINE(TS, timer);
	ASMDEFINE(TS, environment);
	ASMDEFINE(TS, parentPort);
	ASMDEFINE(TS, currentDirName);
	ASMDEFINE(TS, console);
	ASMDEFINE(TS, fcbList);
	ASMDEFINE(TS, FDbitmap);
	ASMDEFINE(TS, pid);
	ASMDEFINE(TS, ds);
	ASMDEFINE(TS, es);
	ASMDEFINE(TS, fs);
	ASMDEFINE(TS, gs);
	ASMDEFINE(TS, ss);
	ASMDEFINE(TS, forking);
	ASMDEFINE(TS, waiting);
	ASMDEFINE(TS, fcb)
}

