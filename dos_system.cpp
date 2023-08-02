#include "system.hpp"

#include <cassert>
#include <cstring>

#include <conio.h>
#include <dos.h>

using std::uint8_t;

// System
#define VIDEO_INT 0x10          // the BIOS video interrupt.
#define WRITE_DOT 0x0C          // BIOS func to plot a pixel.
#define SET_MODE 0x00           // BIOS func to set the video mode.
#define GET_MODE 0x0F           // BIOS func to get the video mode.
#define VGA_256_COLOR_MODE 0x13 // use to set 256-color mode.

#define MOUSE_INT 0x33
#define MOUSE_SETUP 0x0
#define MOUSE_STATUS 0x3

#define DISPLAY_ENABLE 0x01 // VGA input status bits
#define INPUT_STATUS 0x03da
#define VRETRACE 0x08

#define PALETTE_MASK 0x03c6
#define PALETTE_REGISTER_READ 0x03c7
#define PALETTE_REGISTER_WRITE 0x03c8
#define PALETTE_DATA 0x03c9

static uint8_t *const VGA = (uint8_t *)0xA0000000L; // location of video memory

inline uint8_t get_mode() {
  REGS regs;
  regs.h.ah = GET_MODE;
  int86(VIDEO_INT, &regs, &regs);
  return regs.h.al;
}

inline void set_mode(uint8_t const mode) {
  REGS regs;
  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

static uint8_t g_orig_mode = 0xFF;

bool set_vga_mode() {
  uint8_t cur_mode = get_mode();
  if (cur_mode == VGA_256_COLOR_MODE)
    return true;

  g_orig_mode = cur_mode;

  set_mode(VGA_256_COLOR_MODE);
  return get_mode() == VGA_256_COLOR_MODE;
}

void reset_mode() {
  if (g_orig_mode != 0xFF) {
    set_mode(g_orig_mode);
  }
}

void show_buffer(uint8_t *const front_buffer) {
  while ((inp(INPUT_STATUS) & VRETRACE))
    ;
  while (!(inp(INPUT_STATUS) & VRETRACE))
    ;

  std::memcpy(VGA, front_buffer, SCREEN_SIZE);
}

void set_pal_entry(uint8_t const index, uint8_t const red, uint8_t const green,
                   uint8_t const blue) {
  assert(red <= MAX_COLOR_COMPONENT);
  assert(green <= MAX_COLOR_COMPONENT);

  outp(PALETTE_MASK, 0xff);
  outp(PALETTE_REGISTER_WRITE, index); // tell it what index to use (0-255)
  outp(PALETTE_DATA, red);             // enter the red
  outp(PALETTE_DATA, green);           // green
  outp(PALETTE_DATA, blue);            // blue
}

bool has_mouse() {
  REGS regs;
  regs.x.ax = MOUSE_SETUP;
  int86(MOUSE_INT, &regs, &regs);
  return static_cast<bool>(regs.x.ax);
}

void get_mouse_state(MouseState &mouse) {
  REGS regs;
  regs.x.ax = MOUSE_STATUS;
  int86(MOUSE_INT, &regs, &regs);

  // DOS doubles the X coord at 320 SCREEN_WIDTH
  mouse.x = regs.x.cx >> 1;
  mouse.y = regs.x.dx;
  mouse.buttons = regs.x.bx;
}