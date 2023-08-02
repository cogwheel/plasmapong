#pragma once

#include <algorith>
#include <cassert>
#include <cstdint>

#include "system.hpp"

using std::uint8_t;

#define INDEX_OF(x, y) ((y)*SCREEN_WIDTH + (x))
#define IS_ONSCREEN(x, y) ((x) >= 0 && (x) <= MAX_X && (y) >= 0 && (y) <= MAX_Y)

#define MAX_X (SCREEN_WIDTH - 1)
#define MID_X (SCREEN_WIDTH >> 1)
#define MAX_Y (SCREEN_HEIGHT - 1)
#define MID_Y (SCREEN_HEIGHT >> 1)

#define MAX_COLOR (NUM_COLORS - 1)

#define DIGIT_SPACING (DIGIT_WIDTH + 1)

#define assert_minmax(x, min, max)                                             \
  assert((x) >= (min));                                                        \
  assert((x) <= (max))

#define assert_onscreen(x, y) assert(IS_ONSCREEN(x, y))

template <typename T> inline T clamp(T const val, T const min, T const max) {
  return std::min(max, std::max(min, val));
}

template <typename T> inline uint8_t clamp_color(T const color) {
  return static_cast<uint8_t>(clamp<T>(color, 0, MAX_COLOR_COMPONENT));
}

inline void set_pixel(uint8_t *const buffer, int const x, int const y,
                      uint8_t const color) {
  assert_onscreen(x, y);

  buffer[INDEX_OF(x, y)] = color;
}

inline void set_pixel_clipped(uint8_t *const buffer, int const x, int const y,
                              uint8_t const color) {
  if (IS_ONSCREEN(x, y))
    set_pixel(buffer, x, y, color);
}

void set_pixels(std::uint8_t *const buffer, int const x, int const y,
                std::uint8_t const color, int const size);

void set_pixels_clipped(std::uint8_t *const buffer, int x, int y,
                        std::uint8_t const color, int size);

void line(std::uint8_t *const buffer, int const x1, int const y1, int const x2,
          int const y2, std::uint8_t const color);

void draw_digit(std::uint8_t *buffer, int const x, int const y,
                int const digit);

void draw_number(std::uint8_t *const buffer, int x, int const y, int number);
