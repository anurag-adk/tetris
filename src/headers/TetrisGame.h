#pragma once
#include <vector>
#include <random>
#include <chrono>
#include "TetrisPiece.h"
#include "Renderer.h"
#include "GameConstants.h"

class TetrisGame {
private:
    std::vector<std::vector<int>> board;
    TetrisPiece currentPiece;
    TetrisPiece nextPiece;
    std::mt19937 rng;
    std::uniform_int_distribution<int> pieceDist;
    double lastFall;
    double fallSpeed;
    int score;
    int lines;
    bool gameOver;
    bool paused;
    bool gameStarted;
    
    Renderer* renderer;

public:
    TetrisGame();
    ~TetrisGame();
    
    void spawnNewPiece();
    void generateNextPiece();
    bool checkCollision(const TetrisPiece& piece, int dx, int dy);
    void placePiece();
    void clearLines();
    void update(double currentTime);
    
    // Movement functions
    void moveLeft();
    void moveRight();
    void rotate();
    void drop();
    void softDrop();
    
    // Game state functions
    void restart();
    void startGame();
    void togglePause();
    
    // Rendering
    void render();
    
    // Getters
    bool isGameOver() const { return gameOver; }
    bool isPaused() const { return paused; }
    bool hasStarted() const { return gameStarted; }
    int getScore() const { return score; }
    int getLines() const { return lines; }
};
