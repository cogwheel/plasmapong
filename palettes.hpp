#pragma once

#include <cstdint>

#define MAX_PALETTE_RANGES 8

struct PaletteColor {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

// TODO: Remove need to specify both start and end for each entry
struct PaletteRange {
  std::uint8_t first_index;
  std::uint8_t last_index;
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

extern PaletteDef const palettes[];
extern int const NUM_PALETTES;