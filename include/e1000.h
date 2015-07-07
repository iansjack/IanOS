#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

void queue_packet(void *packet, int length);
int receive_packet(void *buffer, int *length);

volatile void *e1000;

#define CTRL     0x00000  /* Device Control - RW */
#define CTRL_DUP 0x00004  /* Device Control Duplicate (Shadow) - RW */
#define STATUS   0x00008  /* Device Status - RO */

#define TCTL     0x00400  /* TX Control - RW */
#define TIPG     0x00410  /* TX Inter-packet gap -RW */


#define TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define TDLEN    0x03808  /* TX Descriptor Length - RW */
#define TDH      0x03810  /* TX Descriptor Head - RW */
#define TDT      0x03818  /* TX Descriptor Tail - RW */
#define TIDV     0x03820  /* TX Interrupt Delay Value - RW */
#define TXDCTL   0x03828  /* TX Descriptor Control - RW */

#define RCTL     0x00100  /* RX Control - RW */
#define RDBAL    0x02800  /* RX Descriptor Base Address Low - RW */
#define RDBAH    0x02804  /* RX Descriptor Base Address High - RW */
#define RDLEN    0x02808  /* RX Descriptor Length - RW */
#define RDH      0x02810  /* RX Descriptor Head - RW */
#define RDT      0x02818  /* RX Descriptor Tail - RW */
#define RDTR     0x02820  /* RX Delay Timer - RW */
#define RAL      0x05400  /* Receive Address Low */
#define RAH		 0x05404  /* Receive Address High */

#define ICR		0x000C0 /* Interrupt Cause Read */
#define IMS		0x000D0	/* Interrupt Mask Set/Read */
#define IMC		0x000D8 /* Interrupt Mask Clear */

/* Transmit Control */
#define TCTL_RST    0x00000001    /* software reset */
#define TCTL_EN     0x00000002    /* enable tx */
#define TCTL_BCE    0x00000004    /* busy check enable */
#define TCTL_PSP    0x00000008    /* pad short packets */
#define TCTL_CT     0x00000ff0    /* collision threshold */
#define TCTL_COLD   0x003ff000    /* collision distance */
#define TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define TCTL_MULR   0x10000000    /* Multiple request support */

/* Receive Control */
#define RCTL_RST            0x00000001    /* Software reset */
#define RCTL_EN             0x00000002    /* enable */
#define RCTL_SBP            0x00000004    /* store bad packet */
#define RCTL_UPE            0x00000008    /* unicast promiscuous enable */
#define RCTL_MPE            0x00000010    /* multicast promiscuous enab */
#define RCTL_LPE            0x00000020    /* long packet enable */
#define RCTL_LBM_NO         0x00000000    /* no loopback mode */
#define RCTL_LBM_MAC        0x00000040    /* MAC loopback mode */
#define RCTL_LBM_SLP        0x00000080    /* serial link loopback mode */
#define RCTL_LBM_TCVR       0x000000C0    /* tcvr loopback mode */
#define RCTL_DTYP_MASK      0x00000C00    /* Descriptor type mask */
#define RCTL_DTYP_PS        0x00000400    /* Packet Split descriptor */
#define RCTL_RDMTS_HALF     0x00000000    /* rx desc min threshold size */
#define RCTL_RDMTS_QUAT     0x00000100    /* rx desc min threshold size */
#define RCTL_RDMTS_EIGTH    0x00000200    /* rx desc min threshold size */
#define RCTL_MO_SHIFT       12            /* multicast offset shift */
#define RCTL_MO_0           0x00000000    /* multicast offset 11:0 */
#define RCTL_MO_1           0x00001000    /* multicast offset 12:1 */
#define RCTL_MO_2           0x00002000    /* multicast offset 13:2 */
#define RCTL_MO_3           0x00003000    /* multicast offset 15:4 */
#define RCTL_MDR            0x00004000    /* multicast desc ring 0 */
#define RCTL_BAM            0x00008000    /* broadcast enable */
/* these buffer sizes are valid if RCTL_BSEX is 0 */
#define RCTL_SZ_2048        0x00000000    /* rx buffer size 2048 */
#define RCTL_SZ_1024        0x00010000    /* rx buffer size 1024 */
#define RCTL_SZ_512         0x00020000    /* rx buffer size 512 */
#define RCTL_SZ_256         0x00030000    /* rx buffer size 256 */
/* these buffer sizes are valid if RCTL_BSEX is 1 */
#define RCTL_SZ_16384       0x00010000    /* rx buffer size 16384 */
#define RCTL_SZ_8192        0x00020000    /* rx buffer size 8192 */
#define RCTL_SZ_4096        0x00030000    /* rx buffer size 4096 */
#define RCTL_VFE            0x00040000    /* vlan filter enable */
#define RCTL_CFIEN          0x00080000    /* canonical form enable */
#define RCTL_CFI            0x00100000    /* canonical form indicator */
#define RCTL_DPF            0x00400000    /* discard pause frames */
#define RCTL_PMCF           0x00800000    /* pass MAC control frames */
#define RCTL_BSEX           0x02000000    /* Buffer size extension */
#define RCTL_SECRC          0x04000000    /* Strip Ethernet CRC */
#define RCTL_FLXBUF_MASK    0x78000000    /* Flexible buffer size */
#define RCTL_FLXBUF_SHIFT   27            /* Flexible buffer shift */

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

struct rx_desc {
    uint64_t buffer_addr; /* Address of the descriptor's data buffer */
    uint16_t length;     /* Length of data DMAed into data buffer */
    uint16_t csum;       /* Packet checksum */
    uint8_t status;      /* Descriptor status */
    uint8_t errors;      /* Descriptor Errors */
    uint16_t special;
};
#endif	// JOS_KERN_E1000_H
