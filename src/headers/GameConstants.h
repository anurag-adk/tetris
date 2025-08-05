#pragma once
#include <vector>

// Game constants
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 20;
const int BLOCK_SIZE = 30;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;
const int BOARD_OFFSET_X = 100;  // Offset from left edge
const int BOARD_OFFSET_Y = 50;   // Offset from bottom edge

// Block Colors
struct Color {
    float r, g, b, a;
    Color(float r = 0, float g = 0, float b = 0, float a = 1) : r(r), g(g), b(b), a(a) {}
};

extern const Color COLORS[];
extern const std::vector<std::vector<std::vector<int>>> PIECES;
