#include "drawing.hpp"

#include <cstring>

#include "sprites.hpp"

using std::uint8_t;

void set_pixels(uint8_t *const buffer, int const x, int const y,
                uint8_t const color, int const size) {
  assert_onscreen(x, y);
  assert_minmax(size, 0, MAX_X - x + 1);

  if (size > 1) {
    std::memset(buffer + INDEX_OF(x, y), color, size);
  } else if (size == 1) {
    // TODO: on old hardware, it might be better to unroll this for small sizes
    // > 1
    set_pixel(buffer, x, y, color);
  }
}

void set_pixels_clipped(uint8_t *const buffer, int x, int y,
                        uint8_t const color, int size) {
  x = clamp(x, 0, MAX_X);
  y = clamp(y, 0, MAX_Y);
  size = clamp(size, 0, MAX_X - x + 1);

  set_pixels(buffer, x, y, color, size);
}

void line(uint8_t *const buffer, int const x1, int const y1, int const x2,
          int const y2, uint8_t const color) {
  int x = x1;
  int y = y1;

  if (y1 == y2) {
    if (x1 > x2)
      x = x2;
    set_pixels_clipped(buffer, x, y, color, std::abs(x2 - x1) + 1);
    return;
  }

  int const xinc = (x1 > x2) ? -1 : 1;
  int const dx = std::abs(x2 - x1);

  int const yinc = (y1 > y2) ? -1 : 1;
  int const dy = std::abs(y2 - y1);

  int const two_dx = dx + dx;
  int const two_dy = dy + dy;

  int error = 0;

  if (dx > dy) {
    error = 0;
    for (int i = 0; i < dx; i++) {
      set_pixel_clipped(buffer, x, y, color);
      x += xinc;
      error += two_dy;
      if (error > dx) {
        error -= two_dx;
        y += yinc;
      }
    }
  } else {
    error = 0;
    for (int i = 0; i < dy; i++) {
      set_pixel_clipped(buffer, x, y, color);
      y += yinc;
      error += two_dx;
      if (error > dy) {
        error -= two_dy;
        x += xinc;
      }
    }
  }
}

void draw_digit(uint8_t *buffer, int const x, int const y, int const digit) {
  for (int y_loop = 0; y_loop < DIGIT_HEIGHT; y_loop++) {
    std::memcpy(buffer + INDEX_OF(x, y + y_loop), digit_sprites[digit][y_loop],
                DIGIT_WIDTH);
  }
}

void draw_number(uint8_t *const buffer, int x, int const y, int number) {
  if (number < 0) {
    // TODO: ascii table (maybe I should use cogp47?)
    int const dash_y = y + (DIGIT_HEIGHT >> 1);
    line(buffer, x, dash_y, x + 4, dash_y, MAX_COLOR);
    number *= -1;
    x += 5;
  }

  if (number < 10) {
    draw_digit(buffer, x, y, number);
    return;
  }

  int divisor = 10000; // Max 16-bit power of 10
  while (divisor > number) {
    divisor /= 10;
  }
  do {
    draw_digit(buffer, x, y, number / divisor);
    x += DIGIT_SPACING;
    number %= divisor;
    divisor /= 10;
  } while (divisor > 0);
}
