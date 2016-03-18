/*
 * mp.h
 *
 *  Created on: Feb 3, 2016
 *      Author: ian
 */

#ifndef MP_H_
#define MP_H_

#include <stddef.h>
#include <stdint.h>

struct mp
{             // floating pointer [MP 4.1]
	uint8_t signature[4];           // "_MP_"
	uint32_t mpconfigtable;            	// phys addr of MP config table
	uint8_t length;                 // 1
	uint8_t specrev;                // [14]
	uint8_t checksum;               // all bytes must add up to 0
	uint8_t type;                   // MP system config type
	uint8_t imcrp;
	uint8_t reserved[3];
}__attribute__((__packed__));

struct mpconf
{         // configuration table header [MP 4.2]
	uint8_t signature[4];           // "PCMP"
	uint16_t length;                // total table length
	uint8_t version;                // [14]
	uint8_t checksum;               // all bytes must add up to 0
	uint8_t product[20];            // product id
	uint32_t oemtable;            // OEM table pointer
	uint16_t oemlength;             // OEM table length
	uint16_t entry;                 // entry count
	uint32_t lapicaddr;           // address of local APIC
	uint16_t xlength;               // extended table length
	uint8_t xchecksum;              // extended table checksum
	uint8_t reserved;
	uint8_t entries[0];             // table entries
}__attribute__((__packed__));

struct mpproc
{         // processor table entry [MP 4.3.1]
	uint8_t type;                   // entry type (0)
	uint8_t apicid;                 // local APIC id
	uint8_t version;                // local APIC version
	uint8_t flags;                  // CPU flags
	uint8_t signature[4];           // CPU signature
	uint32_t feature;               // feature flags from CPUID instruction
	uint8_t reserved[8];
}__attribute__((__packed__));

// mpproc flags
#define MPPROC_BOOT 0x02                // This mpproc is the bootstrap processor

// Table entry types
#define MPPROC    0x00  // One per processor
#define MPBUS     0x01  // One per bus
#define MPIOAPIC  0x02  // One per I/O APIC
#define MPIOINTR  0x03  // One per bus interrupt source
#define MPLINTR   0x04  // One per system interrupt source
#define NCPU 	8

// Per-CPU state
struct CpuInfo
{
	uint8_t cpu_id;                 // Local APIC ID; index into cpus[] below
	volatile unsigned cpu_status;   // The status of the CPU
//	struct Env *cpu_env;            // The currently-running environment.
//	struct Taskstate cpu_ts;        // Used by x86 to find stack for interrupt
};

struct mp *mpsearch(void);

#endif /* MP_H_ */
