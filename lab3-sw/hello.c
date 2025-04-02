#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666   // ~60 FPS (VGA sync)
#define FRAME_SKIP 5                    // Update ball every 5 frames
#define SCALE 4                         // Sub-pixel scaling factor
#define BALL_SIZE 10                    // Size of the ball in pixels

int vga_ball_fd;

void set_background_color(const vga_ball_color_t *c) {
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
    return;
  }
}

void set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla;
  vla.pos_x = x;
  vla.pos_y = y;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
    return;
  }
}

int main() {
  static const char filename[] = "/dev/vga_ball";

  static const vga_ball_color_t green = { 0x00, 0xff, 0x00 };  // Green color

  printf("VGA ball Userspace program started\n");

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  // Set background color to green
  set_background_color(&green);

  // Initialize position with sub-pixel precision
  int x = (BOX_WIDTH / 2) * SCALE;
  int y = (BOX_HEIGHT / 2) * SCALE;
  int dx = 1 * SCALE;  // Slower initial velocity (scaled)
  int dy = 1 * SCALE;

  int frame = 0;

  while (1) {
    if (frame % FRAME_SKIP == 0) {
      // Update position with sub-pixel precision
      x += dx;
      y += dy;

      // Bounce off edges (accounting for ball size)
      if ((x / SCALE) <= 0 || (x / SCALE) >= (BOX_WIDTH - BALL_SIZE)) {
        dx = -dx;
        // Ensure we don't get stuck at edges
        if ((x / SCALE) <= 0) x = 0 * SCALE;
        if ((x / SCALE) >= (BOX_WIDTH - BALL_SIZE)) x = (BOX_WIDTH - BALL_SIZE) * SCALE;
      }
      if ((y / SCALE) <= 0 || (y / SCALE) >= (BOX_HEIGHT - BALL_SIZE)) {
        dy = -dy;
        // Ensure we don't get stuck at edges
        if ((y / SCALE) <= 0) y = 0 * SCALE;
        if ((y / SCALE) >= (BOX_HEIGHT - BALL_SIZE)) y = (BOX_HEIGHT - BALL_SIZE) * SCALE;
      }

      // Set the actual pixel position
      set_ball_position(x / SCALE, y / SCALE);
      printf("Ball position: (%u, %u)\n", x / SCALE, y / SCALE);
    }

    // Maintain frame rate
    usleep(FRAME_TIME_MICROSECONDS);
    frame++;
  }

  close(vga_ball_fd);
  printf("VGA ball Userspace program terminating\n");
  return 0;
}
