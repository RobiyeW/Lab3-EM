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
#define FRAME_SKIP 15                   // Update ball every 15 frames
#define SCALE 16                        // High scaling for smooth movement
#define BALL_SIZE 20                    // Visible ball size
#define INITIAL_SPEED 1                 // Slow movement speed

int vga_ball_fd;

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

  printf("VGA Ball Demo - Constant Background\n");

  if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
    fprintf(stderr, "could not open %s\n", filename);
    return -1;
  }

  // Set fixed background color (dark gray)
  vga_ball_arg_t bg;
  bg.background.red = 0x30;
  bg.background.green = 0x30;
  bg.background.blue = 0x30;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &bg)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
    return -1;
  }

  // Initialize position with high precision
  int x = (BOX_WIDTH / 3) * SCALE;  // Start at 1/3 of screen width
  int y = (BOX_HEIGHT / 3) * SCALE; // Start at 1/3 of screen height
  int dx = INITIAL_SPEED * SCALE;
  int dy = INITIAL_SPEED * SCALE;

  int frame = 0;

  while (1) {
    if (frame % FRAME_SKIP == 0) {
      // Update position with high precision
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
    }

    // Maintain frame rate
    usleep(FRAME_TIME_MICROSECONDS);
    frame++;
  }

  close(vga_ball_fd);
  return 0;
}