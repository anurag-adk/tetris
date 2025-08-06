#include "headers/TetrisGame.h"
#include <GLFW/glfw3.h>
#include <iostream>

TetrisGame::TetrisGame() : board(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0)),
                          currentPiece(0), nextPiece(0), rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                          pieceDist(0, 6), lastFall(0), fallSpeed(1.0), score(0), lines(0), 
                          gameOver(false), paused(false), gameStarted(false) {
    renderer = new Renderer();
    spawnNewPiece();
    generateNextPiece();
}

TetrisGame::~TetrisGame() {
    delete renderer;
}

void TetrisGame::spawnNewPiece() {
    currentPiece = nextPiece;
    generateNextPiece();
    if (checkCollision(currentPiece, 0, 0)) {
        gameOver = true;
    }
}

void TetrisGame::generateNextPiece() {
    nextPiece = TetrisPiece(pieceDist(rng));
}

bool TetrisGame::checkCollision(const TetrisPiece& piece, int dx, int dy) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (piece.shape[i][j] != 0) {
                int newX = piece.x + j + dx;
                int newY = piece.y + i + dy;
                
                if (newX < 0 || newX >= BOARD_WIDTH || 
                    newY >= BOARD_HEIGHT || 
                    (newY >= 0 && board[newY][newX] != 0)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void TetrisGame::placePiece() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (currentPiece.shape[i][j] != 0) {
                int boardX = currentPiece.x + j;
                int boardY = currentPiece.y + i;
                if (boardY >= 0) {
                    board[boardY][boardX] = currentPiece.shape[i][j];
                }
            }
        }
    }
    clearLines();
    spawnNewPiece();
}

void TetrisGame::clearLines() {
    int linesCleared = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                fullLine = false;
                break;
            }
        }
        
        if (fullLine) {
            board.erase(board.begin() + y);
            board.insert(board.begin(), std::vector<int>(BOARD_WIDTH, 0));
            linesCleared++;
            y++; // Check the same line again
        }
    }
    
    if (linesCleared > 0) {
        lines += linesCleared;
        score += linesCleared * linesCleared * 100; // Bonus for multiple lines
        fallSpeed = std::max(0.1, 1.0 - lines * 0.05); // Increase speed
    }
}

void TetrisGame::update(double currentTime) {
    if (gameOver || paused || !gameStarted) return;
    
    if (currentTime - lastFall > fallSpeed) {
        if (!checkCollision(currentPiece, 0, 1)) {
            currentPiece.y++;
        } else {
            placePiece();
        }
        lastFall = currentTime;
    }
}

void TetrisGame::moveLeft() {
    if (!gameOver && !paused && gameStarted && !checkCollision(currentPiece, -1, 0)) {
        currentPiece.x--;
    }
}

void TetrisGame::moveRight() {
    if (!gameOver && !paused && gameStarted && !checkCollision(currentPiece, 1, 0)) {
        currentPiece.x++;
    }
}

void TetrisGame::rotate() {
    if (!gameOver && !paused && gameStarted) {
        TetrisPiece testPiece = currentPiece;
        testPiece.rotate();
        if (!checkCollision(testPiece, 0, 0)) {
            currentPiece.rotate();
        }
    }
}

void TetrisGame::drop() {
    if (!gameOver && !paused && gameStarted) {
        while (!checkCollision(currentPiece, 0, 1)) {
            currentPiece.y++;
        }
        placePiece();
    }
}

void TetrisGame::softDrop() {
    if (!gameOver && !paused && gameStarted && !checkCollision(currentPiece, 0, 1)) {
        currentPiece.y++;
        score += 1; // Small bonus for soft drop
    }
}

void TetrisGame::restart() {
    board = std::vector<std::vector<int>>(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0));
    score = 0;
    lines = 0;
    fallSpeed = 1.0;
    gameOver = false;
    paused = false;
    gameStarted = true;
    lastFall = glfwGetTime();
    spawnNewPiece();
    generateNextPiece();
}

void TetrisGame::startGame() {
    gameStarted = true;
    lastFall = glfwGetTime();
}

void TetrisGame::togglePause() {
    if (!gameOver) {
        paused = !paused;
        if (!paused) {
            // Resume the timer to prevent instant drop when unpausing
            lastFall = glfwGetTime();
        }
    }
}

void TetrisGame::render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Darker background
    
    // Draw the game board
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] != 0) {
                renderer->drawBlock(x, y, COLORS[board[y][x]]);
            }
        }
    }
    
    // Draw the current piece
    if (!gameOver) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece.shape[i][j] != 0) {
                    int drawX = currentPiece.x + j;
                    int drawY = currentPiece.y + i;
                    if (drawX >= 0 && drawX < BOARD_WIDTH && drawY >= 0 && drawY < BOARD_HEIGHT) {
                        renderer->drawBlock(drawX, drawY, COLORS[currentPiece.shape[i][j]]);
                    }
                }
            }
        }
    }
    
    // Draw game board border
    Color borderColor(0.7f, 0.7f, 0.7f, 1.0f);
    int borderThickness = 3;
    // Use UI shader for borders
    renderer->drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y - borderThickness, borderThickness, BOARD_HEIGHT * BLOCK_SIZE + 2 * borderThickness, borderColor);
    renderer->drawRect(BOARD_OFFSET_X + BOARD_WIDTH * BLOCK_SIZE, BOARD_OFFSET_Y - borderThickness, borderThickness, BOARD_HEIGHT * BLOCK_SIZE + 2 * borderThickness, borderColor);
    renderer->drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y - borderThickness, BOARD_WIDTH * BLOCK_SIZE + 2 * borderThickness, borderThickness, borderColor);
    renderer->drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y + BOARD_HEIGHT * BLOCK_SIZE, BOARD_WIDTH * BLOCK_SIZE + 2 * borderThickness, borderThickness, borderColor);
    
    // UI Panel settings
    float panelX = BOARD_OFFSET_X + BOARD_WIDTH * BLOCK_SIZE + 20;
    float panelWidth = 180;
    Color panelBorder(1.0f, 1.0f, 1.0f, 1.0f); // White border only
    Color textColor(1.0f, 1.0f, 1.0f, 1.0f); // White text
    Color numberColor(1.0f, 1.0f, 1.0f, 1.0f); // White numbers

    // Next piece panel
    float nextPanelY = BOARD_OFFSET_Y + BOARD_HEIGHT * BLOCK_SIZE - 120;
    float nextPanelHeight = 100;

    // Draw only border (no fill) for UI panels
    renderer->drawRect(panelX, nextPanelY, panelWidth, 3, panelBorder); // Top
    renderer->drawRect(panelX, nextPanelY + nextPanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
    renderer->drawRect(panelX, nextPanelY, 3, nextPanelHeight, panelBorder); // Left
    renderer->drawRect(panelX + panelWidth - 3, nextPanelY, 3, nextPanelHeight, panelBorder); // Right

    // Draw "NEXT" title
    renderer->drawText("NEXT", panelX + 10, nextPanelY + nextPanelHeight - 30, 18, textColor);

    // Draw next piece preview (use block shader for blocks)
    float previewX = panelX + 60;
    float previewY = nextPanelY + 4;
    int previewSize = 18;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (nextPiece.shape[i][j] != 0) {
                float blockX = previewX + j * previewSize;
                float blockY = previewY + (3 - i) * previewSize;
                glUseProgram(renderer->getBlockShaderProgram());
                GLint offsetLoc = glGetUniformLocation(renderer->getBlockShaderProgram(), "offset");
                glUniform2f(offsetLoc, blockX, blockY);
                GLint scaleLoc = glGetUniformLocation(renderer->getBlockShaderProgram(), "scale");
                glUniform2f(scaleLoc, previewSize - 2, previewSize - 2);
                GLint colorLoc = glGetUniformLocation(renderer->getBlockShaderProgram(), "color");
                Color previewColor = COLORS[nextPiece.shape[i][j]];
                glUniform4f(colorLoc, previewColor.r, previewColor.g, previewColor.b, previewColor.a);
                glBindVertexArray(renderer->getVAO());
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }

    // Score panel
    float scorePanelY = nextPanelY - 110;
    float scorePanelHeight = 70;

    renderer->drawRect(panelX, scorePanelY, panelWidth, 3, panelBorder); // Top
    renderer->drawRect(panelX, scorePanelY + scorePanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
    renderer->drawRect(panelX, scorePanelY, 3, scorePanelHeight, panelBorder); // Left
    renderer->drawRect(panelX + panelWidth - 3, scorePanelY, 3, scorePanelHeight, panelBorder); // Right

    renderer->drawText("SCORE", panelX + 10, scorePanelY + scorePanelHeight - 28, 18, textColor);
    renderer->drawNumber(score, panelX + 20, scorePanelY + 12, 22, numberColor);

    // Lines panel
    float linesPanelY = scorePanelY - 90;
    float linesPanelHeight = 70;

    renderer->drawRect(panelX, linesPanelY, panelWidth, 3, panelBorder); // Top
    renderer->drawRect(panelX, linesPanelY + linesPanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
    renderer->drawRect(panelX, linesPanelY, 3, linesPanelHeight, panelBorder); // Left
    renderer->drawRect(panelX + panelWidth - 3, linesPanelY, 3, linesPanelHeight, panelBorder); // Right

    renderer->drawText("LINES", panelX + 10, linesPanelY + linesPanelHeight - 28, 18, textColor);
    renderer->drawNumber(lines, panelX + 20, linesPanelY + 12, 22, numberColor);
    
    // Show start screen if game hasn't started
    if (!gameStarted) {
        Color overlayColor(0.0f, 0.0f, 0.0f, 0.8f);
        renderer->drawRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlayColor);
        
        Color titleColor(1.0f, 1.0f, 1.0f, 1.0f);
        renderer->drawText("TETRIS", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 50, 40, titleColor);
        
        Color startColor(1.0f, 1.0f, 0.0f, 1.0f);
        renderer->drawText("PRESS SPACE TO START", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 25, 18, startColor);
    }
    
    // Show pause indicator if game is paused
    if (paused && gameStarted) {
        Color overlayColor(0.0f, 0.0f, 0.0f, 0.7f);
        renderer->drawRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlayColor);
        
        Color pauseColor(1.0f, 1.0f, 0.0f, 1.0f);
        renderer->drawText("PAUSED", WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 + 20, 25, pauseColor);
        
        Color resumeColor(0.9f, 0.9f, 0.9f, 1.0f);
        renderer->drawText("PRESS SPACE TO RESUME", WINDOW_WIDTH / 2 - 160, WINDOW_HEIGHT / 2 - 30, 18, resumeColor);
    }
    
    // Show game over screen
    if (gameOver) {
        Color overlayColor(0.0f, 0.0f, 0.0f, 0.8f);
        renderer->drawRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlayColor);
        
        Color gameOverColor(1.0f, 0.0f, 0.0f, 1.0f);
        renderer->drawText("GAME OVER", WINDOW_WIDTH / 2 - 90, WINDOW_HEIGHT / 2 + 50, 25, gameOverColor);
        
        Color scoreColor(1.0f, 1.0f, 1.0f, 1.0f);
        renderer->drawText("SCORE:", WINDOW_WIDTH / 2 - 75, WINDOW_HEIGHT / 2, 20, scoreColor);
        renderer->drawNumber(score, WINDOW_WIDTH / 2 + 20, WINDOW_HEIGHT / 2, 20, scoreColor);

        renderer->drawText("LINES CLEARED:", WINDOW_WIDTH / 2 - 125, WINDOW_HEIGHT / 2 - 50, 20, scoreColor);
        renderer->drawNumber(lines, WINDOW_WIDTH / 2 + 115, WINDOW_HEIGHT / 2 - 50, 20, scoreColor);
        
        Color restartColor(1.0f, 1.0f, 0.0f, 1.0f);
        renderer->drawText("PRESS R TO RESTART", WINDOW_WIDTH / 2 - 140, WINDOW_HEIGHT / 2 - 100, 18, restartColor);
    }
}
