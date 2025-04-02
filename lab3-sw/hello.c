/*
 * Userspace program that communicates with the vga_ball device driver
 * through ioctls to bounce a ball around the screen
 *
 * Updated by ChatGPT for Lab 3 — CSEE 4840
 */

 #include <stdio.h>
 #include <stdint.h>
 #include "vga_ball.h"
 #include <sys/ioctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 
 #define SCREEN_WIDTH  640
 #define SCREEN_HEIGHT 480
 #define BALL_SIZE     8
 
 int vga_ball_fd;
 
 /* Optional: Read and print the background color */
 void print_background_color() {
   vga_ball_arg_t vla;
   if (ioctl(vga_ball_fd, VGA_BALL_READ_BACKGROUND, &vla)) {
     perror("ioctl(VGA_BALL_READ_BACKGROUND) failed");
     return;
   }
   printf("Background: %02x %02x %02x\n",
          vla.background.red, vla.background.green, vla.background.blue);
 }
 
 /* Optional: Set the background color */
 void set_background_color(const vga_ball_color_t *c) {
   vga_ball_arg_t vla;
   vla.background = *c;
   if (ioctl(vga_ball_fd, VGA_BALL_WRITE_BACKGROUND, &vla)) {
     perror("ioctl(VGA_BALL_WRITE_BACKGROUND) failed");
     return;
   }
 }
 
 /* Set the ball position using ioctl */
 void set_ball_position(uint16_t x, uint16_t y) {
   vga_ball_arg_t vla;
   vla.position.x = x;
   vla.position.y = y;
   if (ioctl(vga_ball_fd, VGA_BALL_WRITE_POSITION, &vla)) {
     perror("ioctl(VGA_BALL_WRITE_POSITION) failed");
   }
 }
 
 int main() {
   static const char filename[] = "/dev/vga_ball";
   int x = 100, y = 100;
   int dx = 5, dy = 4;
 
   // Optional: Set background color at the start
   vga_ball_color_t bg_color = {0x00, 0x00, 0x00};  // Black
   printf("VGA ball userspace program started\n");
 
   if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
     perror("Could not open /dev/vga_ball");
     return 1;
   }
 
   set_background_color(&bg_color);
   print_background_color();
 
   for (int i = 0; i < 1000; i++) {
     x += dx;
     y += dy;
 
     if (x <= 0 || x >= (SCREEN_WIDTH - BALL_SIZE)) dx = -dx;
     if (y <= 0 || y >= (SCREEN_HEIGHT - BALL_SIZE)) dy = -dy;
 
     set_ball_position(x, y);
     usleep(16000);  // Roughly 60 FPS
   }
 
   printf("VGA ball userspace program finished\n");
   close(vga_ball_fd);
   return 0;
 }
 