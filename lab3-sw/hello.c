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
#define FRAME_TIME_MICROSECONDS 166666   // ~60 FPS (VGA sync)
#define FRAME_SKIP 1                    // Update ball every 5 frames

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
  vga_ball_arg_t vla;
  vla.background = *c;
  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
    return;
  }
}

// void set_ball_position(unsigned short x, unsigned short y) {
//   vga_ball_arg_t vla;
//   vla.pos_x = x;
//   vla.pos_y = y;
//   if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
//     perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
//     return;
//   }
// }

int set_ball_position(unsigned short x, unsigned short y) {
  vga_ball_arg_t vla;

  // Clamp to valid VGA coordinates (adjust to driver's max)
  vla.pos_x = (x < BOX_WIDTH) ? x : BOX_WIDTH - 1;
  vla.pos_y = (y < BOX_HEIGHT) ? y : BOX_HEIGHT - 1;

  if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
    perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
    return -1;
  }
  return 0;
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

  unsigned short x = BOX_WIDTH / 2;
  unsigned short y = BOX_HEIGHT / 2;
  int dx = 1, dy = 1;

  int frame = 0;

  // while (1) {
  //   // Only update background and ball position every FRAME_SKIP frames
  //   if (frame % FRAME_SKIP == 0) {
  //     set_background_color(&colors[(frame / FRAME_SKIP) % COLORS]);


  //     x += dx;
  //     y += dy;

  //     // Bounce off edges
  //     if (x == 0 || x >= BOX_WIDTH - 1) dx = -dx;
  //     if (y == 0 || y >= BOX_HEIGHT - 1) dy = -dy;


  //     set_ball_position(x, y);
  //     // get_ball_position(&x, &y);
  //     printf("Ball position: (%u, %u)\n", x, y);
  //   }

  //   // Sleep to maintain ~60 FPS

  //   frame++;
  // }

  while (1) {
    // Only update background and ball position every FRAME_SKIP frames
    if (frame % FRAME_SKIP == 0) {
      set_background_color(&colors[(frame / FRAME_SKIP) % COLORS]);
  
      x += dx;
      y += dy;
  
      // Bounce off edges (fixed condition)
      if (x == 0 || x >= BOX_WIDTH) dx = -dx;
      if (y == 0 || y >= BOX_HEIGHT) dy = -dy;
  
      set_ball_position(x, y);
      printf("Ball position: (%u, %u)\n", x, y);
    }
  
    // Sleep to maintain ~60 FPS (ADD THIS)
    usleep(FRAME_TIME_MICROSECONDS);
    frame++;
  }

  close(vga_ball_fd);
  return 0;
}