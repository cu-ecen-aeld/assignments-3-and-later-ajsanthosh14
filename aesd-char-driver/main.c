/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Santhosh");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{

	 //S:
	 //Ref: LDD - 3.5.1
	 struct aesd_dev *dev;
	 dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
	 filp->private_data = dev;
	 PDEBUG("open");
	 //:S
	 
	return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
	PDEBUG("release");
	 
	//S: We don't have real hardware to shutdown.
	 
	return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{

	ssize_t retval = 0;

	//Ref: LLD 3.7.1
	ssize_t entry_offset;
	ssize_t bRead;
	
	
	struct aesd_buffer_entry *read_entry = NULL;
	struct aesd_dev *dev =(struct aesd_dev *)filp->private_data;
	PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

	// mutex lock
	if(mutex_lock_interruptible(&dev->dev_mutex))
		return -ERESTARTSYS;
	
	
	// find the entry from  the device buffer
	read_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->dev_buff, *f_pos, &entry_offset);
	if(read_entry == NULL)
		goto out;
		
	// update read count
	bRead = read_entry->size - entry_offset; 
	if(bRead > count)
		bRead = count;
		
	// copy from kernel to userspace	
	if(copy_to_user(buf, read_entry->buffptr + entry_offset ,bRead)){
		retval = -EFAULT;
		goto out;
	}
	
	// update file position
	*f_pos += bRead;
	retval =bRead;	

out:
	mutex_unlock(&dev->dev_mutex); 
	return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	ssize_t retval = -ENOMEM;
	ssize_t bwrite;
	struct aesd_dev *dev = (struct aesd_dev *)filp->private_data;
	const char* free_write_entry = NULL;
	
	PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
	
	// mutex lock
	if(mutex_lock_interruptible(&dev->dev_mutex))
		return -ERESTARTSYS;
	 
	if(dev == NULL) 
		goto out;
	
	// to check the size
	if(!dev->write_entry.size){
		
		dev->write_entry.buffptr = kmalloc(count * sizeof(char *) , GFP_KERNEL);
		
		if(!dev->write_entry.buffptr){
			retval = -ENOMEM;
			goto out;
			
		}
	}else{
	
		dev->write_entry.buffptr = krealloc(dev->write_entry.buffptr, (dev->write_entry.size + count), GFP_KERNEL);
		
		if(!dev->write_entry.buffptr){
			retval = -ENOMEM;
			goto out;
		}
	}
 

	// read from user buffer
	bwrite = copy_from_user((void *)(&dev->write_entry.buffptr[dev->write_entry.size]), buf, count);

	// update the size of the write_entry
	retval = count - bwrite;
	dev->write_entry.size += retval;	
	
	// Ref: https://man7.org/linux/man-pages/man3/memchr.3.html
	// To check the null character at the end of the string.
	if(memchr(dev->write_entry.buffptr, '\n', dev->write_entry.size) != NULL){
		free_write_entry = aesd_circular_buffer_add_entry(&dev->dev_buff, &dev->write_entry);
		
		// free the write_entry
		if(free_write_entry != NULL)
			kfree(free_write_entry);
		
		dev->write_entry.size = 0;
		dev->write_entry.buffptr = NULL;
	}
	
	*f_pos = 0;
  out:	 
	mutex_unlock(&dev->dev_mutex);	 
	return retval;
}
struct file_operations aesd_fops = {
	.owner =    THIS_MODULE,
	.read =     aesd_read,
	.write =    aesd_write,
	.open =     aesd_open,
	.release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
	int err, devno = MKDEV(aesd_major, aesd_minor);

	cdev_init(&dev->cdev, &aesd_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &aesd_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	if (err) {
		printk(KERN_ERR "Error %d adding aesd cdev", err);
	}
	return err;
}



int aesd_init_module(void)
{
	dev_t dev = 0;
	int result;
	result = alloc_chrdev_region(&dev, aesd_minor, 1,
			"aesdchar");
	aesd_major = MAJOR(dev);
	if (result < 0) {
		printk(KERN_WARNING "Can't get major %d\n", aesd_major);
		return result;
	}
	memset(&aesd_device,0,sizeof(struct aesd_dev));
	 
	 //S: initialize mutex and buffer
	 //Ref: Assignment 7 --> aesd-circular-buffer
	 aesd_circular_buffer_init(&(aesd_device.dev_buff));	 
	 mutex_init(&(aesd_device.dev_mutex));
	 //:S

	result = aesd_setup_cdev(&aesd_device);

	if( result ) {
		unregister_chrdev_region(dev, 1);
	}
	return result;

}

void aesd_cleanup_module(void)
{

	//Ref: Assignment 7 -> aesd-circular-buffer.h
	uint8_t index;
	struct aesd_buffer_entry *dev_entry;
	
	
	dev_t devno = MKDEV(aesd_major, aesd_minor);

	cdev_del(&aesd_device.cdev);

	
	//S: Free the device buffer
	kfree(aesd_device.write_entry.buffptr);
	
	AESD_CIRCULAR_BUFFER_FOREACH(dev_entry, &aesd_device.dev_buff, index){
		if(dev_entry->buffptr != NULL)
			kfree(dev_entry->buffptr);
	
	}
	unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
