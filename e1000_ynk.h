#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/capability.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/pkt_sched.h>
#include <linux/list.h>
#include <linux/reboot.h>
#include <net/checksum.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>

#define INTEL_E1000_ETHERNET_DEVICE(device_id) {\
	PCI_DEVICE(PCI_VENDOR_ID_INTEL, device_id)}

#define E1000_EECD 0x00010
#define E1000_CTRL 0x00000
#define E1000_EERD 0x00014
#define E1000_EERD_START 0x00000001
#define E1000_EERD_DONE 0x00000010

char e1000_ynk_driver_name[] = "YNK E1000 driver";
static int e1000_ynk_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void e1000_ynk_remove(struct pci_dev *dev);
int e1000_ynk_open(struct net_device *net_dev);
u8* read_mac_from_eeprom(u8 __iomem* bar0 , u8 *mac_adrr);
void reset_hardware( u8 __iomem* bar0);
struct e1000_ynk_adapter
{
	 u8 __iomem* bar0;
};
