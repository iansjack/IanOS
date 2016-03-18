/*
 * elffunctions.h
 *
 *  Created on: Mar 2, 2016
 *      Author: ian
 */
#include <reent.h>

#ifndef ELFFUNCTIONS_H_
#define ELFFUNCTIONS_H_

struct library
{
	char *name;
	l_Address base;
	unsigned long size;
	Elf64_Sym *symbols;
	char *symbolnames;
//	struct _reent *_impure_ptr;
	struct library *next;
};

int DoRelocation(struct library *, char*, Elf64_Rela, l_Address);
struct library *GetLib(char *libName);

#endif /* ELFFUNCTIONS_H_ */
