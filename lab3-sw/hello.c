#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666   // ~60 FPS
#define BALL_RADIUS 10

int vga_ball_fd;

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

void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla;
  vla.background = *c;
  ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla);
}

void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla;
  vla.pos_x = x;
  vla.pos_y = y;
  ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla);
}

int main() {
  static const char filename[] = "/dev/vga_ball";

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  // Ball initial state
  unsigned short x = BOX_WIDTH / 2;
  unsigned short y = BOX_HEIGHT / 2;
  int dx = 2, dy = 2; // QR code style constant speed
  int colorIndex = 0;

  set_background_color(&colors[colorIndex]);
  set_ball_position(x, y);

  while (1) {
    // Bounce detection (accounting for circle radius)
    if (x <= BALL_RADIUS || x >= BOX_WIDTH - BALL_RADIUS) {
      dx = -dx;
      colorIndex = (colorIndex + 1) % COLORS;
      set_background_color(&colors[colorIndex]);
    }

    if (y <= BALL_RADIUS || y >= BOX_HEIGHT - BALL_RADIUS) {
      dy = -dy;
      colorIndex = (colorIndex + 1) % COLORS;
      set_background_color(&colors[colorIndex]);
    }

    x += dx;
    y += dy;

    set_ball_position(x, y);

    usleep(FRAME_TIME_MICROSECONDS);
  }

  close(vga_ball_fd);
  return 0;
}
