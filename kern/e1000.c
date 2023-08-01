#include <kern/e1000.h>
#include <kern/pmap.h>
#include <kern/pci.h>
#include <kern/pcireg.h>
#include <inc/x86.h>
#include <inc/mmu.h>
#include <inc/string.h>

volatile void* e1000;
#define E1000_REG(offset)    (*(volatile uint32_t *)(e1000 + offset))

#define TX_BUF_SIZE 1536  // 16-byte aligned for performance
#define NTXDESC     64

static struct e1000_tx_desc e1000_tx_queue[NTXDESC] __attribute__((aligned(16)));
static uint8_t e1000_tx_buf[NTXDESC][TX_BUF_SIZE];

static void
e1000_tx_init()
{
    // initialize tx queue
    int i;
    memset(e1000_tx_queue, 0, sizeof(e1000_tx_queue));
    for (i = 0; i < NTXDESC; i++) {
        e1000_tx_queue[i].addr = PADDR(e1000_tx_buf[i]);
        e1000_tx_queue[i].status = E1000_TXD_STAT_DD;
        e1000_tx_queue[i].cmd = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    }

    // initialize transmit descriptor registers
    E1000_REG(E1000_TDBAL) = PADDR(e1000_tx_queue);
    E1000_REG(E1000_TDBAH) = 0;
    E1000_REG(E1000_TDLEN) = sizeof(e1000_tx_queue);
    E1000_REG(E1000_TDH) = 0;
    E1000_REG(E1000_TDT) = 0;

    // initialize transmit control registers
    E1000_REG(E1000_TCTL) &= ~(E1000_TCTL_CT | E1000_TCTL_COLD);
    E1000_REG(E1000_TCTL) |= E1000_TCTL_EN | E1000_TCTL_PSP |
                            (E1000_COLLISION_THRESHOLD << E1000_CT_SHIFT) |
                            (E1000_COLLISION_DISTANCE << E1000_COLD_SHIFT);
    E1000_REG(E1000_TIPG) &= ~(E1000_TIPG_IPGT_MASK | E1000_TIPG_IPGR1_MASK | E1000_TIPG_IPGR2_MASK);
    E1000_REG(E1000_TIPG) |= E1000_DEFAULT_TIPG_IPGT |
                            (E1000_DEFAULT_TIPG_IPGR1 << E1000_TIPG_IPGR1_SHIFT) |
                            (E1000_DEFAULT_TIPG_IPGR2 << E1000_TIPG_IPGR2_SHIFT);
}


int e1000_transmit(const void* buf, size_t size){
int tail = E1000_REG(E1000_TDT);

    if(size > 1518){
        return -1;
    }
struct e1000_tx_desc* tail_desc = &(e1000_tx_queue[tail]);
    if(!(tail_desc->status & E1000_TXD_STAT_DD)){
        // state is not DD
        return -1;
    }
    memcpy(e1000_tx_buf[tail], buf, size);
    tail_desc->length = (uint16_t)size;
    // clear DD
    tail_desc->status &= ~E1000_TXD_STAT_DD;
    // tail_desc->cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    E1000_REG(E1000_TDT) = (tail + 1) % NTXDESC;
    return 0;
    
}

// LAB 6: Your driver code here
int e1000_attach(struct pci_func* pcif){
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("e1000: status 0x%08x\n", E1000_REG(E1000_STATUS));
    // cprintf("device status:[%08x]\n", *(uint32_t *)((uint8_t *)e1000 + E1000_STATUS));
    e1000_tx_init();
    char *str = "hello";
    int r = e1000_transmit(str, 6);
    return 0;
}