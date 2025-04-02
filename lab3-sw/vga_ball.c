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

// Register offsets
#define BG_RED(x)     ((x) + 0)
#define BG_GREEN(x)   ((x) + 1)
#define BG_BLUE(x)    ((x) + 2)
#define BALL_X_L(x)   ((x) + 3)
#define BALL_X_H(x)   ((x) + 4)
#define BALL_Y_L(x)   ((x) + 5)
#define BALL_Y_H(x)   ((x) + 6)

struct vga_ball_dev {
	struct resource res;
	void __iomem *virtbase;
	vga_ball_color_t background;
	unsigned short ball_x, ball_y;
} dev;

// Write background color to registers
static void write_background(vga_ball_color_t *background) {
	iowrite8(background->red,   BG_RED(dev.virtbase));
	iowrite8(background->green, BG_GREEN(dev.virtbase));
	iowrite8(background->blue,  BG_BLUE(dev.virtbase));
	dev.background = *background;
}

// Write ball position to registers
static void write_position(unsigned short x, unsigned short y) {
	iowrite8(x & 0xFF, BALL_X_L(dev.virtbase));
	iowrite8((x >> 8) & 0x03, BALL_X_H(dev.virtbase));
	iowrite8(y & 0xFF, BALL_Y_L(dev.virtbase));
	iowrite8((y >> 8) & 0x03, BALL_Y_H(dev.virtbase));
	dev.ball_x = x;
	dev.ball_y = y;
}

// Read ball position from memory (optional, reads last written value)
static void read_position(unsigned short *x, unsigned short *y) {
	*x = dev.ball_x;
	*y = dev.ball_y;
}

static long vga_ball_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
	vga_ball_arg_t vla;

	switch (cmd) {
	case VGA_BALL_WRITE_BACKGROUND:
		if (copy_from_user(&vla, (vga_ball_arg_t *)arg, sizeof(vga_ball_arg_t)))
			return -EACCES;
		write_background(&vla.background);
		break;

	case VGA_BALL_READ_BACKGROUND:
		vla.background = dev.background;
		if (copy_to_user((vga_ball_arg_t *)arg, &vla, sizeof(vga_ball_arg_t)))
			return -EACCES;
		break;

	case VGA_BALL_WRITE_POSITION:
		if (copy_from_user(&vla, (vga_ball_arg_t *)arg, sizeof(vga_ball_arg_t)))
			return -EACCES;
		write_position(vla.pos_x, vla.pos_y);
		break;

	case VGA_BALL_READ_POSITION:
		read_position(&vla.pos_x, &vla.pos_y);
		if (copy_to_user((vga_ball_arg_t *)arg, &vla, sizeof(vga_ball_arg_t)))
			return -EACCES;
		break;

	default:
		return -EINVAL;
	}
	return 0;
}

static const struct file_operations vga_ball_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = vga_ball_ioctl,
};

static struct miscdevice vga_ball_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DRIVER_NAME,
	.fops = &vga_ball_fops,
};

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
	if (dev.virtbase == NULL) {
		ret = -ENOMEM;
		goto out_release_mem_region;
	}

	write_background(&beige);
	write_position(160, 120);

	return 0;

out_release_mem_region:
	release_mem_region(dev.res.start, resource_size(&dev.res));
out_deregister:
	misc_deregister(&vga_ball_misc_device);
	return ret;
}

static int vga_ball_remove(struct platform_device *pdev) {
	iounmap(dev.virtbase);
	release_mem_region(dev.res.start, resource_size(&dev.res));
	misc_deregister(&vga_ball_misc_device);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id vga_ball_of_match[] = {
	{ .compatible = "csee4840,vga_ball-1.0" },
	{},
};
MODULE_DEVICE_TABLE(of, vga_ball_of_match);
#endif

static struct platform_driver vga_ball_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(vga_ball_of_match),
	},
	.remove = __exit_p(vga_ball_remove),
};

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
MODULE_DESCRIPTION("VGA ball driver with coordinate control");
