#pragma once

#include <cstdint>

#define SCREEN_WIDTH 320  // width in pixels of mode 0x13
#define SCREEN_HEIGHT 200 // height in pixels of mode 0x13
#define NUM_COLORS 256    // number of colors in mode 0x13

#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define MAX_COLOR_COMPONENT 63 // RGB components in the palette

#define LMB 1
#define RMB 2

bool set_vga_mode();
void reset_mode();

void show_buffer(std::uint8_t *const front_buffer);

void set_pal_entry(std::uint8_t const index, std::uint8_t const red,
                   std::uint8_t const green, std::uint8_t const blue);

bool has_mouse();

struct MouseState {
  int x;
  int y;
  int buttons;
};

void get_mouse_state(MouseState &mouse);