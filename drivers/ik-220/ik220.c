/* ik220 - Linux-Driver for Heidenhain IK220-Card
 *
 * Author: DR.JOHANNES HEIDENHAIN GmbH / a14551
 * Created: 09/09/03
 *
 * This is a port of the WINDOWS-driver. 
 *
 * "@(#) $Id$"
 *
 * who      when     what
 * -------- -------- ------------------------------
 * dwischol 09/09/03 created
 */

 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include "ik220.h"

static const char driver_name[] = "IK220";
static const char driver_version[20] = "DRV: V "IK220_DRIVER_VERSION"(R)";

static int number_of_cards = 0;

/* Global device-specific struct */
static struct ik220_device_data {
   struct pci_dev *dev;
   unsigned long conf_iomem_start;
   unsigned long conf_iomem_end;
   unsigned long conf_iomem_flags;
   unsigned long conf_iomem_len;
   u32 *conf_iomem_virtual;
   unsigned long iomem_1_start;
   unsigned long iomem_1_end;
   unsigned long iomem_1_flags;
   unsigned long iomem_1_len;
   u16 *iomem_1_virtual;
   unsigned long iomem_2_start;
   unsigned long iomem_2_end;
   unsigned long iomem_2_flags;
   unsigned long iomem_2_len;
   u16 *iomem_2_virtual;
} ik220_card[IK220_MAX_CARDS];   


static struct pci_device_id ik220_tbl[] __devinitdata = {
        {IK220_VENDOR_ID, IK220_DEVICE_ID, IK220_SUBVENDOR_ID, IK220_SUBDEVICE_ID, 0, 0, 0},
        {0,},
};
MODULE_DEVICE_TABLE(pci,ik220_tbl);

MODULE_AUTHOR ("JH");
MODULE_DESCRIPTION ("Heidenhain IK220 driver");
#ifdef MODULE_LICENSE
   MODULE_LICENSE ("GPL"); /*LOOK AT FILE : Describe.txt"); */
#endif



/************************************************************************************************/
/* Access-Functions                                                                             */
/************************************************************************************************/

static long ik220_read_dataw(int axis, unsigned short address, u16 *data, unsigned int nr_of_words)
{
   u16 *axis_regs = 0;
   int i;

/*FIXME: Check address out of bounds! Remember size!*/
   switch(axis % 2)
   {
      case 0: axis_regs = ik220_card[axis / 2].iomem_1_virtual;
                          break;
      case 1: axis_regs = ik220_card[axis / 2].iomem_2_virtual;
                          break;
   }

   if(axis_regs == NULL) return -EFAULT;

   axis_regs += address;

   for(i = 0; i < nr_of_words; ++i)
   {
      data[i] = readw(axis_regs + i);
   }

   //printk("Hier ist ein 0x%X fuer sie!\n", data[0]);

return 0;
}

static long ik220_write_dataw(int axis, unsigned short address, u16 *data, unsigned int nr_of_words)
{
   u16 *axis_regs = 0;
   int i;

/*FIXME: Check address out of bounds! Remember size!*/
   switch(axis % 2)
   {
      case 0: axis_regs = ik220_card[axis / 2].iomem_1_virtual;
                          break;
      case 1: axis_regs = ik220_card[axis / 2].iomem_2_virtual;
                          break;
   }

   if(axis_regs == NULL) return -EFAULT;

   axis_regs += address;

   for(i = 0; i < nr_of_words; ++i)
   {
      writew(data[i], axis_regs + i);
   }

return 0;
}

static long ik220_install_program(int axis, unsigned short address, u16 *program, int nr_of_words)
{
   unsigned short *axis_regs_base = NULL;
   int i;
  
/*FIXME: Check address out of bounds, maybe in download? Remember progsize!*/
   switch(axis % 2)
   {
      case 0: axis_regs_base = ik220_card[axis / 2].iomem_1_virtual;
                          break;
      case 1: axis_regs_base = ik220_card[axis / 2].iomem_2_virtual;
                          break;
   }

   if(axis_regs_base == NULL) return -EFAULT;

   for(i = 0; i < nr_of_words; ++i)
   {
      writew(IK220_BOOTMODE, axis_regs_base + IK220_CONTROLREGISTER);
      while(!(readw(axis_regs_base + IK220_STATUSREGISTER) & IK220_STATUS_PIPE_EMPTY))
      {
         /*FIXME: TIMEOUT!!!!!!!!!!!!!!!!!!!!!!!! */
         /*FIXME: Do some idle-stuff! Save to call schedule? */
      }   
     
      writew(address + i, axis_regs_base + IK220_DATAREGISTER_0);
      writew(program[i], axis_regs_base + IK220_DATAREGISTER_1);
      writew(IK220_WRITE_RAM_MODE, axis_regs_base + IK220_CONTROLREGISTER);
      
      while(!(readw(axis_regs_base + IK220_STATUSREGISTER) & IK220_STATUS_PIPE_EMPTY))
      {
         /*FIXME: TIMEOUT!!!!!!!!!!!!!!!!!!!!!!!! */
         /*FIXME: Do some idle-stuff! Save to call schedule? */
      }   
   }  
   

return 0;
}





/************************************************************************************************/
/* Implementations of IOCTLs                                                                    */
/************************************************************************************************/
static long ik220_input(unsigned long data_addr)
{
   u16 data[IK220_MAX_INPUT_SIZE];
   int card_nr;
   struct ik220_data data_struct;
   unsigned int nr_of_words;
   
/* FIXME: Add common error-handler */
   if(copy_from_user(&data_struct, (struct ik220_dataram *)data_addr, sizeof(data_struct)))
      return -EFAULT;


   if(((card_nr = (data_struct.axis / 2)) > IK220_MAX_CARDS - 1) ||
      (card_nr < 0) ||
      (ik220_card[card_nr].dev == NULL))
   {
      return -ENODEV;
   }   

   if(data_struct.size > IK220_MAX_INPUT_SIZE)
      return -EFAULT;

   nr_of_words = data_struct.size / 2;

   if(ik220_read_dataw(data_struct.axis, data_struct.address, data, nr_of_words) < 0)
      return -EFAULT;    
 
   if(copy_to_user((char *) data_struct.data, data, data_struct.size))
      return -EFAULT;

return 0;
}   

static long ik220_output(unsigned long data_addr)
{
   u16 data[IK220_MAX_OUTPUT_SIZE];
   int card_nr;
   struct ik220_data data_struct;
   unsigned int nr_of_words;

/* FIXME: Add common error-handler */
   if(copy_from_user(&data_struct, (struct ik220_dataram *)data_addr, sizeof(data_struct)))
      return -EFAULT;

   if(((card_nr = (data_struct.axis / 2)) > IK220_MAX_CARDS - 1) ||
      (card_nr < 0) ||
      (ik220_card[card_nr].dev == NULL))
   {
      return -ENODEV;
   }   

   if(data_struct.size > IK220_MAX_OUTPUT_SIZE)
      return -EFAULT;

   if(copy_from_user(data, (u16 *)data_struct.data, data_struct.size))
      return -EFAULT;

   nr_of_words = data_struct.size / 2;

   if(ik220_write_dataw(data_struct.axis, data_struct.address, data, nr_of_words) < 0)
      return -EFAULT;    

return 0;
}   


static long ik220_get_status(char *arg)
{
   unsigned long addresses[IK220_MAX_AXIS];
   u16 status;
   int i;

   for(i = 0; i < IK220_MAX_CARDS; ++i)
   {
      if(ik220_card[i].dev == NULL)
      {
         addresses[2 * i] = 0L;
         addresses[(2 * i) + 1] = 0L;
      }
      else
      {
         if(ik220_read_dataw(2 * i, IK220_CODEREGISTER, &status, 1) < 0)
         {
            addresses[2 * i] = 0L;
         }
         else
         {
            if(status != 0x0007)
               addresses[2 * i] = 0L;
            else
               addresses[2 * i] = (u32)ik220_card[i].iomem_1_virtual;
         }

         if(ik220_read_dataw((2 * i) + 1, IK220_CODEREGISTER, &status, 1) < 0)
         {
            addresses[(2 * i) + 1] = 0L;
         }
         else
         {
            if(status != 0x0007)
               addresses[(2 * i) + 1] = 0L;
            else
               addresses[(2 * i) + 1] = (u32)ik220_card[i].iomem_2_virtual;
         }
      }
   }

   //printk("Kurz davor!\n");
   put_user(addresses[0], (u32 *)arg);
   //printk("Kurz dazwischen!\n");
   put_user(addresses[1], ((u32 *)arg) + 1);
   put_user(addresses[2], ((u32 *)arg) + 2);
   put_user(addresses[3], ((u32 *)arg) + 3);
   put_user(addresses[4], ((u32 *)arg) + 4);
   put_user(addresses[5], ((u32 *)arg) + 5);
   /* FIXME: copy_to_user not working here! */

return 0;
}

static long ik220_download(unsigned long data_addr)
{
   u16 *prog;
   int card_nr;
   struct ik220_data prog_struct;
   unsigned int nr_of_words;
/* FIXME: Add common error-handler */
   if(copy_from_user(&prog_struct, (struct ik220_program *)data_addr, sizeof(prog_struct)))
      return -EFAULT;

   if(((card_nr = (prog_struct.axis / 2)) > IK220_MAX_CARDS - 1) ||
      (card_nr < 0) ||
      (ik220_card[card_nr].dev == NULL))
   {
      return -ENODEV;
   }   

   if(prog_struct.size > IK220_MAX_PROG_SIZE)
      return -EFAULT;

   prog = kmalloc(prog_struct.size, GFP_KERNEL);
  
   if(copy_from_user(prog, (u16 *)prog_struct.data, prog_struct.size))
   {
      kfree(prog);
      return -EFAULT;
   }

   nr_of_words = prog_struct.size / 2;

   if(ik220_install_program(prog_struct.axis, prog_struct.address, prog, nr_of_words))
   {
      kfree(prog);
      return -EFAULT;
   }
   
   kfree(prog);

return 0;
}

static long ik220_get_version(unsigned long data_addr)
{
   u8 version[IK220_VERSION_SIZE];
   u16 tmp_word;
   int card_nr;
   struct ik220_data data_struct;
   int i, j;
   int rc;


/* FIXME: Add common error-handler */
   if(copy_from_user(&data_struct, (struct ik220_dataram *)data_addr, sizeof(data_struct)))
      return -EFAULT;

   if(((card_nr = (data_struct.axis / 2)) > IK220_MAX_CARDS - 1) ||
      (card_nr < 0) ||
      (ik220_card[card_nr].dev == NULL))
   {
      return -ENODEV;
   }   

   if(data_struct.size != IK220_VERSION_SIZE)
      return -EFAULT;

   tmp_word = IK220_CMD_GET_VERSION;
   if((rc = ik220_write_dataw(data_struct.axis, IK220_CMDREGISTER, &tmp_word, 1)) < 0)
      return rc;

   if((rc = ik220_read_dataw(data_struct.axis, IK220_FLAG_1_REGISTER, &tmp_word, 1)) < 0)
      return rc;

   while(!(tmp_word & IK220_G28SEM_0))
   {
      /*FIXME: Do some idle-stuff! Save to call schedule? */
      /*FIXME: TIMEOUT!!!!!!!!!!!!!!!!!!!!!!!! */
      if((rc = ik220_read_dataw(data_struct.axis, IK220_FLAG_1_REGISTER, &tmp_word, 1)) < 0)
         return rc; /* What about set command? */
   }   

   if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_0, &tmp_word, 1)) < 0)
      return rc;

   if(tmp_word == IK220_CMD_GET_VERSION)
   {
      i = 0;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_1, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_2, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_3, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_4, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_5, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      if((rc = ik220_read_dataw(data_struct.axis, IK220_DATAREGISTER_6, &tmp_word, 1)) < 0)
         return rc;
      version[i++] = tmp_word >>8;
      version[i++] = tmp_word & 0x00ff;

      version[i++] = '\0';
      for(j = 0; j < (sizeof driver_version); ++j)
            version[i++] = driver_version[j];

   }
   else
   {
      version[0] = '\0';
   }
   //printk("Alles klar!\n");
    
   if(copy_to_user((char *) (data_struct.data), version, data_struct.size))
      return -EFAULT;

return 0;
}



/************************************************************************************************/
/* IOCTL                                                                                        */
/************************************************************************************************/
static  int ik220_driver_ioctl (struct inode *inode, struct file *file,
                                unsigned int cmd, unsigned long arg)
{
   long rc = 0;

   if(_IOC_TYPE(cmd) != IK220_IOCTL_TYPE)
      return -ENOTTY;

   if(_IOC_NR(cmd) > IK220_MAX_IOCTL)
      return -ENOTTY;

   switch(_IOC_NR(cmd))
   {
      case IK220_IOCTL_INPUT:    rc = ik220_input(arg);
                                 break;
      case IK220_IOCTL_OUTPUT:   rc = ik220_output(arg);
                                 break;
      case IK220_IOCTL_VERSION:  rc = ik220_get_version(arg);
                                 break;
      case IK220_IOCTL_STATUS:   rc = ik220_get_status((char *)arg);
                                 break;
      case IK220_IOCTL_DOWNLOAD: rc = ik220_download(arg);
                                 printk("Download status: %ld\n", rc);
                                 break;
      default: rc = -ENOTTY;
   }   



return rc;
}



/************************************************************************************************/
/* Functions for device init and exit                                                           */
/************************************************************************************************/
static void __ik220_cleanup_card(struct ik220_device_data *device)
{
   struct pci_dev *pdev;

   if(device != NULL)
   {
      pdev = device->dev;
      if(device->conf_iomem_virtual != NULL)
      {
	 iounmap(device->conf_iomem_virtual); 
	 device->conf_iomem_virtual = NULL;
      }
      if(device->iomem_1_virtual != NULL)
      {
	 iounmap(device->iomem_1_virtual); 
	 device->iomem_1_virtual = NULL;
      }
      if(device->iomem_2_virtual != NULL)
      {
	 iounmap(device->iomem_2_virtual);
	 device->iomem_2_virtual = NULL;
      }
      device->dev = NULL;
      device->conf_iomem_start = 0;
      device->conf_iomem_end   = 0;
      device->conf_iomem_flags = 0;
      device->conf_iomem_len = 0;
      device->iomem_1_start = 0;
      device->iomem_1_end   = 0;
      device->iomem_1_flags = 0;
      device->iomem_1_len = 0;
   
      if(pdev != NULL)
      {
         pci_set_drvdata(pdev, NULL);
         pci_release_regions(pdev);
      }
   }
   --number_of_cards;
}

static void __devexit ik220_remove_one (struct pci_dev *pdev)
{
   struct ik220_device_data *dev;
   dev = pci_get_drvdata(pdev);
   if(dev != NULL)
   {
      __ik220_cleanup_card(dev);
   }
}

static int __devinit ik220_init_board (struct pci_dev *pdev)
{
   int rc, slot;

   if(number_of_cards >= IK220_MAX_CARDS)
   {
      printk(KERN_WARNING "%s: Found Card beyond cardlimit of %d cards\n", driver_name, IK220_MAX_CARDS);
      rc = number_of_cards;
      goto err_out;
   }   
    
   printk(KERN_INFO "%s: Card found\n", driver_name); 


   /* find first empty slot in ik220_card */
   for(slot = 0; (slot < IK220_MAX_CARDS) && (ik220_card[slot].dev != NULL); ++slot);
      
   if(slot == IK220_MAX_CARDS)
   {
      printk(KERN_ERR "%s: No free slot in driver_struct while there should be!!!\n", driver_name);
      rc = number_of_cards;
      goto err_out;
   }   
   else
   {
      printk(KERN_INFO "%s: Card will be inserted in slot %d!\n", driver_name, slot);
   }
   
   ++number_of_cards;

   ik220_card[slot].dev = pdev;
   pci_set_drvdata(pdev, &(ik220_card[slot]));

   /* From now on we need to jump to err_cleanup to undo all initializations */

   rc = pci_enable_device(pdev);
   if(rc) goto err_cleanup;

   ik220_card[slot].conf_iomem_start = pci_resource_start(pdev, 0);
   ik220_card[slot].conf_iomem_end   = pci_resource_end(pdev, 0);
   ik220_card[slot].conf_iomem_flags = pci_resource_flags(pdev, 0);
   ik220_card[slot].conf_iomem_len = pci_resource_len(pdev, 0);
   printk(KERN_INFO "%s: Config-Region start: 0x%lX end: 0x%lX flags: 0x%lX\n", 
                                                              driver_name, 
                                                              ik220_card[slot].conf_iomem_start,
                                                              ik220_card[slot].conf_iomem_end,
                                                              ik220_card[slot].conf_iomem_flags);


   ik220_card[slot].iomem_1_start = pci_resource_start(pdev, 2);
   ik220_card[slot].iomem_1_end   = pci_resource_end(pdev, 2);
   ik220_card[slot].iomem_1_flags = pci_resource_flags(pdev, 2);
   ik220_card[slot].iomem_1_len = pci_resource_len(pdev, 2);
   printk(KERN_INFO "%s: 1st IO-Region start: 0x%lX end: 0x%lX flags: 0x%lX\n", 
                                                              driver_name, 
                                                              ik220_card[slot].iomem_1_start,
                                                              ik220_card[slot].iomem_1_end,
                                                              ik220_card[slot].iomem_1_flags);



   ik220_card[slot].iomem_2_start = pci_resource_start(pdev, 3);
   ik220_card[slot].iomem_2_end   = pci_resource_end(pdev, 3);
   ik220_card[slot].iomem_2_flags = pci_resource_flags(pdev, 3);
   ik220_card[slot].iomem_2_len = pci_resource_len(pdev, 3);
   printk(KERN_INFO "%s: 2nd IO-Region start: 0x%lX end: 0x%lX flags: 0x%lX\n", 
                                                              driver_name, 
                                                              ik220_card[slot].iomem_2_start,
                                                              ik220_card[slot].iomem_2_end,
                                                              ik220_card[slot].iomem_2_flags);


   /* Checking regions */
   if (!(ik220_card[slot].conf_iomem_flags & IORESOURCE_MEM))
   {
      printk(KERN_ERR "%s: 1st IO-Resource is no IOMemory! Wrong Card?\n", driver_name);
      rc = -ENODEV;
      goto err_cleanup;
   }

   if (!(ik220_card[slot].iomem_1_flags & IORESOURCE_MEM))
   {
      printk(KERN_ERR "%s: 2nd IO-Resource is no IOMemory! Wrong Card?\n", driver_name);
      rc = -ENODEV;
      goto err_cleanup;
   }

   if (!(ik220_card[slot].iomem_2_flags & IORESOURCE_MEM))
   {
      printk(KERN_ERR "%s: 3rd IO-Resource is no IOMemory! Wrong Card?\n", driver_name);
      rc = -ENODEV;
      goto err_cleanup;
   }

   if (pci_request_regions(pdev, "ik220") != 0) { 
      printk(KERN_ERR "%s: Unable to request regions\n", driver_name);
      rc = -ENODEV;
      goto err_cleanup;
  }

   ik220_card[slot].conf_iomem_virtual = ioremap_nocache(ik220_card[slot].conf_iomem_start, ik220_card[slot].conf_iomem_len);
   printk(KERN_INFO "%s: Config-Region remaped to virtual address 0x%lX\n", driver_name,
                                                                        (unsigned long)ik220_card[slot].conf_iomem_virtual); 
   ik220_card[slot].iomem_1_virtual = ioremap(ik220_card[slot].iomem_1_start, ik220_card[slot].iomem_1_len);
   printk(KERN_INFO "%s: 1st IO-Region remaped to virtual address 0x%lX\n", driver_name,
                                                                        (unsigned long)ik220_card[slot].iomem_1_virtual); 
   ik220_card[slot].iomem_2_virtual = ioremap(ik220_card[slot].iomem_2_start, ik220_card[slot].iomem_2_len);
   printk(KERN_INFO "%s: 2nd IO-Region remaped to virtual address 0x%lX\n", driver_name,
                                                                        (unsigned long)ik220_card[slot].iomem_2_virtual); 



   return 0;


err_cleanup:
   __ik220_cleanup_card(&(ik220_card[slot]));

err_out:
   return rc;

}


static int __devinit ik220_init_one (struct pci_dev *pdev,
                                     const struct pci_device_id *ent)
{
   int i;

if((pdev == NULL) || (ent == NULL)) return -1;

   i = ik220_init_board(pdev);
   if(i < 0) return i;
 
   return 0;
}   


/************************************************************************************************/
/* Driver init and exit                                                                         */
/************************************************************************************************/
static struct pci_driver ik220_driver = {
        name:           DRV_NAME,
        id_table:       ik220_tbl,
        probe:          ik220_init_one,
        remove:         ik220_remove_one,
};

struct file_operations ik220_fops = {
   ioctl:       ik220_driver_ioctl,
};

static int __init ik220_init_module (void)
{
   int rc, i;
   
   for(i = 0; i < IK220_MAX_CARDS; ++i)
   {
      ik220_card[i].dev = NULL;
      ik220_card[i].conf_iomem_start = 0;
      ik220_card[i].conf_iomem_end   = 0;
      ik220_card[i].conf_iomem_flags = 0;
      ik220_card[i].conf_iomem_len = 0;
      ik220_card[i].conf_iomem_virtual = NULL;
      ik220_card[i].iomem_1_start = 0;
      ik220_card[i].iomem_1_end   = 0;
      ik220_card[i].iomem_1_flags = 0;
      ik220_card[i].iomem_1_len = 0;
      ik220_card[i].iomem_1_virtual = NULL;
      ik220_card[i].iomem_2_start = 0;
      ik220_card[i].iomem_2_end   = 0;
      ik220_card[i].iomem_2_flags = 0;
      ik220_card[i].iomem_2_len = 0;
      ik220_card[i].iomem_2_virtual = NULL;
   }
      

   rc = pci_register_driver(&ik220_driver);
   if(rc < 0)
   {
      return -ENODEV;
   }

/*FIXME: Use automatic generation of device-node */
   rc = register_chrdev(61, "ik220", &ik220_fops);

   if(rc < 0)
   {
      return rc;
   }

   return 0;
}

static void __exit ik220_cleanup_module (void)
{
   pci_unregister_driver(&ik220_driver);
/*FIXME: Use automatic device-node */
   unregister_chrdev(61, "ik220");
   printk(KERN_INFO "%s: Driver removed\n", driver_name);
}

module_init(ik220_init_module);
module_exit(ik220_cleanup_module);

