#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include <kern/pci.h>

#define E1000_VEN_ID            0x8086
#define E1000_DEV_ID_82540EM    0x100E

#define E1000_STATUS   0x00008  /* Device Status - RO */

#define E1000_TCTL     0x00400  /* TX Control - RW */
#define E1000_TIPG     0x00410  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    0x03800  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    0x03804  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    0x03808  /* TX Descriptor Length - RW */
#define E1000_TDH      0x03810  /* TX Descriptor Head - RW */
#define E1000_TDT      0x03818  /* TX Descripotr Tail - RW */

/* Transmit Control */
#define E1000_TCTL_EN   0x00000002  /* enable tx */
#define E1000_TCTL_PSP  0x00000008  /* pad short packets */
#define E1000_TCTL_CT   0x00000ff0  /* collision threshold */
#define E1000_TCTL_COLD 0x003ff000  /* collision distance */

/* Collision related configuration parameters */
#define E1000_COLLISION_THRESHOLD   0x10
#define E1000_CT_SHIFT              4

/* Collision distance is a 0-based value that applies to half-duplex-capable hardware only. */
#define E1000_COLLISION_DISTANCE    0x40
#define E1000_COLD_SHIFT            12

/* Default values for the transmit IPG register */
#define E1000_DEFAULT_TIPG_IPGT     10
#define E1000_DEFAULT_TIPG_IPGR1    4
#define E1000_DEFAULT_TIPG_IPGR2    6
#define E1000_TIPG_IPGT_MASK        0x000003FF
#define E1000_TIPG_IPGR1_MASK       0x000FFC00
#define E1000_TIPG_IPGR2_MASK       0x3FF00000
#define E1000_TIPG_IPGR1_SHIFT      10
#define E1000_TIPG_IPGR2_SHIFT      20

/* Transmit Descriptor bit definitions */
#define E1000_TXD_CMD_EOP   0x01  /* End of Packet */
#define E1000_TXD_CMD_RS    0x08  /* Report Status */
#define E1000_TXD_STAT_DD   0x01  /* Descriptor Done */

struct e1000_tx_desc{
    uint64_t addr; 
    uint16_t length; 
    uint8_t cso; 
    uint8_t cmd; 
    uint8_t status; 
    uint8_t css; 
    uint16_t special;
}__attribute__((packed));

int e1000_attach(struct pci_func* pcif);
int e1000_transmit(const void* buf, size_t size);
#endif  // SOL >= 6
