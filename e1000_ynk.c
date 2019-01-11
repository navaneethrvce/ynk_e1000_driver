#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/netdevice.h>
#include <linux/io.h>
#include <linux/prefetch.h>
#include <linux/bitops.h>
#include "e1000_ynk.h"
static const struct pci_device_id desig_init_pci_device_id_tbl[] =
{
	INTEL_E1000_ETHERNET_DEVICE(0x100F),
	{0,}
};

static struct pci_driver e1000_ynk_driver=
{
	.name = e1000_ynk_driver_name,
	.id_table = desig_init_pci_device_id_tbl,
	.probe = e1000_ynk_probe,
	.remove = e1000_ynk_remove
};

static const struct net_device_ops e1000_ynk_netdev_ops = {
	.ndo_open		= e1000_ynk_open
};
static int e1000_init(void)
{
	printk(KERN_INFO "Initializing E1000 driver\n");
	printk(KERN_INFO "Registering driver with PCI\n");
	int ret = pci_register_driver(&e1000_ynk_driver);
	printk(KERN_INFO "The return value of the driver registration is %d",ret);
	return 0;
}

static void e1000_exit(void)
{
	printk(KERN_INFO "Unloading e1000 driver \n");
	pci_unregister_driver(&e1000_ynk_driver);
}

static int e1000_ynk_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
	int bars,err;
	struct e1000_ynk_adapter *network_adapter;
	printk(KERN_INFO "Starting E1000 driver\n");
	/*Initialize device before it's used by a driver. Ask low-level code to enable Memory resources. 
	Wake up the device if it was suspended. Beware, this function can fail.*/
	printk(KERN_INFO "Initializing Device and enable memory resources\n");
	err = pci_enable_device_mem(dev);
	pci_request_regions(dev, e1000_ynk_driver_name);
	/*For E1000 Drivers BAR0 has MMIO to internal registers and memories*/
	struct net_device *net_dev = alloc_etherdev(sizeof(struct e1000_ynk_adapter));
	network_adapter = netdev_priv(net_dev);
	network_adapter->bar0 = pci_ioremap_bar(dev,0);
	SET_NETDEV_DEV(net_dev, &dev->dev);
	net_dev->netdev_ops = &e1000_ynk_netdev_ops;
	net_dev->watchdog_timeo = 5 * HZ;
	printk(KERN_INFO "MAC address length is %d bytes\n",net_dev->addr_len);
	printk(KERN_INFO "BAR 0 address is %0x",network_adapter->bar0);
	printk(KERN_INFO "%d \n",ioread32((network_adapter->bar0)+0x0008));
	reset_hardware(network_adapter->bar0);
	u8 mac_addr[6];
	read_mac_from_eeprom(network_adapter->bar0,mac_addr);
	memcpy(net_dev->dev_addr, mac_addr, net_dev->addr_len);
	register_netdev(net_dev);
	pci_set_drvdata(dev,net_dev);
	strcpy(net_dev->name,"eth0_ynk");
	return err;
}

void reset_hardware( u8 __iomem* bar0)
{
	pr_info("Reading EECD before global reset");
	int EECD_init_value = ioread32(bar0+E1000_EECD);
	pr_info("EECD value is %d\n",EECD_init_value);
	pr_info("Performing HW  RESET");
	iowrite32(0x04000000,bar0 + E1000_CTRL);
	pr_info("EECD value after reset %d\n",ioread32(bar0+E1000_EECD));
}

u8* read_mac_from_eeprom(u8 __iomem* bar0,u8 *mac_addr)
{

	printk(KERN_INFO "read_mac_from_eeprom");
	pr_info("Reading EECD before write");
	int EECD_init_value = ioread32(bar0+E1000_EECD);
	printk(KERN_INFO "%d \n",EECD_init_value);
	iowrite32(0x00000040 ,bar0+E1000_EECD);
	pr_info("Reading EECD after write");
	int EECD_post_value = ioread32(bar0+E1000_EECD);
	pr_info("EECD post value %d",EECD_post_value & 0x00000080);
	if((EECD_post_value & 0x00000080) == 0x00000080)
	{
		/*Intel EEPROM Structure for MAC address
		----------------------------------------------------------------
			|       BIT 15 - 8          |             BIT 7 - 0
		----------------------------------------------------------------
		00h	|ETHERNET ADDRESS BYTE 2	|	ETHERNET ADDRESS BYTE 1	
		01h	|ETHERNET ADDRESS BYTE 4	|	ETHERNET ADDRESS BYTE 3
		02h	|ETHERNET ADDRESS BYTE 6	|	ETHERNET ADDRESS BYTE 5
		----------------------------------------------------------------
		*/
		int mac_words = 3;
		int mac_byte[3];
		int iter ;
		int mac_addr_iter =0;
		for(iter = 0 ; iter < mac_words ; iter++)
		{
			pr_info("Turning on Read, Writing EEPROM address , Reading data from EEPROM");
			//The reason we are left shifting by 8, because the eeprom address should be written in the 8th byte of EERD
			iowrite32(E1000_EERD_START | (iter<< 8) , (bar0+E1000_EERD));
			int mac_wrd = ioread32(bar0+E1000_EERD)&0xFFFF0000;
			u8 mac_byte_1 = (u8)(mac_wrd>>16)&0xF + ((mac_wrd>>16)&0xF0);
			u8 mac_byte_2 = (u8)(mac_wrd>>24)&0xF + ((mac_wrd>>24)&0xF0);
			pr_info("mac_addr_part %02x",mac_byte_1);
			pr_info("mac_addr_part %02x",mac_byte_2);
			mac_addr[mac_addr_iter] = mac_byte_1;
			mac_addr_iter+=1;
			mac_addr[mac_addr_iter] = mac_byte_2;
			mac_addr_iter+=1;
			pr_info("Turning off Read");
			iowrite32(E1000_EERD_DONE,bar0 + E1000_EERD);
		}
		pr_info("%02x:%02x:%02x:%02x:%02x:%02x",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
		return mac_addr;
	}
}
static void e1000_ynk_remove(struct pci_dev *dev)
{
	printk(KERN_INFO "Shutting Down Ethernet Device");
	pci_release_regions(dev);
	struct net_device *net_dev = pci_get_drvdata(dev);
	unregister_netdev(net_dev);
}


int e1000_ynk_open(struct net_device *net_dev)
{
	printk(KERN_INFO "The ethernet device was asked to open \n");
	net_dev->flags = net_dev->flags | IFF_UP;
	//set_bit(IFF_UP,net_dev->flags);
}


MODULE_DEVICE_TABLE(pci,desig_init_pci_device_id_tbl);
module_init(e1000_init);
module_exit(e1000_exit);
MODULE_LICENSE("GPL v2");

