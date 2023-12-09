#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>   //kmalloc()
#include <linux/uaccess.h>      //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/err.h>
#include <linux/namei.h>
#include <linux/vmalloc.h>
#include <linux/dcache.h>
#include <linux/types.h>
#include <linux/numa.h>
#include<linux/gfp.h>
#include <linux/mm.h>
#include <linux/buffer_head.h>
#include  <linux/mm_types.h>
#include <linux/rmap.h>
#include "/home/boss/linux-6.2/mm/slab.h"
#include "core_mode.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TatianaP");
MODULE_DESCRIPTION("Lab Work 2 ioctl slabtop -o");

// to registrate my device in dev
dev_t dev = 0;
static struct class *dev_class;

// character device for ioctl
static struct cdev my_driver_cdev;

// file status
static int status_driver_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...\n");
        return 0;
}

static int status_driver_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...\n");
        return 0;
}


// process status
static ssize_t status_driver_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");
        return 0;
}

static ssize_t status_driver_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");
        return len;
}


// print my slabinfo struct to log
static void print_slabinfo(struct my_slabinfo* slb){
        pr_info("Slabinfo \n");
        pr_info("number of objects = %lu, \n", slb->num_objs);
        pr_info("active objects = %lu, \n", slb->active_objs);
       	pr_info("fake use = %u, \n", slb->use);
       	pr_info("object size in bytes = %lu", slb->obj_size);
        pr_info("number of slabs = %lu, \n", slb->num_slabs);
        pr_info("objects per slab = %u, \n", slb->objects_per_slab);
        pr_info("fake cache size = %luK, \n", slb->cache_size);
        pr_info("\n");
}

// print answer struct to log
static void print_answer(struct answer* a){
        pr_info("\ngoing to send:");
        print_slabinfo(&(a->sld));
}


// functions from source code
static inline unsigned long node_nr_slabs(struct kmem_cache_node *n)
{
	return atomic_long_read(&n->nr_slabs);
}

static inline unsigned long node_nr_objs(struct kmem_cache_node *n)
{
	return atomic_long_read(&n->total_objects);
}

static unsigned long count_partial(struct kmem_cache_node *n,
					int (*get_count)(struct slab *))
{
	unsigned long flags;
	unsigned long x = 0;
	struct slab *slab;

	spin_lock_irqsave(&n->list_lock, flags);
	list_for_each_entry(slab, &n->partial, slab_list)
		x += get_count(slab);
	spin_unlock_irqrestore(&n->list_lock, flags);
	return x;
}
static int count_free(struct slab *slab)
{
	return slab->objects - slab->inuse;
}

#define OO_SHIFT	16
#define OO_MASK		((1 << OO_SHIFT) - 1)
static inline unsigned int oo_order(struct kmem_cache_order_objects x)
{
	return x.x >> OO_SHIFT;
}

static inline unsigned int oo_objects(struct kmem_cache_order_objects x)
{
	return x.x & OO_MASK;
}

// from kmem_cache to slabinfo from source code
void get_slabinfo(struct kmem_cache *s, struct slabinfo *sinfo)
{
	unsigned long nr_slabs = 0;
	unsigned long nr_objs = 0;
	unsigned long nr_free = 0;
	int node;
	struct kmem_cache_node *n;

	for_each_kmem_cache_node(s, node, n) {
		nr_slabs += node_nr_slabs(n);
		nr_objs += node_nr_objs(n);
		nr_free += count_partial(n, count_free);
	}

	sinfo->active_objs = nr_objs - nr_free;
	sinfo->num_objs = nr_objs;
	sinfo->active_slabs = nr_slabs;
	sinfo->num_slabs = nr_slabs;
	sinfo->objects_per_slab = oo_objects(s->oo);
	sinfo->cache_order = oo_order(s->oo);
}

// preparing real slabinfo for answer
void set_my_type (struct my_slabinfo *m_sinfo, struct slabinfo *sinfo, struct kmem_cache *kmems){
	m_sinfo->active_objs = sinfo->active_objs;
	m_sinfo->num_objs = sinfo->num_objs;
	m_sinfo->num_slabs = sinfo->num_slabs;
	m_sinfo->use = 0;
	m_sinfo->obj_size = kmems->object_size;
	m_sinfo->objects_per_slab = sinfo->objects_per_slab;
	m_sinfo->cache_size = 0;

}


#define SLAB_PANIC		((slab_flags_t __force)0x00040000U)
#define SLAB_NO_USER_FLAGS	((slab_flags_t __force)0x10000000U)


static long core_mode_ioctl(struct file *file, unsigned int cmd, unsigned long arg){

        if(cmd == RW_ANS){
        	struct my_slabinfo *m_slb;
        	struct kmem_cache *my_cache;
        	struct slabinfo *norm_slab;
                struct answer* a = vmalloc(sizeof(struct answer));
                if (!a) {
            		pr_err("Memory allocation failed!\n");
            		a->status = "NOT OK";
           		 return -ENOMEM;
        		}

       		// copy error handler
        	if (copy_from_user(a, (struct answer *)arg, sizeof(struct answer))) {
            		pr_err("Can't read answer from user space!\n");
            		a->status = "NOT OK";
            		vfree(a);
            		return -EFAULT;
       			}


                pr_info("STRUCT_ID = %d\n", a->st_num);

              	switch(a->st_num){
              		default:
              		case 1:
              			my_cache = KMEM_CACHE(vm_area_struct, SLAB_PANIC|SLAB_ACCOUNT);
              			break;
              		case 2:
              			my_cache = KMEM_CACHE(dentry, SLAB_PANIC|SLAB_ACCOUNT);
              			break;
              		case 3:
              			my_cache = KMEM_CACHE(vmap_area, SLAB_PANIC|SLAB_ACCOUNT);
              			break;
              		case 4:
              			my_cache = KMEM_CACHE(anon_vma_chain, SLAB_PANIC|SLAB_ACCOUNT);
              			break;
              	}


		if (!my_cache){
			printk("No cache");
			return -EFAULT;
		}
		else{
			norm_slab = kmalloc(sizeof(struct slabinfo), GFP_KERNEL);
			if (!norm_slab) {
            			pr_err("Memory allocation for slabinfo failed!\n");
            			a->status = "NOT OK";
            			vfree(a);
            			return -ENOMEM;
        			}

        		// sample fill slab
        		norm_slab->active_objs = 10;

        		// getting slabinfo

               		get_slabinfo(my_cache, norm_slab);
               		m_slb = kmalloc(sizeof(struct my_slabinfo), GFP_KERNEL);
			if (!m_slb) {
    				pr_err("Memory allocation for my_slabinfo failed!\n");
    				a->status = "NOT OK";
    				return -ENOMEM; // allocation error handler
			}


               		set_my_type(m_slb, norm_slab, my_cache);
               		a->sld =*m_slb;
               		a->status = "OK";
               		print_answer(a);
               		kfree(norm_slab);
               		kfree(m_slb);
                	if (copy_to_user((struct answer *) arg, a, sizeof(struct answer))) pr_err("Data read error!\n");
                }

		vfree(a);
        }

        return 0;
}


// device file operation structure
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = status_driver_read,
        .write          = status_driver_write,
        .open           = status_driver_open,
        .unlocked_ioctl = core_mode_ioctl,
        .release        = status_driver_release,
};


// driver initialization
static int __init my_driver_init(void)
{
        // allocation of MYMAJOR (driver sys id)
        if((alloc_chrdev_region(&dev, 0, 1, "my_driver")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

        // initialization of the device
        cdev_init(&my_driver_cdev,&fops);

        // adding device to /dev
        if((cdev_add(&my_driver_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }

        // driver class in /sys/class
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"my_driver_class"))){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }

        // device creation
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL, DEVICE_NAME))){
            pr_err("Cannot create the Device \n");
            goto r_device;
        }
        pr_info("Device Driver Inserted Done\n");
        return 0;

// error handler
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

// exit driver
static void __exit my_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&my_driver_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit)