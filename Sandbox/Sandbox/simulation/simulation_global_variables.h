#pragma once

#include <cstdint>

// Size of a chunk
constexpr uint16_t CHUNKSIZE = 128;
// Amount of chunks in one axis - not the total amount
constexpr uint16_t CHUNKCOUNT = 2;

// Scale to use for rendering, should probably be placed somewhere else
constexpr uint8_t SCALE = 4;