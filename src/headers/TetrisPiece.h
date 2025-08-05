#pragma once
#include <vector>
#include "GameConstants.h"

class TetrisPiece {
public:
    std::vector<std::vector<int>> shape;
    int x, y, type;
    
    TetrisPiece(int pieceType);
    void rotate();
};
