/* 
 * Device driver for the VGA video generator with ball control
 * Columbia University - CSEE 4840 Lab 3
 */

 #include <linux/module.h>
 #include <linux/init.h>
 #include <linux/errno.h>
 #include <linux/version.h>
 #include <linux/kernel.h>
 #include <linux/platform_device.h>
 #include <linux/miscdevice.h>
 #include <linux/slab.h>
 #include <linux/io.h>
 #include <linux/of.h>
 #include <linux/of_address.h>
 #include <linux/fs.h>
 #include <linux/uaccess.h>
 #include "vga_ball.h"
 
 #define DRIVER_NAME "vga_ball"
 
 /* Device register offsets */
 #define BG_RED(x)       (x)
 #define BG_GREEN(x)     ((x) + 1)
 #define BG_BLUE(x)      ((x) + 2)
 #define BALL_X_LO(x)    ((x) + 3)
 #define BALL_X_HI(x)    ((x) + 4)
 #define BALL_Y_LO(x)    ((x) + 5)
 #define BALL_Y_HI(x)    ((x) + 6)
 
 /*
  * Device structure
  */
 struct vga_ball_dev {
	 struct resource res;             // Resource: register address range
	 void __iomem *virtbase;          // Virtual memory base address
	 vga_ball_color_t background;     // Stored background color
	 vga_ball_position_t position;    // Stored ball position
 } dev;
 
 /*
  * Write background color to registers
  */
 static void write_background(vga_ball_color_t *bg) {
	 iowrite8(bg->red,   BG_RED(dev.virtbase));
	 iowrite8(bg->green, BG_GREEN(dev.virtbase));
	 iowrite8(bg->blue,  BG_BLUE(dev.virtbase));
	 dev.background = *bg;
 }
 
 /*
  * Write ball position to registers
  */
 static void write_position(vga_ball_position_t *pos) {
	 iowrite8(pos->x & 0xFF, BALL_X_LO(dev.virtbase));
	 iowrite8((pos->x >> 8) & 0xFF, BALL_X_HI(dev.virtbase));
	 iowrite8(pos->y & 0xFF, BALL_Y_LO(dev.virtbase));
	 iowrite8((pos->y >> 8) & 0xFF, BALL_Y_HI(dev.virtbase));
	 dev.position = *pos;
 }
 
 /*
  * ioctl handler
  */
 static long vga_ball_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
	 vga_ball_arg_t vla;
 
	 switch (cmd) {
	 case VGA_BALL_WRITE_BACKGROUND:
		 if (copy_from_user(&vla, (vga_ball_arg_t *) arg, sizeof(vga_ball_arg_t)))
			 return -EACCES;
		 write_background(&vla.background);
		 break;
 
	 case VGA_BALL_READ_BACKGROUND:
		 vla.background = dev.background;
		 if (copy_to_user((vga_ball_arg_t *) arg, &vla, sizeof(vga_ball_arg_t)))
			 return -EACCES;
		 break;
 
	 case VGA_BALL_WRITE_POSITION:
		 if (copy_from_user(&vla, (vga_ball_arg_t *) arg, sizeof(vga_ball_arg_t)))
			 return -EACCES;
		 write_position(&vla.position);
		 break;
 
	 default:
		 return -EINVAL;
	 }
 
	 return 0;
 }
 
 /*
  * File operations
  */
 static const struct file_operations vga_ball_fops = {
	 .owner = THIS_MODULE,
	 .unlocked_ioctl = vga_ball_ioctl,
 };
 
 /*
  * Misc device structure
  */
 static struct miscdevice vga_ball_misc_device = {
	 .minor = MISC_DYNAMIC_MINOR,
	 .name = DRIVER_NAME,
	 .fops = &vga_ball_fops,
 };
 
 /*
  * Probe function: called when device is found
  */
 static int __init vga_ball_probe(struct platform_device *pdev) {
	 vga_ball_color_t beige = { 0xf9, 0xe4, 0xb7 };
	 int ret;
 
	 ret = misc_register(&vga_ball_misc_device);
	 if (ret)
		 return ret;
 
	 ret = of_address_to_resource(pdev->dev.of_node, 0, &dev.res);
	 if (ret)
		 goto out_deregister;
 
	 if (request_mem_region(dev.res.start, resource_size(&dev.res), DRIVER_NAME) == NULL) {
		 ret = -EBUSY;
		 goto out_deregister;
	 }
 
	 dev.virtbase = of_iomap(pdev->dev.of_node, 0);
	 if (!dev.virtbase) {
		 ret = -ENOMEM;
		 goto out_release;
	 }
 
	 write_background(&beige);  // Initial background color
	 return 0;
 
 out_release:
	 release_mem_region(dev.res.start, resource_size(&dev.res));
 out_deregister:
	 misc_deregister(&vga_ball_misc_device);
	 return ret;
 }
 
 /*
  * Remove function
  */
 static int vga_ball_remove(struct platform_device *pdev) {
	 iounmap(dev.virtbase);
	 release_mem_region(dev.res.start, resource_size(&dev.res));
	 misc_deregister(&vga_ball_misc_device);
	 return 0;
 }
 
 /*
  * Match table for compatible hardware in the device tree
  */
 #ifdef CONFIG_OF
 static const struct of_device_id vga_ball_of_match[] = {
	 { .compatible = "csee4840,vga_ball-1.0" },
	 {},
 };
 MODULE_DEVICE_TABLE(of, vga_ball_of_match);
 #endif
 
 /*
  * Platform driver structure
  */
 static struct platform_driver vga_ball_driver = {
	 .driver = {
		 .name = DRIVER_NAME,
		 .owner = THIS_MODULE,
		 .of_match_table = of_match_ptr(vga_ball_of_match),
	 },
	 .remove = __exit_p(vga_ball_remove),
 };
 
 /*
  * Module initialization and exit
  */
 static int __init vga_ball_init(void) {
	 pr_info(DRIVER_NAME ": init\n");
	 return platform_driver_probe(&vga_ball_driver, vga_ball_probe);
 }
 
 static void __exit vga_ball_exit(void) {
	 platform_driver_unregister(&vga_ball_driver);
	 pr_info(DRIVER_NAME ": exit\n");
 }
 
 module_init(vga_ball_init);
 module_exit(vga_ball_exit);
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
 MODULE_DESCRIPTION("VGA ball driver with (x, y) position control");
 