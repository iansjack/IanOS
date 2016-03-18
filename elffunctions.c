#include <linux/types.h>
#include <kernel.h>
#include <filesystem.h>
#include <errno.h>
#include <elf.h>
#include <blocks.h>
#include <pagetab.h>
#include <elffunctions.h>

extern struct MessagePort *FSPort;
struct library *libs = 0;
l_Address base = 0x000008100000000L;

struct library *GetLib(char *libName)
{
	struct library *currentLib = libs;
	struct library *lastLib = libs;
	struct library *lib = 0;

	// Is the library list empty? If so create new entry
	if (!libs)
	{
		lib = AllocKMem(sizeof(struct library));
		lib->name = AllocKMem(strlen(libName));
		strcpy(lib->name, libName);
		lib->base = base;
		lib->next = 0;
		libs = lib;
	}
	else
	{
		// Is the library already in the list
		currentLib = libs;
		while (currentLib)
		{
			if ((!strcmp(currentLib->name, libName)))
			{
				lib = currentLib;
				return lib;
			}
			lastLib = currentLib;
			currentLib = currentLib->next;
		}
	}

	if (!lib)
	{
		lib = AllocKMem(sizeof(struct library));
		lib->name = AllocKMem(strlen(libName));
		strcpy(lib->name, libName);
		lib->base = base;
		lib->next = 0;
		lastLib->next = lib;
	}

	LoadLibrary(lib);
	return lib;
}

ReadElf(struct Message * FSMsg, struct FCB *fHandle, struct library *lib,
		long *entrypoint, long pid)
{
	Elf64_Ehdr header;
	long loadlocation;
	l_Address base2;

	if (lib)
		base2 = lib->base;
	else
		base2 = 0;

	// Read Elf header
	ReadFromFile(fHandle, (char *) &header, (long) sizeof(Elf64_Ehdr));

	// Read Program Header Table
	SeekFile(FSMsg, fHandle, (long) header.e_phoff, SEEK_SET);
	int pheadersize = header.e_phnum * header.e_phentsize;
	Elf64_Phdr *pheadertable = AllocKMem(pheadersize);
	ReadFromFile(fHandle, (char *) pheadertable, (long) pheadersize);
	int i;

	int baseinc = 0;
	Elf64_Dyn *dynamic;
	int dynamicsize;
	// Load the loadable segments into memory
	for (i = 0; i < header.e_phnum; i++)
	{
		switch (pheadertable[i].p_type)
		{
		case PT_LOAD:
		{
			AllocateRange(base2 + pheadertable[i].p_vaddr,
					pheadertable[i].p_memsz, pid);
			loadlocation = (long) pheadertable[i].p_offset;
			SeekFile(FSMsg, fHandle, loadlocation, SEEK_SET);
			ReadFromFile(fHandle, (char *) (base2 + pheadertable[i].p_vaddr),
					(long) (pheadertable[i].p_filesz));
			baseinc = pheadertable[i].p_vaddr + pheadertable[i].p_memsz;
			break;
		}
		case PT_DYNAMIC:
		{
			dynamic = (Elf64_Dyn *) (base2 + pheadertable[i].p_vaddr);
			dynamicsize = pheadertable[i].p_filesz / sizeof(Elf64_Dyn);
			break;
		}
		}
	}

	// Find the sections information
	int sectionsize = header.e_shnum * header.e_shentsize;
	Elf64_Shdr *sections = (Elf64_Shdr *) AllocKMem(sectionsize);
	SeekFile(FSMsg, fHandle, header.e_shoff, SEEK_SET);
	ReadFromFile(fHandle, (char *) sections, sectionsize);

	// Find the section names string table
	Elf64_Shdr stringsection = sections[header.e_shstrndx];
	char *sectionnames = AllocKMem(stringsection.sh_size);
	SeekFile(FSMsg, fHandle, stringsection.sh_offset, SEEK_SET);
	ReadFromFile(fHandle, sectionnames, stringsection.sh_size);

	l_Address dynstrtab = 0;
	Elf64_Sym *dynsymtab = 0;
	l_Address bssptr = 0;
	int bsssize = 0;

	// Find the sections of interest for relocation
	Elf64_Rela *reladyn = 0;
	int reladynsize = 0;
	Elf64_Rela *relaplt = 0;
	int relapltsize = 0;

	for (i = 0; i < header.e_shnum; i++)
	{
		if (!strcmp(sections[i].sh_name + sectionnames, ".rela.dyn"))
		{
			reladyn = (Elf64_Rela *) (base2 + sections[i].sh_addr);
			reladynsize = sections[i].sh_size / sizeof(Elf64_Rela);
		}
		if (!strcmp(sections[i].sh_name + sectionnames, ".rela.plt"))
		{
			relaplt = (Elf64_Rela *) (base2 + sections[i].sh_addr);
			relapltsize = sections[i].sh_size / sizeof(Elf64_Rela);
		}
		if (!strcmp(sections[i].sh_name + sectionnames, ".dynsym"))
			dynsymtab = (Elf64_Sym *) (base2 + sections[i].sh_addr);
		if (!strcmp(sections[i].sh_name + sectionnames, ".dynstr"))
			dynstrtab = base2 + sections[i].sh_addr;
		if (!strcmp(sections[i].sh_name + sectionnames, ".bss"))
		{
			bssptr = base2 + sections[i].sh_addr;
			bsssize = sections[i].sh_size;
		}
	}

	// zero .bss
	while (bsssize)
	{
		*((char *)bssptr) = 0;
		bssptr++;
		bsssize--;
	}

	if (lib)
	{
		lib->size = baseinc;
		lib->symbols = dynsymtab;
		lib->symbolnames = (char *)dynstrtab;
//		lib->_impure_ptr = 0x8100261d20;
	}

	if (dynamicsize)
	{
		int numberneeded = 0;
		for (i = 0; i < dynamicsize; i++)
			if (dynamic[i].d_tag == DT_NEEDED)
				numberneeded++;

		struct neededlibs
		{
			struct library *lib;
			int name;
		};

		struct neededlibs *needed = AllocKMem(
				(numberneeded + 1) * sizeof(struct neededlibs));

		int j = 0;
		for (i = 0; i < dynamicsize; i++)
			if (dynamic[i].d_tag == DT_NEEDED)
			{
				needed[j].lib = GetLib(((char *)(dynstrtab + dynamic[i].d_un.d_val)));
				needed[j].name = dynamic[i].d_un.d_val;
				j++;
			}
		needed[j].lib = lib;
		needed[j].name = 1000000;

		// Do the data relocations
		for (i = 0; i < reladynsize; i++)
		{
			switch (ELF64_R_TYPE (reladyn[i].r_info))
			{
			case R_386_GLOB_DAT:
			{
				long * address = (long *) (base2 + reladyn[i].r_offset);
				long value = (long) (base2
						+ dynsymtab[ELF64_R_SYM(reladyn[i].r_info)].st_value);
				*address = value;
				break;
			}
			case R_386_RELATIVE:
			{
				long * address = (long *) (base2 + reladyn[i].r_offset);
				*address = (long) (base2 + reladyn[i].r_addend);
				break;
			}
			case R_386_32:
			{
				long * address = (long *) (base2 + reladyn[i].r_offset);
				long value = (long) (base2
						+ dynsymtab[ELF64_R_SYM(reladyn[i].r_info)].st_value
						+ reladyn[i].r_addend);
				*address = value;
				break;
			}
			case R_386_COPY:
			{
				long name = dynsymtab[ELF64_R_SYM(reladyn[i].r_info)].st_name;
				Elf64_Rela rela = reladyn[i];
				for (j = 0; 1; j++)
					if (name < needed[j + 1].name || name < needed[0].name)
						break;
				struct library *lib = needed[j].lib;
				name = (long) (dynstrtab + name);
				int i;
				for (i = 0; 1; i++)
				{
					if (!strcmp(lib->symbolnames + lib->symbols[i].st_name,
							(char *) name))
						break;
				}
				long * address = (long *) (base2 + rela.r_offset);
				*address = *(long *) (lib->base + lib->symbols[i].st_value);
				break;
			}
			default:
				asm("jmp .");
			}
		}
		// Now do the procedure relocations
		for (i = 0; i < relapltsize; i++)
		{
			int name = dynsymtab[ELF64_R_SYM(relaplt[i].r_info)].st_name;
			for (j = 0; 1; j++)
				if (name < needed[j + 1].name || name < needed[0].name)
					break;
			DoRelocation(needed[j].lib, (char *)(dynstrtab + name), relaplt[i],
					(long) base2);
		}
	}

	if (lib)
		base = PAGE((long) (base + baseinc + PageSize));

	*entrypoint = header.e_entry;
	DeallocMem(pheadertable);
	DeallocMem(sections);
	DeallocMem(sectionnames);
}

LoadLibrary(struct library *lib)
{
	struct FCB *fHandle;
	long argv, argc;
	int retval = -ENOEXEC;
	struct Message *FSMsg = ALLOCMSG;

	char *kname = AllocKMem((size_t) strlen(lib->name) + 6);

	strcpy(kname, "/lib/");
	strcat(kname, lib->name);

	// Open file
	FSMsg->nextMessage = 0;
	FSMsg->byte = OPENFILE;
	FSMsg->quad1 = (long) kname;
	FSMsg->quad2 = (long) fHandle;
	SendReceiveMessage(FSPort, FSMsg);

	fHandle = (struct FCB *) FSMsg->quad1;

	DeallocMem(kname);

	ReadElf(FSMsg, fHandle, lib, 0, 0);
}

DoRelocation(struct library *lib, char *name, Elf64_Rela rela, l_Address base)
{
	char *symname;
	int i;
	for (i = 0; 1; i++)
	{
		symname = lib->symbolnames + lib->symbols[i].st_name;
		if (!strcmp(symname, name))
			break;
	}
	long * address = (long *) (base + rela.r_offset);
	*address = (long) (lib->base + lib->symbols[i].st_value);
}
