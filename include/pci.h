#ifndef JOS_KERN_PCI_H
#define JOS_KERN_PCI_H

#include <types.h>

// PCI subsystem interface
enum { pci_res_bus, pci_res_mem, pci_res_io, pci_res_max };

struct pci_bus;

struct pci_func {
    struct pci_bus *bus;	// Primary bus for bridges

    uint_32 dev;
    uint_32 func;

    uint_32 dev_id;
    uint_32 dev_class;

    uint_32 reg_base[6];
    uint_32 reg_size[6];
    uint_8 irq_line;
};

struct pci_bus {
    struct pci_func *parent_bridge;
    uint_32 busno;
};

int  pci_init(void);
void pci_func_enable(struct pci_func *f);

#endif
