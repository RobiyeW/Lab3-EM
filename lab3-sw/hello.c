#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int vga_ball_fd;

// Read and print the background color
void print_background_color() {
  vga_ball_arg_t vla;

  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
    return;
  }
  printf("Background color: %02x %02x %02x\n",
         vla.background.red, vla.background.green, vla.background.blue);
}

// Set the background color
void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
    return;
  }
}

// Set the ball position
void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla;
  vla.pos_x = x;
  vla.pos_y = y;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
    return;
  }
}

// Read and print the ball position
void print_ball_position() {
  vga_ball_arg_t vla;
  if (ioctl(vga_ball_fd, VGA_BALL_READ_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_READ_POSITION) failed");
    return;
  }
  printf("Ball position: (%u, %u)\n", vla.pos_x, vla.pos_y);
}

int main() {
  vga_ball_arg_t vla;
  int i;
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t colors[] = {
    { 0xff, 0x00, 0x00 }, /* Red */
    { 0x00, 0xff, 0x00 }, /* Green */
    { 0x00, 0x00, 0xff }, /* Blue */
    { 0xff, 0xff, 0x00 }, /* Yellow */
    { 0x00, 0xff, 0xff }, /* Cyan */
    { 0xff, 0x00, 0xff }, /* Magenta */
    { 0x80, 0x80, 0x80 }, /* Gray */
    { 0x00, 0x00, 0x00 }, /* Black */
    { 0xff, 0xff, 0xff }  /* White */
  };

#define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  printf("Initial background: ");
  print_background_color();
  printf("Initial ball position: ");
  print_ball_position();

  // Ball movement direction
  int ball_dir_x = 1; // 1 for right, -1 for left
  int ball_dir_y = 1; // 1 for down, -1 for up

  // Animate color and position with screen boundaries
  for (i = 0; i < 1000; i++) {
    set_background_color(&colors[i % COLORS]);
    
    // Get current position and move ball
    vga_ball_arg_t vla;
    ioctl(vga_ball_fd, VGA_BALL_READ_POSITION, &vla);
    
    unsigned short new_x = vla.pos_x + ball_dir_x;
    unsigned short new_y = vla.pos_y + ball_dir_y;

    // Check for border collision and reverse direction
    if (new_x >= 100 || new_x < 0) ball_dir_x = -ball_dir_x;
    if (new_y >= 100 || new_y < 0) ball_dir_y = -ball_dir_y;

    // Set new position
    set_ball_position(new_x, new_y);
    
    print_background_color();
    print_ball_position();
    usleep(100000); // Smooth motion
  }

  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
