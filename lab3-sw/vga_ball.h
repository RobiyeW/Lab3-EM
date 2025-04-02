#ifndef _VGA_BALL_H
#define _VGA_BALL_H

#include <linux/ioctl.h>
#include <linux/types.h>  // Kernel-safe integer types

typedef struct {
  unsigned char red, green, blue;
} vga_ball_color_t;

typedef struct {
  __u16 x;
  __u16 y;
} vga_ball_position_t;

typedef struct {
  vga_ball_color_t background;
  vga_ball_position_t position;
} vga_ball_arg_t;

#define VGA_BALL_MAGIC 'q'

#define VGA_BALL_WRITE_BACKGROUND _IOW(VGA_BALL_MAGIC, 1, vga_ball_arg_t)
#define VGA_BALL_READ_BACKGROUND  _IOR(VGA_BALL_MAGIC, 2, vga_ball_arg_t)
#define VGA_BALL_WRITE_POSITION   _IOW(VGA_BALL_MAGIC, 3, vga_ball_arg_t)

#endif
