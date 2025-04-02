#include <stdio.h>
#include "vga_ball.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666   // ~60 FPS

int vga_ball_fd;

void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
  }
}

void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla;
  vla.pos_x = x;
  vla.pos_y = y;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
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

  // Use black background
  static const vga_ball_color_t background = { 0x00, 0x00, 0x00 };

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    perror("could not open /dev/vga_ball");
    return -1;
  }

  set_background_color(&background);

  // Initialize coordinates from driver state
  unsigned short x, y;
  get_ball_position(&x, &y);

  int dx = 1, dy = 1;

  while (1) {
    // Update position
    x += dx;
    y += dy;

    // Bounce
    if (x == 0 || x >= BOX_WIDTH - 1) dx = -dx;
    if (y == 0 || y >= BOX_HEIGHT - 1) dy = -dy;

    // Redraw every frame
    set_ball_position(x, y);

    // For debugging (can be removed to avoid flicker)
    // printf("Ball position: (%u, %u)\n", x, y);

    usleep(FRAME_TIME_MICROSECONDS);
  }

  close(vga_ball_fd);
  return 0;
}
