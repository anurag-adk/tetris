#include "headers/TetrisPiece.h"

TetrisPiece::TetrisPiece(int pieceType) : type(pieceType), x(BOARD_WIDTH/2 - 2), y(0) {
    shape = PIECES[pieceType];
}

void TetrisPiece::rotate() {
    std::vector<std::vector<int>> rotated(4, std::vector<int>(4, 0));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            rotated[j][3-i] = shape[i][j];
        }
    }
    shape = rotated;
}
