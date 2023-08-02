/*
PlasmaPong

Copyright © 1998-2023 Matthew Orlando and Forrest Briggs

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <algorith>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <conio.h>
#include <dos.h>

using std::uint8_t;

/*
 * Configuration constants
 *
 * These will likely become settings/config files, command line args, etc.
 */

// Gameplay
#define START_SPEED 1.8
#define SIDE_SPEED_FACTOR (1.0 / 8)

#define COLLISION_THRESHOLD 15

#define PADDLE_MARGIN_HIT 13
#define HALF_PADDLE_HIT 18

#define MAX_RAND_NUMS 1021

// Graphics
#define SCORE_X 10
#define SCORE_Y 10

#define COUNTDOWN_X 154
#define COUNTDOWN_Y 93
#define COUNTDOWN_FRAMES 4

#define PADDLE_MARGIN 10
#define HALF_PADDLE 16

#define NEBULA_PARTICLES 25
#define WAVE_SEGMENTS 10

#define DIM_AMOUNT 0.2
#define GAMMA ((float)2.2)

/*
 * Data constants
 *
 * These are characteristics of the sprite and palette data that will eventually
 * be read from a file
 */

#define DIGIT_WIDTH 5
#define DIGIT_HEIGHT 7
#define DIGIT_SIZE (DIGIT_WIDTH * DIGIT_HEIGHT)
#define DIGIT_SPACING 7

#define MAX_PALETTE_RANGES 8

/*
 * Defined constants
 *
 * Changing these values would require code changes, name changes, or
 * tearing apart the fabric of reality.
 */

#define TAU 6.2831853071795864

// System
#define VIDEO_INT 0x10          // the BIOS video interrupt.
#define WRITE_DOT 0x0C          // BIOS func to plot a pixel.
#define SET_MODE 0x00           // BIOS func to set the video mode.
#define GET_MODE 0x0F           // BIOS func to get the video mode.
#define VGA_256_COLOR_MODE 0x13 // use to set 256-color mode.

#define MOUSE_INT 0x33
#define MOUSE_STATUS 0x3
#define LMB 1
#define RMB 2
#define QUIT (LMB + RMB)

#define DISPLAY_ENABLE 0x01 // VGA input status bits
#define INPUT_STATUS 0x03da
#define VRETRACE 0x08

#define PALETTE_MASK 0x03c6
#define PALETTE_REGISTER_READ 0x03c7
#define PALETTE_REGISTER_WRITE 0x03c8
#define PALETTE_DATA 0x03c9

// Graphics
#define SCREEN_WIDTH 320 // width in pixels of mode 0x13
#define MAX_X (SCREEN_WIDTH - 1)
#define MID_X (SCREEN_WIDTH >> 1)

#define SCREEN_HEIGHT 200 // height in pixels of mode 0x13
#define MAX_Y (SCREEN_HEIGHT - 1)
#define MID_Y (SCREEN_HEIGHT >> 1)

#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

#define INDEX_OF(x, y) ((y)*SCREEN_WIDTH + (x))

#define NUM_COLORS 256 // number of colors in mode 0x13
#define MAX_COLOR (NUM_COLORS - 1)
#define MAX_COLOR_COMPONENT 63 // RGB components in the palette

#define MAX_WEIGHT 12

// Others
#define NUM_ANGLES 256

#define MOUSE_MARGIN ((PADDLE_MARGIN) + (HALF_PADDLE))
#define MOUSE_X_RANGE ((SCREEN_WIDTH)-2 * (MOUSE_MARGIN))
#define MOUSE_Y_RANGE ((SCREEN_HEIGHT)-2 * (MOUSE_MARGIN))

#define MOUSE_X_SCALE (float(MOUSE_X_RANGE) / (SCREEN_WIDTH))
#define MOUSE_Y_SCALE (float(MOUSE_Y_RANGE) / (SCREEN_HEIGHT))

/*
 * Graphics data
 */

// clang-format off
uint8_t const digit_sprites[10][DIGIT_HEIGHT][DIGIT_WIDTH] = {
  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    0, 0, 0, 0, 255,
    0, 0, 255, 255, 0,
    0, 255, 0, 0, 0,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 255 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    0, 0, 0, 0, 255,
    0, 0, 255, 255, 0,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 0, 0, 255, 0,
    255, 255, 255, 255, 255,
    0, 0, 0, 255, 0,
    0, 0, 0, 255, 0 },

  { 255, 255, 255, 255, 255,
    255, 0, 0, 0, 0,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 0,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 0,
    255, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 255, 255, 255, 255, 255,
    0, 0, 0, 0, 255,
    0, 0, 0, 255, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0,
    0, 0, 255, 0, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 },

  { 0, 255, 255, 255, 0,
    255, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 255,
    0, 0, 0, 0, 255,
    255, 0, 0, 0, 255,
    0, 255, 255, 255, 0 }
};
// clang-format on

struct PaletteColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// TODO: Remove need to specify both start and end for each entry
struct PaletteRange {
  uint8_t first_index;
  uint8_t last_index;
  PaletteColor first_color;
  PaletteColor last_color;
};

// TODO: read these from a file
struct PaletteDef {
  // using the old static-sized approach since C++98 doesn't have initializer
  // lists
  int num_ranges;
  bool is_noisy;
  PaletteRange ranges[MAX_PALETTE_RANGES];
};

// clang-format off
static PaletteDef const pal_table[] = {
  { 5, true, {
    { 0, 31,
      {0,   0,   0},
      {0,   0,  63},
    },
    {32,  63,
      {0,   0,  63},
      {0,   0,   0},
    },
    {64,  95,
      {0,   0,   0},
      {63,   0,   0},
    },
    {96, 127,
      {63,   0,   0},
      {0,   0,   0},
    },
    {128, 255,
      {0,   0,   0},
      {63,  63,   0},
    },
  }},
  { 5, true, {
    {0,  31,
      {0,   0,   0},
      {21,   39,  23},
    },
    {32,  63,
      {21,   39,  23},
      {63,   19,   0},
    },
    {64,  95,
      {63,   19,   0},
      {32,   33,   27},
    },
    {96, 127,
      {32,   33,   27},
      {26,   5,   18},
    },
    {128, 255,
      {26,   5,   18},
      {63,  63,   0},
    },
  }},
  { 5, true, {
    {0,  31,
      {0,   0,   0},
      {21,   33,  40},
    },
    {32,  63,
      {21,   33,  40},
      {12,   12,   20},
    },
    {64,  110,
      {12,   12,   20},
      {43,   33,   38},
    },
    {111, 127,
      {43,   33,   38},
      {63,   17,   3},
    },
    {128, 255,
      {63,   17,   3},
      {54,  46,   30},
    },
  }},
  { 5, false, {
    {0,  31,
      {0,   0,   0},
      {57,   57,  63},
    },
    {32,  63,
      {57,   57,  63},
      {0,   0,   0},
    },
    {64,  110,
      {0,   0,   0},
      {63,   63,   63},
    },
    {111, 127,
      {63,   63,   63},
      {0,   0,   0},
    },
    {128, 255,
      {0,   0,   0},
      {63,  63,   63},
    },
  }}
};
// clang-format on

static int const NUM_PALETTES = sizeof(pal_table) / sizeof(PaletteDef);

/*
 * Helpers
 */

#define assert_minmax(x, min, max)                                             \
  assert((x) >= (min));                                                        \
  assert((x) <= (max))

#define assert_onscreen(x, y)                                                  \
  assert_minmax((x), 0, MAX_X);                                                \
  assert_minmax((y), 0, MAX_Y)

template <typename T> inline T clamp(T const val, T const min, T const max) {
  return std::min(max, std::max(min, val));
}

template <typename T> uint8_t clamp_color(T const color) {
  return static_cast<uint8_t>(clamp<T>(color, 0, MAX_COLOR_COMPONENT));
}

/*
 * System interface
 *
 * DOS/PC-specific functionality
 */

static uint8_t *const VGA = (uint8_t *)0xA0000000L; // location of video memory

uint8_t get_mode() {
  REGS regs;
  regs.h.ah = GET_MODE;
  int86(VIDEO_INT, &regs, &regs);
  return regs.h.al;
}

void set_mode(uint8_t const mode) {
  REGS regs;
  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

inline void show_buffer(uint8_t *const front_buffer) {
  while ((inp(INPUT_STATUS) & VRETRACE))
    ;
  while (!(inp(INPUT_STATUS) & VRETRACE))
    ;

  std::memcpy(VGA, front_buffer, SCREEN_SIZE);
}

inline void set_pal_entry(uint8_t const index, uint8_t const red,
                          uint8_t const green, uint8_t const blue) {
  assert(red <= MAX_COLOR_COMPONENT);
  assert(green <= MAX_COLOR_COMPONENT);

  outp(PALETTE_MASK, 0xff);
  outp(PALETTE_REGISTER_WRITE, index); // tell it what index to use (0-255)
  outp(PALETTE_DATA, red);             // enter the red
  outp(PALETTE_DATA, green);           // green
  outp(PALETTE_DATA, blue);            // blue
}

struct MouseState {
  int x;
  int y;
  int buttons;
};

void get_mouse_state(MouseState &mouse) {
  REGS regs;
  regs.x.ax = MOUSE_STATUS;
  int86(MOUSE_INT, &regs, &regs);

  // DOS doubles the X coord at 320 SCREEN_WIDTH
  int const raw_x = regs.x.cx >> 1;
  int const raw_y = regs.x.dx;

  mouse.x = raw_x * MOUSE_X_SCALE + MOUSE_MARGIN;
  mouse.y = raw_y * MOUSE_Y_SCALE + MOUSE_MARGIN;

  assert_onscreen(mouse.x, mouse.y);

  mouse.buttons = regs.x.bx;
}

/*
 * Graphics routines
 */

inline void set_pixel(uint8_t *const buffer, int const x, int const y,
                      uint8_t const color) {
  assert_onscreen(x, y);

  buffer[INDEX_OF(x, y)] = color;
}

inline void set_pixel_clipped(uint8_t *const buffer, int const x, int const y,
                              uint8_t const color) {
  if (x < 0 || x > MAX_X || y < 0 || y > MAX_Y)
    return;

  set_pixel(buffer, x, y, color);
}

inline void set_pixels(uint8_t *const buffer, int const x, int const y,
                       uint8_t const color, int const size) {
  if (size == 0)
    return;

  assert_onscreen(x, y);
  assert_minmax(x + size - 1, 0, MAX_X);

  if (size == 1) {
    set_pixel(buffer, x, y, color);
    return;
  }

  std::memset(buffer + INDEX_OF(x, y), color, size);
}

inline void set_pixels_clipped(uint8_t *const buffer, int x, int y,
                               uint8_t const color, int size) {
  x = clamp(x, 0, MAX_X);
  y = clamp(y, 0, MAX_Y);
  size = clamp(size, 0, MAX_X - x);

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

inline void draw_digit(uint8_t *buffer, int const x, int const y, int const digit) {
  for (int y_loop = 0; y_loop < DIGIT_HEIGHT; y_loop++) {
    std::memcpy(buffer + INDEX_OF(x, y + y_loop), digit_sprites[digit][y_loop],
                DIGIT_WIDTH);
  }
}

void draw_number(uint8_t * const buffer, int x, int const y, int number) {
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

/*
 * Look-up tables
 */

int rnd_tbl[MAX_RAND_NUMS];
int next_rnd_index;

inline int get_rnd() {
  if (++next_rnd_index >= MAX_RAND_NUMS) {
    next_rnd_index = 0;
  }
  return rnd_tbl[next_rnd_index];
}

void init_rnd() {
  next_rnd_index = 0;
  for (int i = 0; i < MAX_RAND_NUMS; i++) {
    rnd_tbl[i] = std::rand();
  }
}

double cos_table[NUM_ANGLES], sin_table[NUM_ANGLES];

void fill_trig_tables() {
  for (int i = 0; i < NUM_ANGLES; i++) {
    double const radians = TAU * i / NUM_ANGLES;
    cos_table[i] = std::cos(radians);
    sin_table[i] = std::sin(radians);
  }
}

unsigned short target_x[SCREEN_WIDTH];
unsigned short target_y[SCREEN_HEIGHT];

void fill_targets() {
  for (int i = 0; i < SCREEN_WIDTH; i++) {
    short target = ((i - MID_X) / 1.03) + MID_X;
    if (target < (MID_X - 1))
      ++target;
    target_x[i] = clamp<short>(target, 0, SCREEN_WIDTH);
  }

  for (int i = 0; i < SCREEN_HEIGHT; i++) {
    short target = (((i - MID_Y) / 1.03) + MID_Y);
    if (i < (MID_Y - 1))
      ++target;
    target_y[i] = SCREEN_WIDTH * clamp<short>(target, 0, SCREEN_HEIGHT);
  }
}

uint8_t weighted_averages[MAX_WEIGHT * MAX_COLOR];

void fill_weighted_averages() {
  for (int i = 0; i < (MAX_WEIGHT * MAX_COLOR); i++) {
    // TODO: DIM_AMOUNT should be subtracted instead of divided maybe?
    weighted_averages[i] = static_cast<uint8_t>(i / (MAX_WEIGHT + DIM_AMOUNT));
  }
}

/*
 * Background effects
 */

typedef void (*EffectFunc)(uint8_t *const);

void none(uint8_t *const) {}

void wave_effect(uint8_t *const buffer) {
  int y1 = get_rnd() % 60 + 60;
  int const dx = SCREEN_WIDTH / WAVE_SEGMENTS;

  for (int i = 0; i <= WAVE_SEGMENTS; i++) {
    int const y2 = get_rnd() % 60 + 60;
    line(buffer, i * dx, y1, i * dx + dx, y2, 128);
    y1 = y2;
  }
}

void dot_effect(uint8_t *const buffer) {
  for (int i = 0; i < 8; i++) {
    int const drop_x = get_rnd() % (SCREEN_WIDTH - 3);
    int const drop_y = get_rnd() % (SCREEN_HEIGHT - 3);

    // top-mid
    set_pixel(buffer, drop_x + 1, drop_y, MAX_COLOR);

    // middle row
    set_pixels(buffer, drop_x, drop_y + 1, MAX_COLOR, 3);

    // bottom mid
    set_pixel(buffer, drop_x + 1, drop_y + 2, MAX_COLOR);
  }
}

void line_effect(uint8_t *const buffer) {
  line(buffer, get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       static_cast<uint8_t>(get_rnd() % NUM_COLORS));
}

static EffectFunc const effects[] = {none, dot_effect, line_effect,
                                     wave_effect};

static int const NUM_EFFECTS = sizeof(effects) / sizeof(EffectFunc);

inline EffectFunc choose_effect() { return effects[get_rnd() % NUM_EFFECTS]; }

void set_palette(PaletteDef const &pal_data, bool &is_noisy) {
  is_noisy = pal_data.is_noisy;

  // TODO: maybe precalculate the palettes
  for (int i = 0; i <= pal_data.num_ranges; ++i) {
    PaletteRange const &range = pal_data.ranges[i];
    float const difference = range.last_index - range.first_index;

    float working_red = std::pow(range.first_color.r, GAMMA);
    float working_green = std::pow(range.first_color.g, GAMMA);
    float working_blue = std::pow(range.first_color.b, GAMMA);

    float const red_end = std::pow(range.last_color.r, GAMMA);
    float const green_end = std::pow(range.last_color.g, GAMMA);
    float const blue_end = std::pow(range.last_color.b, GAMMA);

    float const red_inc = (red_end - working_red) / difference;
    float const green_inc = (green_end - working_green) / difference;
    float const blue_inc = (blue_end - working_blue) / difference;

    for (int j = range.first_index; j <= range.last_index; j++) {
      set_pal_entry(static_cast<uint8_t>(j),
                    clamp_color(std::pow(working_red, 1 / GAMMA)),
                    clamp_color(std::pow(working_green, 1 / GAMMA)),
                    clamp_color(std::pow(working_blue, 1 / GAMMA)));

      working_red = clamp(working_red + red_inc, 0.f, FLT_MAX);
      working_green = clamp(working_green + green_inc, 0.f, FLT_MAX);
      working_blue = clamp(working_blue + blue_inc, 0.f, FLT_MAX);
    }
  }
}

void blur(uint8_t *const front_buffer, uint8_t *const back_buffer,
          bool const is_noisy) {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      int weighted_sum = 0;
      int const index = target_y[y] + target_x[x];

      // Center pixel gets 8x weight
      weighted_sum += (back_buffer[index]) << 2;

      // Top, bottom, left, right get 1x weight
      weighted_sum += back_buffer[index + 1] << 1;
      weighted_sum += back_buffer[index + SCREEN_WIDTH] << 1;

      weighted_sum += back_buffer[index - 1] << 1;
      weighted_sum += back_buffer[index - SCREEN_WIDTH] << 1;

      int target_color = weighted_averages[weighted_sum];
      if (is_noisy)
        target_color = clamp(target_color + get_rnd() % 2 - 1, 0, MAX_COLOR);
      set_pixel(front_buffer, x, y, static_cast<uint8_t>(target_color));
    }
  }
}

/*
 * Gameplay
 */

void init(uint8_t *&front_buffer, uint8_t *&back_buffer) {
  // allocate mem for the front_buffer
  if ((front_buffer = new uint8_t[SCREEN_SIZE]) == NULL) {
    std::cout << "Not enough memory for front buffer.\n";
    std::exit(1);
  }

  if ((back_buffer = new uint8_t[SCREEN_SIZE]) == NULL) {
    std::cout << "Not enough memory for back buffer.\n";
    std::exit(1);
  }

  std::memset(front_buffer, 0, SCREEN_SIZE);
  std::memset(back_buffer, 0, SCREEN_SIZE);

  set_mode(VGA_256_COLOR_MODE);

  if (get_mode() != VGA_256_COLOR_MODE) {
    std::cerr << "Unable to set 320x200x256 color mode\n";
    std::exit(1);
  }

  fill_trig_tables();
  fill_targets();
  fill_weighted_averages();

  std::srand(15);
  init_rnd();
}

struct GameData {
  float ball_x;
  float ball_y;
  float ball_dx;
  float ball_dy;
  float speed;

  EffectFunc curr_effect;
  int score;
  int countdown;

  bool is_noisy;

  struct {
    // distance from center of ball
    float r[NEBULA_PARTICLES];

    // starting angle
    uint8_t phase[NEBULA_PARTICLES];

    // change of angle each frame
    uint8_t sweep[NEBULA_PARTICLES];
  } nebula;
};

void enter_play(GameData &g, MouseState const &) {
  init_rnd();
  set_palette(pal_table[get_rnd() % NUM_PALETTES], g.is_noisy);

  float const DIAG_START = START_SPEED / std::sqrt(2.0);

  g.ball_x = MID_X;
  g.ball_y = MID_Y;
  g.ball_dx = (get_rnd() % 2) ? DIAG_START : -DIAG_START;
  g.ball_dy = (get_rnd() % 2) ? DIAG_START : -DIAG_START;
  g.speed = START_SPEED;
  g.curr_effect = choose_effect();
  g.score = 0;

  for (int i = 0; i < NEBULA_PARTICLES; i++) {
    g.nebula.r[i] = get_rnd() % 4 + 5;
    g.nebula.phase[i] = static_cast<uint8_t>(get_rnd() % NUM_ANGLES);
    // Take advantage of uint underflow to create complementary angles
    g.nebula.sweep[i] = static_cast<uint8_t>(get_rnd() % 30 - 15);
  }
}

enum Direction {
  kForward = 1,
  kReverse = -1,
};

void process_hit(GameData &g, float &front_delta, float &front_pos,
                 int paddle_pos, float &side_delta, float const side_pos,
                 float const mouse_pos, Direction const direction) {
  // TODO: use the speed as an actual magnitude
  g.speed += .05;
  front_delta = g.speed * direction;
  front_pos = paddle_pos + (paddle_pos - front_pos);
  side_delta = g.speed * (side_pos - mouse_pos) * SIDE_SPEED_FACTOR;
  set_palette(pal_table[get_rnd() % NUM_PALETTES], g.is_noisy);
  g.curr_effect = choose_effect();
  g.score++;
}

enum State {
  kPlaying = 0,
  kLosing,
  kLost,
  kNumStates,
};

typedef void (*EnterFn)(GameData &g, MouseState const &mouse);
typedef State (*UpdateFn)(GameData &g, MouseState const &mouse);
typedef void (*RenderFn)(uint8_t *buffer, GameData const &g,
                         MouseState const &mouse);

struct StateEntry {
  EnterFn enter;
  UpdateFn update;
  RenderFn render_back;
  RenderFn render_front;
};

inline void apply_deltas(GameData &g) {
  g.ball_x += g.ball_dx;
  g.ball_y += g.ball_dy;

  for (int i = 0; i < NEBULA_PARTICLES; i++) {
    g.nebula.phase[i] += g.nebula.sweep[i];
  }
}

State update_play(GameData &g, MouseState const &mouse) {
  apply_deltas(g);

  bool is_out = false;

  if (g.ball_x >= (SCREEN_WIDTH - PADDLE_MARGIN_HIT) ||
      g.ball_x < PADDLE_MARGIN_HIT ||
      g.ball_y >= (SCREEN_HEIGHT - PADDLE_MARGIN_HIT) ||
      g.ball_y < PADDLE_MARGIN_HIT) {

    is_out = true;

    if (g.ball_x < PADDLE_MARGIN_HIT &&
        g.ball_y > (mouse.y - HALF_PADDLE_HIT) &&
        g.ball_y < (mouse.y + HALF_PADDLE_HIT)) {
      // Left paddle hit
      process_hit(g, g.ball_dx, g.ball_x, PADDLE_MARGIN_HIT, g.ball_dy,
                  g.ball_y, mouse.y, kForward);
      is_out = false;
    } else if (g.ball_x > (SCREEN_WIDTH - PADDLE_MARGIN_HIT) &&
               g.ball_y < (MAX_Y - (mouse.y - HALF_PADDLE_HIT)) &&
               g.ball_y > (MAX_Y - (mouse.y + HALF_PADDLE_HIT))) {
      // Right paddle hit
      process_hit(g, g.ball_dx, g.ball_x, SCREEN_WIDTH - PADDLE_MARGIN_HIT,
                  g.ball_dy, g.ball_y, MAX_Y - mouse.y, kReverse);
      is_out = false;
    } else if (g.ball_y < PADDLE_MARGIN_HIT &&
               g.ball_x < (MAX_X - (mouse.x - HALF_PADDLE_HIT)) &&
               g.ball_x > (MAX_X - (mouse.x + HALF_PADDLE_HIT))) {
      // top paddle hit
      process_hit(g, g.ball_dy, g.ball_y, PADDLE_MARGIN_HIT, g.ball_dx,
                  g.ball_x, MAX_X - mouse.x, kForward);
      is_out = false;
    } else if (g.ball_y > (SCREEN_HEIGHT - PADDLE_MARGIN_HIT) &&
               g.ball_x > (mouse.x - HALF_PADDLE_HIT) &&
               g.ball_x < (mouse.x + HALF_PADDLE_HIT)) {
      // bottom paddle hit
      process_hit(g, g.ball_dy, g.ball_y, SCREEN_HEIGHT - PADDLE_MARGIN_HIT,
                  g.ball_dx, g.ball_x, mouse.x, kReverse);
      is_out = false;
    }
  }

  return is_out ? kLosing : kPlaying;
}

void render_play_back(uint8_t *buffer, GameData const &g, MouseState const &) {
  g.curr_effect(buffer);
}

void render_play_front(uint8_t *buffer, GameData const &g,
                       MouseState const &mouse) {
  draw_number(buffer, SCORE_X, SCORE_Y, g.score);

  // draw paddles

  // TOP
  line(buffer, MAX_X - (mouse.x - HALF_PADDLE), PADDLE_MARGIN,
       MAX_X - (mouse.x + HALF_PADDLE), PADDLE_MARGIN, MAX_COLOR);
  // BOTTOM
  line(buffer, mouse.x - HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN,
       mouse.x + HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN, MAX_COLOR);
  // LEFT
  line(buffer, PADDLE_MARGIN, mouse.y - HALF_PADDLE, PADDLE_MARGIN,
       mouse.y + HALF_PADDLE, MAX_COLOR);
  // RIGHT
  line(buffer, SCREEN_WIDTH - PADDLE_MARGIN, MAX_Y - (mouse.y - HALF_PADDLE),
       SCREEN_WIDTH - PADDLE_MARGIN, MAX_Y - (mouse.y + HALF_PADDLE),
       MAX_COLOR);

  // Draw "nucleus"
  for (int i = 0; i < 5; i++) {
    line(buffer, (int)g.ball_x + get_rnd() % 6 - 3,
         (int)g.ball_y + get_rnd() % 6 - 3, (int)g.ball_x + get_rnd() % 6 - 3,
         (int)g.ball_y + get_rnd() % 6 - 3, 230);
  }

  // Draw nebula
  for (int i = 0; i < NEBULA_PARTICLES; i++) {
    int const x = g.ball_x + g.nebula.r[i] * cos_table[g.nebula.phase[i]];
    int const y = g.ball_y + g.nebula.r[i] * sin_table[g.nebula.phase[i]];
    set_pixel_clipped(buffer, x, y, MAX_COLOR);
  }
}

State update_losing(GameData &g, MouseState const &) {
  apply_deltas(g);

  if (g.ball_x < -18 || g.ball_x > (MAX_X + 18) || g.ball_y < -18 ||
      g.ball_y > (MAX_Y + 18)) {
    return kLost;
  }

  return kLosing;
}

void enter_lost(GameData &g, MouseState const &) {
  g.countdown = COUNTDOWN_FRAMES;
}

State update_lost(GameData &g, MouseState const &) {
  g.countdown--;
  if (g.countdown == 0) {
    g.score--;
    g.countdown = COUNTDOWN_FRAMES;
  }

  if (g.score < 0) {
    return kPlaying;
  }

  return kLost;
}

void render_lost(uint8_t *buffer, GameData const &g, MouseState const &) {
  draw_number(buffer, COUNTDOWN_X, COUNTDOWN_Y, g.score);
}

static const StateEntry state_table[kNumStates] = {
    {enter_play, update_play, render_play_back, render_play_front}, // kPlaying
    {NULL, update_losing, render_play_back, render_play_front},     // kLosing
    {enter_lost, update_lost, NULL, render_lost},                   // kLost
};

int main() {
  uint8_t const old_mode = get_mode();

  uint8_t *front_buffer, *back_buffer;
  init(front_buffer, back_buffer);

  GameData g;
  MouseState mouse; // TODO: general input state?

  State state = kPlaying;
  enter_play(g, mouse);

  for (get_mouse_state(mouse); mouse.buttons != QUIT; get_mouse_state(mouse)) {
    State const new_state = state_table[state].update(g, mouse);

    if (new_state != state) {
      state = new_state;

      if (state_table[state].enter) {
        state_table[state].enter(g, mouse);
      }
    }

    if (state_table[state].render_back) {
      state_table[state].render_back(back_buffer, g, mouse);
    }

    blur(front_buffer, back_buffer, g.is_noisy);

    state_table[state].render_front(front_buffer, g, mouse);

    show_buffer(front_buffer);
    std::swap(front_buffer, back_buffer);
  }

  set_mode(old_mode);

  return 0;
}
