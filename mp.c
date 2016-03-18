/*
 * mp.c
 *
 *  Created on: Feb 3, 2016
 *      Author: ian
 */

#include <mp.h>
#include <sys/io.h>

struct CpuInfo cpus[NCPU];
struct CpuInfo *bootcpu;
int ismp;
extern int ncpu;
extern uint64_t lapicaddr;

// Per-CPU kernel stacks
// unsigned char percpu_kstacks[NCPU][KSTKSIZE]
// __attribute__ ((aligned(PGSIZE)));

static uint8_t sum(void *addr, int len)
{
	int i, sum;

	sum = 0;
	for (i = 0; i < len; i++)
		sum += ((uint8_t *) addr)[i];
	return sum;
}

// Find an MP table. We could be clever, but why bother?
// Just search all of memory between 0x0 and 0x100000 for the table
static struct mp *mpsearch1(unsigned char *a, int len)
{
	struct mp *mp = (struct mp*) a, *end = (struct mp *) (a + len);

	for (; mp < end; mp++)
		if (strncmp(mp->signature, "_MP_", 4) == 0 && sum(mp, sizeof(*mp)) == 0)
			return mp;
	return 0;
}

// Search for an MP configuration table.
static struct mpconf * mpconfig(struct mp **pmp)
{
	struct mpconf *conf;
	struct mp *mp;

	if ((mp = mpsearch1(0, 0x100000)) == 0)
		return 0;

	if (mp->mpconfigtable == 0 || mp->type != 0)
	{
		//cprintf("SMP: Default configurations not implemented\n");
		return NULL ;
	}
	conf = (struct mpconf *) ((long) (mp->mpconfigtable));
	if (strncmp((char *) conf, "PCMP", 4) != 0)
	{
		//cprintf("SMP: Incorrect MP configuration table signature\n");
		return NULL ;
	}
	if (sum(conf, conf->length) != 0)
	{
		//cprintf("SMP: Bad MP configuration checksum\n");
		return NULL ;
	}
	if (conf->version != 1 && conf->version != 4)
	{
		//cprintf("SMP: Unsupported MP version %d\n", conf->version);
		return NULL ;
	}
	if ((sum((uint8_t *) conf + conf->length, conf->xlength) + conf->xchecksum)
			& 0xff)
	{
		//cprintf("SMP: Bad MP configuration extended checksum\n");
		return NULL ;
	}
	*pmp = mp;
	return conf;
}

void mp_init(void)
{
	struct mp *mp;
	struct mpconf *conf;
	struct mpproc *proc;
	uint8_t *p;
	unsigned int i;

	ncpu = 0;
	bootcpu = &cpus[0];
	if ((conf = mpconfig(&mp)) == 0)
		return;
	ismp = 1;
	lapicaddr = (uint64_t) (conf->lapicaddr);

	for (p = conf->entries, i = 0; i < conf->entry; i++)
	{
		switch (*p)
		{
		case MPPROC:
			proc = (struct mpproc *) p;
			if (proc->flags & MPPROC_BOOT)
				bootcpu = &cpus[ncpu];
			if (ncpu < NCPU)
			{
				cpus[ncpu].cpu_id = ncpu;
				ncpu++;
			}
			p += sizeof(struct mpproc);
			continue;
		case MPBUS:
		case MPIOAPIC:
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;
		default:
			ismp = 0;
			i = conf->entry;
		}
	}

	bootcpu->cpu_status = 1 /*CPU_STARTED*/;
	if (!ismp)
	{
		// Didn't like what we found; fall back to no MP.
		ncpu = 1;
		lapicaddr = 0;
		return;
	}

	if (mp->imcrp)
	{
		// [MP 3.2.6.1] If the hardware implements PIC mode,
		// switch to getting interrupts from the LAPIC.
		//cprintf("SMP: Setting IMCR to switch from PIC mode to symmetric I/O mode\n");
		outb(0x22, 0x70);   // Select IMCR
		outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
	}
}

