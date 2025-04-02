#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666  // ~60 FPS

int vga_ball_fd;

void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla = { .background = *c };
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
  }
}

void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla = { .pos_x = x, .pos_y = y };
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
  }
}

int main() {
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t background = { 0x00, 0x00, 0x00 }; // black
  static const vga_ball_color_t bounce_colors[] = {
    { 0xff, 0x00, 0x00 }, // red
    { 0x00, 0xff, 0x00 }, // green
    { 0x00, 0x00, 0xff }, // blue
    { 0xff, 0xff, 0x00 }, // yellow
    { 0x00, 0xff, 0xff }, // cyan
    { 0xff, 0x00, 0xff }, // magenta
    { 0xff, 0xff, 0xff }, // white
  };
  #define NUM_COLORS 7

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    perror("could not open /dev/vga_ball");
    return 1;
  }

  // Set initial state
  set_background_color(&background);
  unsigned short x = BOX_WIDTH / 2, y = BOX_HEIGHT / 2;
  int dx = 1, dy = 1;
  int color_index = 0;

  while (1) {
    // Bounce logic
    if (x + dx >= BOX_WIDTH || x + dx <= 0) {
      dx = -dx;
      color_index = (color_index + 1) % NUM_COLORS;
      set_background_color(&bounce_colors[color_index]); // change on wall bounce
    }
    if (y + dy >= BOX_HEIGHT || y + dy <= 0) {
      dy = -dy;
      color_index = (color_index + 1) % NUM_COLORS;
      set_background_color(&bounce_colors[color_index]);
    }

    x += dx;
    y += dy;

    set_ball_position(x, y);

    // Optional: print every 15 frames to avoid slowing it down
    static int frame = 0;
    if (++frame % 15 == 0)
      printf("Ball position: (%u, %u)\n", x, y);

    usleep(FRAME_TIME_MICROSECONDS);  // ~60 FPS
  }

  close(vga_ball_fd);
  return 0;
}
