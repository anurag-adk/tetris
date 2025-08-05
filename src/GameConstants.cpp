#include "headers/GameConstants.h"
#include <vector>

const Color COLORS[] = {
    Color(0.0f, 0.0f, 0.0f, 1.0f),     // Empty (black)
    Color(0.0f, 0.9f, 0.9f, 1.0f),     // I-piece (cyan)
    Color(0.0f, 0.0f, 0.9f, 1.0f),     // O-piece (blue)
    Color(0.6f, 0.0f, 0.9f, 1.0f),     // T-piece (purple)
    Color(0.0f, 0.9f, 0.0f, 1.0f),     // S-piece (green)
    Color(0.9f, 0.0f, 0.0f, 1.0f),     // Z-piece (red)
    Color(0.9f, 0.5f, 0.0f, 1.0f),     // J-piece (orange)
    Color(0.9f, 0.9f, 0.0f, 1.0f)      // L-piece (yellow)
};

// Tetris piece shapes
const std::vector<std::vector<std::vector<int>>> PIECES = {
    // I-piece
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    // O-piece
    {
        {0,0,0,0},
        {0,2,2,0},
        {0,2,2,0},
        {0,0,0,0}
    },
    // T-piece
    {
        {0,0,0,0},
        {0,3,0,0},
        {3,3,3,0},
        {0,0,0,0}
    },
    // S-piece
    {
        {0,0,0,0},
        {0,4,4,0},
        {4,4,0,0},
        {0,0,0,0}
    },
    // Z-piece
    {
        {0,0,0,0},
        {5,5,0,0},
        {0,5,5,0},
        {0,0,0,0}
    },
    // J-piece
    {
        {0,0,0,0},
        {6,0,0,0},
        {6,6,6,0},
        {0,0,0,0}
    },
    // L-piece
    {
        {0,0,0,0},
        {0,0,7,0},
        {7,7,7,0},
        {0,0,0,0}
    }
};
