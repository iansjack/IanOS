#include <stdint.h>
#include <mp.h>

struct transfer
{
	uint64_t nPagesFree;
	uint64_t nPages;
	uint64_t firstFreePage;
	uint64_t virtualPDP;
	uint64_t kernelPT;
};


