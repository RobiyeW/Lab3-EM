#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666   // ~60 FPS (VGA sync)
#define FRAME_SKIP 5                    // Update ball every 5 frames
#define SCALE 4                         // Sub-pixel scaling factor
#define BALL_SIZE 10                    // Size of the ball in pixels

int vga_ball_fd;

void print_background_color() {
  vga_ball_arg_t vla;
  if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
    return;
  }
  printf("Background color: %02x %02x %02x\n",
         vla.background.red, vla.background.green, vla.background.blue);
}

void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla = { .background = *c };
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
    return;
  }
}

void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla = { .pos_x = x, .pos_y = y };
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
    return;
  }
}

void get_ball_position(unsigned short *x, unsigned short *y) {
  vga_ball_arg_t vla;
  if (ioctl(vga_ball_fd, VGA_BALL_READ_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_READ_POSITION) failed");
    return;
  }
  *x = vla.pos_x;
  *y = vla.pos_y;
}

int main() {
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t background = { 0x00, 0x00, 0x00 }; // Black background
  static const vga_ball_color_t ball_colors[] = {
    { 0xff, 0x00, 0x00 }, // Red
    { 0x00, 0xff, 0x00 }, // Green
    { 0x00, 0x00, 0xff }, // Blue
    { 0xff, 0xff, 0x00 }, // Yellow
    { 0x00, 0xff, 0xff }, // Cyan
    { 0xff, 0x00, 0xff }, // Magenta
    { 0xff, 0xff, 0xff }  // White
  };

#define COLORS 9

  printf("VGA ball Userspace program started\n");

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    perror("could not open /dev/vga_ball");
    return -1;
  }

  unsigned short x = BOX_WIDTH / 2;
  unsigned short y = BOX_HEIGHT / 2;
  int dx = 2, dy = 2;
  int color_index = 0;

  // Set initial background
  set_background_color(&background);

  while (1) {
    // Bounce off edges
    if (x + dx >= BOX_WIDTH || x + dx <= 0) {
      dx = -dx;
      color_index = (color_index + 1) % NUM_COLORS; // Change color on bounce
    }

    if (y + dy >= BOX_HEIGHT || y + dy <= 0) {
      dy = -dy;
      color_index = (color_index + 1) % NUM_COLORS;
    }

    x += dx;
    y += dy;

    // Send only new position and (if changed) background color
    set_background_color(&ball_colors[color_index]);
    set_ball_position(x, y);

    usleep(FRAME_TIME_MICROSECONDS); // Sync with VGA refresh (~60fps)
  }

  close(vga_ball_fd);
  printf("VGA ball Userspace program terminating\n");
  return 0;
}