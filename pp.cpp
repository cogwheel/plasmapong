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

#include <algorithm>
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

// General
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
#define GAMMA ((float)2.2)

#define DIGIT_WIDTH 5
#define DIGIT_HEIGHT 7
#define DIGIT_SIZE (DIGIT_WIDTH * DIGIT_HEIGHT)
#define DIGIT_SPACING 7

#define MAX_PALETTE_RANGES 8

#define MAX_WEIGHT 12
#define DIM_AMOUNT 0.2

// LUT sizes
#define MAX_RAND_NUMS 1021
// 1021 is prime so it should be hard to stumble upon cycles
#define NUM_ANGLES 256

// Gameplay
#define START_SPEED 2.3

#define COLLISION_THRESHOLD 15

#define PADDLE_MARGIN_HIT 13
#define HALF_PADDLE_HIT 18

#define NEBULA_PARTICLES 25

// UI
#define SCORE_X 10
#define SCORE_Y 10

#define COUNTDOWN_X 154
#define COUNTDOWN_Y 93
#define COUNTDOWN_FRAMES 4

#define PADDLE_MARGIN 10
#define HALF_PADDLE 16

template <typename T> inline T clamp(T val, T min, T max) {
  return val < min ? min : val > max ? max : val;
}

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

int rnd_tbl[MAX_RAND_NUMS];
int next_rnd_index;

struct Nebula {
  float x[NEBULA_PARTICLES];
  float y[NEBULA_PARTICLES];
  uint8_t omega[NEBULA_PARTICLES];
};

double cosTable[NUM_ANGLES], sinTable[NUM_ANGLES];

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

unsigned short target_x[SCREEN_WIDTH];
unsigned short target_y[SCREEN_HEIGHT];

short weighted_averages[MAX_WEIGHT * MAX_COLOR];

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
PaletteDef pal_table[] = {
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

int const NUM_PALETTES = sizeof(pal_table) / sizeof(PaletteDef);

uint8_t *const VGA = (uint8_t *)0xA0000000L; // location of video memory

uint8_t get_mode() {
  REGS regs;
  regs.h.ah = GET_MODE;
  int86(VIDEO_INT, &regs, &regs);
  return regs.h.al;
}

void set_mode(uint8_t mode) {
  REGS regs;

  regs.h.ah = SET_MODE;
  regs.h.al = mode;
  int86(VIDEO_INT, &regs, &regs);
}

struct MouseState {
  int x, y, buttons;
};

void get_mouse_state(MouseState &mouse) {
  REGS regs;
  regs.x.ax = MOUSE_STATUS;
  int86(MOUSE_INT, &regs, &regs);

  // I *think* this magic math normalizes the mouse coordinates to the range
  // of the paddles
  mouse.x = regs.x.cx * 0.420062695924 + 26;
  mouse.y = regs.x.dx * 0.74 + 26;

  mouse.buttons = regs.x.bx;
}

inline void set_pixel(uint8_t *buffer, int x, int y, uint8_t color) {
  buffer[INDEX_OF(x, y)] = color;
}

inline void set_pixels(uint8_t *buffer, int x, int y, uint8_t color, int size) {
  if (size == 0)
    return;
  if (size == 1) {
    set_pixel(buffer, x, y, color);
    return;
  }

  std::memset(buffer + INDEX_OF(x, y), color, size);
}

inline void show_buffer(uint8_t *&front_buffer, uint8_t *&back_buffer) {
  while ((inp(INPUT_STATUS) & VRETRACE))
    ;
  while (!(inp(INPUT_STATUS) & VRETRACE))
    ;

  std::memcpy(VGA, front_buffer, SCREEN_SIZE);
  std::swap(front_buffer, back_buffer);
}

inline void s_pal_entry(uint8_t index, uint8_t red, uint8_t green,
                        uint8_t blue) {
  outp(PALETTE_MASK, 0xff);
  outp(PALETTE_REGISTER_WRITE, index); // tell it what index to use (0-255)
  outp(PALETTE_DATA, red);             // enter the red
  outp(PALETTE_DATA, green);           // green
  outp(PALETTE_DATA, blue);            // blue
}

void line(uint8_t *buffer, int x1, int y1, int x2, int y2, uint8_t color) {
  int dx, dy, xinc, yinc, two_dx, two_dy, x = x1, y = y1, i, error;

  if (y1 == y2) {
    if (x2 < x1)
      std::swap(x1, x2);
    set_pixels(buffer, x1, y1, color, x2 - x1 + 1);
    return;
  }

  dx = (x2 - x1);
  if (dx < 0) {
    dx = -dx;
    xinc = -1;
  } else
    xinc = 1;

  dy = (y2 - y1);
  if (dy < 0) {
    dy = -dy;
    yinc = -1;
  } else
    yinc = 1;

  two_dx = dx + dx;
  two_dy = dy + dy;

  if (dx > dy) {
    error = 0;
    for (i = 0; i < dx; i++) {
      set_pixel(buffer, x, y, color);
      x += xinc;
      error += two_dy;
      if (error > dx) {
        error -= two_dx;
        y += yinc;
      }
    }
  } else {
    error = 0;
    for (i = 0; i < dy; i++) {
      set_pixel(buffer, x, y, color);
      y += yinc;
      error += two_dx;
      if (error > dy) {
        error -= two_dy;
        x += xinc;
      }
    }
  }
}

///--------------main stuff

typedef void (*EffectFunc)(uint8_t *);

void none(uint8_t *) {}

void waves(uint8_t *buffer) {
  int vertices[11];
  for (int i = 0; i <= 10; i++) {
    vertices[i] = get_rnd() % 60 + 60;
  }
  for (int i = 0; i < 10; i++) {
    line(buffer, i * 32, vertices[i], i * 32 + 32, vertices[i + 1], 128);
  }
}

void dots(uint8_t *buffer) {
  for (int i = 0; i < 8; i++) {
    int drop_x = get_rnd() % (SCREEN_WIDTH - 3),
        drop_y = get_rnd() % (SCREEN_HEIGHT - 3);
    // top-mid
    set_pixel(buffer, drop_x + 1, drop_y, MAX_COLOR);

    // middle row
    set_pixels(buffer, drop_x, drop_y + 1, MAX_COLOR, 3);

    // bottom mid
    set_pixel(buffer, drop_x + 1, drop_y + 2, MAX_COLOR);
  }
}

void lines(uint8_t *buffer) {
  line(buffer, get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       get_rnd() % SCREEN_WIDTH, get_rnd() % SCREEN_HEIGHT,
       static_cast<uint8_t>(get_rnd() % NUM_COLORS));
}

EffectFunc const effects[] = {none, dots, lines, waves};

int const NUM_EFFECTS = sizeof(effects) / sizeof(EffectFunc);

inline EffectFunc choose_effect() { return effects[get_rnd() % NUM_EFFECTS]; }

struct GameData {
  float ball_x, ball_y, ball_dx, ball_dy;
  float speed;

  EffectFunc curr_effect;
  int score;

  bool is_noisy;

  Nebula nebula;
};

template <typename T> T clamp_color(T color) {
  return color < 0                     ? 0
         : color > MAX_COLOR_COMPONENT ? MAX_COLOR_COMPONENT
                                       : color;
}

uint8_t assert_color(uint8_t color) {
  assert(color <= MAX_COLOR_COMPONENT);
  return color;
}

void set_palette(PaletteDef const &pal_data, bool &is_noisy) {
  float red_inc, green_inc, blue_inc, difference, working_red, working_green,
      working_blue, red_end, green_end, blue_end;

  // TODO: maybe precalculate the palettes
  for (int i = 0; i <= pal_data.num_ranges; ++i) {
    PaletteRange const &range = pal_data.ranges[i];
    difference = range.last_index - range.first_index;

    working_red = std::pow(range.first_color.r, GAMMA);
    working_green = std::pow(range.first_color.g, GAMMA);
    working_blue = std::pow(range.first_color.b, GAMMA);

    red_end = std::pow(range.last_color.r, GAMMA);
    green_end = std::pow(range.last_color.g, GAMMA);
    blue_end = std::pow(range.last_color.b, GAMMA);

    red_inc = (red_end - working_red) / difference;
    green_inc = (green_end - working_green) / difference;
    blue_inc = (blue_end - working_blue) / difference;

    for (int j = range.first_index; j <= range.last_index; j++) {
      s_pal_entry(
          static_cast<uint8_t>(j),
          static_cast<uint8_t>(clamp_color(std::pow(working_red, 1 / GAMMA))),
          static_cast<uint8_t>(clamp_color(std::pow(working_green, 1 / GAMMA))),
          static_cast<uint8_t>(clamp_color(std::pow(working_blue, 1 / GAMMA))));

      working_red = clamp(working_red + red_inc, 0.f, FLT_MAX);
      working_green = clamp(working_green + green_inc, 0.f, FLT_MAX);
      working_blue = clamp(working_blue + blue_inc, 0.f, FLT_MAX);
    }
  }

  is_noisy = pal_data.is_noisy;
}

void blur(uint8_t *front_buffer, uint8_t *back_buffer, bool is_noisy) {
  /*   int rand_x=0, rand_y=0;
  rand_x=rand()%4-2; rand_y=rand()%4-2; */
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

  for (int i = 0; i < (MAX_WEIGHT * MAX_COLOR); i++) {
    // TODO: DIM_AMOUNT should be subtracted instead of divided maybe?
    weighted_averages[i] = i / (MAX_WEIGHT + DIM_AMOUNT);
  }

  for (int i = 0; i < NUM_ANGLES; i++) {
    double const radians = TAU * i / NUM_ANGLES;
    cosTable[i] = std::cos(radians);
    sinTable[i] = std::sin(radians);
  }

  std::srand(15);
  init_rnd();
}

void init_game(GameData &g) {
  init_rnd();
  set_palette(pal_table[get_rnd() % NUM_PALETTES], g.is_noisy);

  static float const DIAG_START = START_SPEED / std::sqrt(2.0);

  g.ball_x = MID_X;
  g.ball_y = MID_Y;
  g.ball_dx = (get_rnd() % 2) ? DIAG_START : -DIAG_START;
  g.ball_dy = (get_rnd() % 2) ? DIAG_START : -DIAG_START;
  g.speed = START_SPEED;
  g.curr_effect = choose_effect();
  g.score = 0;

  for (int i = 0; i < NEBULA_PARTICLES; i++) {
    g.nebula.x[i] = get_rnd() % 3 - 5;
    g.nebula.y[i] = get_rnd() % 3 - 5;
    // Take advantage of uint underflow to create complementary angles
    g.nebula.omega[i] = static_cast<uint8_t>(get_rnd() % 30 - 15);
  }
}

enum Direction {
  kForward = 1,
  kReverse = -1,
};

void process_hit(GameData &g, float &front_delta, float &front_pos,
                 float const temp_pos, float &side_delta, float const side_pos,
                 float const mouse_pos, Direction const direction) {
  // TODO: use the speed as an actual magnitude
  g.speed += .05;
  front_delta = g.speed * direction;
  // TODO: instead of reseting to previous pos, reflect across the paddle
  front_pos = temp_pos;
  side_delta = (side_pos - mouse_pos) / 4;
  set_palette(pal_table[get_rnd() % NUM_PALETTES], g.is_noisy);
  g.curr_effect = choose_effect();
  g.score++;
}

inline void draw_digit(uint8_t *buffer, int x, int y, int digit) {
  for (int y_loop = 0; y_loop < DIGIT_HEIGHT; y_loop++) {
    std::memcpy(buffer + INDEX_OF(x, y + y_loop), digit_sprites[digit][y_loop],
                DIGIT_WIDTH);
  }
}

void draw_number(uint8_t *buffer, int x, int y, int number) {
  if (number < 10) {
    draw_digit(buffer, x, y, number);
    return;
  }

  int divisor = 10000; // Max 16-bit power of 10
  while (divisor > number) {
    divisor /= 10;
  }
  int offset = 0;
  do {
    int digit = number / divisor;
    draw_digit(buffer, x + offset, y, digit);
    offset += DIGIT_SPACING;
    number %= divisor;
    divisor /= 10;
  } while (divisor > 0);
}

int main() {
  uint8_t old_mode = get_mode();

  uint8_t *front_buffer, *back_buffer;
  init(front_buffer, back_buffer);

  if (get_mode() != VGA_256_COLOR_MODE) {
    std::cerr << "Unable to set 320x200x256 color mode\n";
    std::exit(1);
  }

  GameData g;
  init_game(g);

  MouseState mouse;
  for (get_mouse_state(mouse); mouse.buttons != QUIT; get_mouse_state(mouse)) {
    float prev_x = g.ball_x;
    float prev_y = g.ball_y;

    g.ball_x += g.ball_dx;
    g.ball_y += g.ball_dy;

    // TODO: De-nest these ifs (outer one seems redundant; might be
    // optimization)
    if (g.ball_x > (SCREEN_WIDTH - COLLISION_THRESHOLD) ||
        g.ball_x < COLLISION_THRESHOLD ||
        g.ball_y > (SCREEN_HEIGHT - COLLISION_THRESHOLD) ||
        g.ball_y < COLLISION_THRESHOLD) {
      if (g.ball_x >= SCREEN_WIDTH || g.ball_x < 0 ||
          g.ball_y >= SCREEN_HEIGHT || g.ball_y < 0) {
        for (; g.score > 0; g.score--) {
          for (int frame = 0; frame < COUNTDOWN_FRAMES; ++frame) {
            blur(front_buffer, back_buffer, g.is_noisy);
            draw_number(front_buffer, COUNTDOWN_X, COUNTDOWN_Y, g.score);
            show_buffer(front_buffer, back_buffer);

            // TODO: Use a state machine so this is handled by the outer loop
            get_mouse_state(mouse);
            if (mouse.buttons == QUIT)
              goto quit;
          }
        }
        init_game(g);
      }
      if (g.ball_y > (mouse.y - HALF_PADDLE_HIT) &&
          g.ball_y < (mouse.y + HALF_PADDLE_HIT) &&
          g.ball_x < PADDLE_MARGIN_HIT) {
        // Left paddle hit
        process_hit(g, g.ball_dx, g.ball_x, prev_x, g.ball_dy, g.ball_y,
                    mouse.y, kForward);
      } else if (g.ball_y < (MAX_Y - (mouse.y - HALF_PADDLE_HIT)) &&
                 g.ball_y > (MAX_Y - (mouse.y + HALF_PADDLE_HIT)) &&
                 g.ball_x > (SCREEN_WIDTH - PADDLE_MARGIN_HIT)) {
        // Right paddle hit
        process_hit(g, g.ball_dx, g.ball_x, prev_x, g.ball_dy, g.ball_y,
                    MAX_Y - mouse.y, kReverse);
      } else if (g.ball_x < (MAX_X - (mouse.x - HALF_PADDLE_HIT)) &&
                 g.ball_x > (MAX_X - (mouse.x + HALF_PADDLE_HIT)) &&
                 g.ball_y < PADDLE_MARGIN_HIT) {
        // top paddle hit
        process_hit(g, g.ball_dy, g.ball_y, prev_y, g.ball_dx, g.ball_x,
                    MAX_X - mouse.x, kForward);
      } else if (g.ball_x > (mouse.x - HALF_PADDLE_HIT) &&
                 g.ball_x < (mouse.x + HALF_PADDLE_HIT) &&
                 g.ball_y >= (SCREEN_HEIGHT - PADDLE_MARGIN_HIT)) {
        // bottom paddle hit
        process_hit(g, g.ball_dy, g.ball_y, prev_y, g.ball_dx, g.ball_x,
                    mouse.x, kReverse);
      }
    }

    blur(front_buffer, back_buffer, g.is_noisy);

    g.curr_effect(front_buffer);

    draw_number(front_buffer, SCORE_X, SCORE_Y, g.score);

    // draw paddles

    // TOP
    line(front_buffer, MAX_X - (mouse.x - HALF_PADDLE), PADDLE_MARGIN,
         MAX_X - (mouse.x + HALF_PADDLE), PADDLE_MARGIN, MAX_COLOR);
    // BOTTOM
    line(front_buffer, mouse.x - HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN,
         mouse.x + HALF_PADDLE, SCREEN_HEIGHT - PADDLE_MARGIN, MAX_COLOR);
    // LEFT
    line(front_buffer, PADDLE_MARGIN, mouse.y - HALF_PADDLE, PADDLE_MARGIN,
         mouse.y + HALF_PADDLE, MAX_COLOR);
    // RIGHT
    line(front_buffer, SCREEN_WIDTH - PADDLE_MARGIN,
         MAX_Y - (mouse.y - HALF_PADDLE), SCREEN_WIDTH - PADDLE_MARGIN,
         MAX_Y - (mouse.y + HALF_PADDLE), MAX_COLOR);

    // Draw "nucleus" (i think?)
    for (int i = 0; i < 5; i++) {
      line(front_buffer, (int)g.ball_x + get_rnd() % 6 - 3,
           (int)g.ball_y + get_rnd() % 6 - 3, (int)g.ball_x + get_rnd() % 6 - 3,
           (int)g.ball_y + get_rnd() % 6 - 3, 230);
    }

    for (int i = 0; i < NEBULA_PARTICLES; i++) {

      float orig_x = g.nebula.x[i];

      g.nebula.x[i] = g.nebula.x[i] * cosTable[g.nebula.omega[i]] -
                      g.nebula.y[i] * sinTable[g.nebula.omega[i]];
      g.nebula.y[i] = g.nebula.y[i] * cosTable[g.nebula.omega[i]] +
                      orig_x * sinTable[g.nebula.omega[i]];

      set_pixel(front_buffer, g.nebula.x[i] + g.ball_x,
                g.nebula.y[i] + g.ball_y, MAX_COLOR);
    }

    show_buffer(front_buffer, back_buffer);
  }

quit:
  set_mode(old_mode);

  return 0;
}
