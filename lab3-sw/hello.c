#include <stdio.h>
#include "vga_ball.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define BOX_WIDTH 640
#define BOX_HEIGHT 480
#define FRAME_TIME_MICROSECONDS 16666   // ~60 FPS (1s / 60 ≈ 16.66ms)

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

int main() {
    static const char filename[] = "/dev/vga_ball";

    const vga_ball_color_t background = { 0x00, 0x00, 0x00 }; // black background

    if ((vga_ball_fd = open(filename, O_RDWR)) == -1) {
        perror("Could not open /dev/vga_ball");
        return -1;
    }

    // Set background color once
    set_background_color(&background);

    unsigned short x = BOX_WIDTH / 2;
    unsigned short y = BOX_HEIGHT / 2;
    int dx = 2, dy = 2; // Velocity in x and y

    while (1) {
        // Update position
        x += dx;
        y += dy;

        // Bounce on wall hit (ensure ball stays inside the box)
        if (x <= 0 || x >= BOX_WIDTH - 1) dx = -dx;
        if (y <= 0 || y >= BOX_HEIGHT - 1) dy = -dy;

        set_ball_position(x, y);

        // Optional debug output
        // printf("Ball position: (%u, %u)\n", x, y);

        usleep(FRAME_TIME_MICROSECONDS);
    }

    close(vga_ball_fd);
    return 0;
}
