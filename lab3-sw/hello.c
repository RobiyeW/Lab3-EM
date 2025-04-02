#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define SCREEN_WIDTH 150
#define SCREEN_HEIGHT 150

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

  // Set initial color and center ball
  set_background_color(&colors[0]);
  unsigned short x = SCREEN_WIDTH / 2;
  unsigned short y = SCREEN_HEIGHT / 2;
  set_ball_position(x, y);

  int dx = 1;
  int dy = 1;

  for (int i = 0; i < 500; i++) {
    // Bounce off edges
    if (x + dx >= SCREEN_WIDTH || x + dx == 0) dx = -dx;
    if (y + dy >= SCREEN_HEIGHT || y + dy == 0) dy = -dy;

    x += dx;
    y += dy;

    set_background_color(&colors[i % COLORS]);
    set_ball_position(x, y);
    print_ball_position();

    usleep(300000); // Slow animation (300 ms)
  }

  printf("VGA BALL Userspace program terminating\n");
  return 0;
}
