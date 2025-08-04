#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <array>
#include <cmath>

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

class TetrisPiece {
public:
    std::vector<std::vector<int>> shape;
    int x, y, type;
    
    TetrisPiece(int pieceType) : type(pieceType), x(BOARD_WIDTH/2 - 2), y(0) {
        shape = PIECES[pieceType];
    }
    
    void rotate() {
        std::vector<std::vector<int>> rotated(4, std::vector<int>(4, 0));
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                rotated[j][3-i] = shape[i][j];
            }
        }
        shape = rotated;
    }
};

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

    GLuint blockShaderProgram; // bevel effect for blocks
    GLuint uiShaderProgram;    // plain color for UI
    GLuint VAO, VBO;
    
public:
    TetrisGame() : board(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0)),
                   currentPiece(0), nextPiece(0), rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                   pieceDist(0, 6), lastFall(0), fallSpeed(1.0), score(0), lines(0), gameOver(false) {
        spawnNewPiece();
        generateNextPiece();
        initOpenGL();
    }
    
    void initOpenGL() {
        // Vertex shader source (shared)
        const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            uniform mat4 projection;
            uniform vec2 offset;
            uniform vec2 scale;
            out vec2 fragCoord;
            void main() {
                fragCoord = aPos;
                vec2 pos = aPos * scale + offset;
                gl_Position = projection * vec4(pos, 0.0, 1.0);
            }
        )";

        // Fragment shader source with bevel effect (for blocks)
        const char* blockFragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 color;
            in vec2 fragCoord;
            void main() {
                vec2 pos = fragCoord;
                float bevelWidth = 0.15;
                float highlightIntensity = 1.4;
                float shadowIntensity = 0.6;
                vec4 finalColor = color;
                if (pos.y > 1.0 - bevelWidth || pos.x < bevelWidth) {
                    finalColor.rgb = min(finalColor.rgb * highlightIntensity, vec3(1.0));
                } else if (pos.y < bevelWidth || pos.x > 1.0 - bevelWidth) {
                    finalColor.rgb *= shadowIntensity;
                }
                float dist = min(min(pos.x, 1.0 - pos.x), min(pos.y, 1.0 - pos.y));
                float glow = smoothstep(0.0, 0.3, dist);
                finalColor.rgb *= (0.9 + 0.1 * glow);
                FragColor = finalColor;
            }
        )";

        // Fragment shader source for UI (plain color)
        const char* uiFragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 color;
            void main() {
                FragColor = color;
            }
        )";

        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        // Compile block fragment shader
        GLuint blockFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(blockFragmentShader, 1, &blockFragmentShaderSource, NULL);
        glCompileShader(blockFragmentShader);

        // Compile UI fragment shader
        GLuint uiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(uiFragmentShader, 1, &uiFragmentShaderSource, NULL);
        glCompileShader(uiFragmentShader);

        // Create block shader program
        blockShaderProgram = glCreateProgram();
        glAttachShader(blockShaderProgram, vertexShader);
        glAttachShader(blockShaderProgram, blockFragmentShader);
        glLinkProgram(blockShaderProgram);

        // Create UI shader program
        uiShaderProgram = glCreateProgram();
        glAttachShader(uiShaderProgram, vertexShader);
        glAttachShader(uiShaderProgram, uiFragmentShader);
        glLinkProgram(uiShaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(blockFragmentShader);
        glDeleteShader(uiFragmentShader);

        // Set up vertex data for a quad
        float vertices[] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };
        GLuint indices[] = { 0, 1, 2, 2, 3, 0 };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        GLuint EBO;
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Set up projection matrix for both shaders
        float projection[16] = {
            2.0f/WINDOW_WIDTH, 0, 0, 0,
            0, 2.0f/WINDOW_HEIGHT, 0, 0,
            0, 0, -1, 0,
            -1, -1, 0, 1
        };
        GLint projLocBlock = glGetUniformLocation(blockShaderProgram, "projection");
        glUseProgram(blockShaderProgram);
        glUniformMatrix4fv(projLocBlock, 1, GL_FALSE, projection);
        GLint projLocUI = glGetUniformLocation(uiShaderProgram, "projection");
        glUseProgram(uiShaderProgram);
        glUniformMatrix4fv(projLocUI, 1, GL_FALSE, projection);
    }
    
    void spawnNewPiece() {
        currentPiece = nextPiece;
        generateNextPiece();
        if (checkCollision(currentPiece, 0, 0)) {
            gameOver = true;
        }
    }
    
    void generateNextPiece() {
        nextPiece = TetrisPiece(pieceDist(rng));
    }
    
    bool checkCollision(const TetrisPiece& piece, int dx, int dy) {
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
    
    void placePiece() {
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
    
    void clearLines() {
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
    
    void update(double currentTime) {
        if (gameOver) return;
        
        if (currentTime - lastFall > fallSpeed) {
            if (!checkCollision(currentPiece, 0, 1)) {
                currentPiece.y++;
            } else {
                placePiece();
            }
            lastFall = currentTime;
        }
    }
    
    void moveLeft() {
        if (!gameOver && !checkCollision(currentPiece, -1, 0)) {
            currentPiece.x--;
        }
    }
    
    void moveRight() {
        if (!gameOver && !checkCollision(currentPiece, 1, 0)) {
            currentPiece.x++;
        }
    }
    
    void rotate() {
        if (!gameOver) {
            TetrisPiece testPiece = currentPiece;
            testPiece.rotate();
            if (!checkCollision(testPiece, 0, 0)) {
                currentPiece.rotate();
            }
        }
    }
    
    void drop() {
        if (!gameOver) {
            while (!checkCollision(currentPiece, 0, 1)) {
                currentPiece.y++;
            }
            placePiece();
        }
    }
    
    void softDrop() {
        if (!gameOver && !checkCollision(currentPiece, 0, 1)) {
            currentPiece.y++;
            score += 1; // Small bonus for soft drop
        }
    }
    
    void restart() {
        board = std::vector<std::vector<int>>(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0));
        score = 0;
        lines = 0;
        fallSpeed = 1.0;
        gameOver = false;
        lastFall = glfwGetTime();
        spawnNewPiece();
        generateNextPiece();
    }
    
    void drawRect(float x, float y, float width, float height, const Color& color) {
        // Default: use UI shader (plain color)
        glUseProgram(uiShaderProgram);
        GLint offsetLoc = glGetUniformLocation(uiShaderProgram, "offset");
        glUniform2f(offsetLoc, x, y);
        GLint scaleLoc = glGetUniformLocation(uiShaderProgram, "scale");
        glUniform2f(scaleLoc, width, height);
        GLint colorLoc = glGetUniformLocation(uiShaderProgram, "color");
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    void drawText(const std::string& text, float x, float y, float size, const Color& color) {
        // Pixel-perfect bitmap font rendering like classic Tetris
        float charWidth = size * 0.7f;
        float charHeight = size;
        float spacing = charWidth + 3;
        float strokeWidth = 3;
        
        for (size_t i = 0; i < text.length(); i++) {
            char c = text[i];
            float charX = x + i * spacing;
            
            // Draw pixelated characters with clean, readable design
            switch (c) {
                case 'N':
                    drawRect(charX, y, strokeWidth, charHeight, color); // left
                    drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight, color); // right
                    drawRect(charX + strokeWidth, y + charHeight * 0.6f, charWidth - 2*strokeWidth, strokeWidth, color); // diagonal bar
                    break;
                case 'E':
                    drawRect(charX, y, strokeWidth, charHeight, color); // left
                    drawRect(charX, y, charWidth, strokeWidth, color); // bottom
                    drawRect(charX, y + charHeight/2 - strokeWidth/2, charWidth * 0.75f, strokeWidth, color); // middle
                    drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                    break;
                case 'X':
                    drawRect(charX + strokeWidth, y + strokeWidth, charWidth - 2*strokeWidth, strokeWidth, color); // top diagonal
                    drawRect(charX + strokeWidth, y + charHeight - 2*strokeWidth, charWidth - 2*strokeWidth, strokeWidth, color); // bottom diagonal
                    drawRect(charX + charWidth/2 - strokeWidth/2, y + charHeight/2 - strokeWidth/2, strokeWidth, strokeWidth, color); // center
                    drawRect(charX, y, strokeWidth, strokeWidth * 2, color); // top left
                    drawRect(charX + charWidth - strokeWidth, y, strokeWidth, strokeWidth * 2, color); // top right
                    drawRect(charX, y + charHeight - strokeWidth * 2, strokeWidth, strokeWidth * 2, color); // bottom left
                    drawRect(charX + charWidth - strokeWidth, y + charHeight - strokeWidth * 2, strokeWidth, strokeWidth * 2, color); // bottom right
                    break;
                case 'T':
                    drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                    drawRect(charX + charWidth/2 - strokeWidth/2, y, strokeWidth, charHeight, color); // middle vertical
                    break;
                case 'S':
                    drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                    drawRect(charX, y + charHeight/2 - strokeWidth/2, charWidth, strokeWidth, color); // middle
                    drawRect(charX, y, charWidth, strokeWidth, color); // bottom
                    drawRect(charX, y + charHeight/2, strokeWidth, charHeight/2 - strokeWidth, color); // left top
                    drawRect(charX + charWidth - strokeWidth, y + strokeWidth, strokeWidth, charHeight/2 - strokeWidth, color); // right bottom
                    break;
                case 'C':
                    drawRect(charX, y + strokeWidth, strokeWidth, charHeight - 2*strokeWidth, color); // left
                    drawRect(charX, y, charWidth, strokeWidth, color); // bottom
                    drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                    break;
                case 'O':
                    drawRect(charX, y + strokeWidth, strokeWidth, charHeight - 2*strokeWidth, color); // left
                    drawRect(charX + charWidth - strokeWidth, y + strokeWidth, strokeWidth, charHeight - 2*strokeWidth, color); // right
                    drawRect(charX + strokeWidth, y, charWidth - 2*strokeWidth, strokeWidth, color); // bottom
                    drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - 2*strokeWidth, strokeWidth, color); // top
                    break;
                case 'R':
                    drawRect(charX, y, strokeWidth, charHeight, color); // left
                    drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - strokeWidth, strokeWidth, color); // top
                    drawRect(charX + charWidth - strokeWidth, y + charHeight/2, strokeWidth, charHeight/2 - strokeWidth, color); // right top
                    drawRect(charX + strokeWidth, y + charHeight/2 - strokeWidth/2, charWidth - strokeWidth, strokeWidth, color); // middle
                    drawRect(charX + charWidth/2, y, strokeWidth, charHeight/2, color); // diagonal
                    break;
                case 'L':
                    drawRect(charX, y, strokeWidth, charHeight, color); // left
                    drawRect(charX + strokeWidth, y, charWidth - strokeWidth, strokeWidth, color); // bottom
                    break;
                case 'I':
                    drawRect(charX, y, charWidth, strokeWidth, color); // bottom
                    drawRect(charX + charWidth/2 - strokeWidth/2, y, strokeWidth, charHeight, color); // middle vertical
                    drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                    break;
                case ' ':
                    // Space - do nothing
                    break;
            }
        }
    }
    
    void drawDigit(int digit, float x, float y, float size, const Color& color) {
        // Simple 7-segment display style digits using rectangles
        float segWidth = size * 0.8f;
        float segHeight = size * 0.1f;
        float segThick = size * 0.15f;
        
        // Segment positions (7-segment display)
        bool segments[10][7] = {
            {1,1,1,1,1,1,0}, // 0
            {0,1,1,0,0,0,0}, // 1
            {1,1,0,1,1,0,1}, // 2
            {1,1,1,1,0,0,1}, // 3
            {0,1,1,0,0,1,1}, // 4
            {1,0,1,1,0,1,1}, // 5
            {1,0,1,1,1,1,1}, // 6
            {1,1,1,0,0,0,0}, // 7
            {1,1,1,1,1,1,1}, // 8
            {1,1,1,1,0,1,1}  // 9
        };
        
        if (digit < 0 || digit > 9) return;
        
        // Draw segments
        if (segments[digit][0]) drawRect(x, y + size - segHeight, segWidth, segHeight, color); // top
        if (segments[digit][1]) drawRect(x + segWidth - segThick, y + size/2, segThick, size/2 - segHeight/2, color); // top right
        if (segments[digit][2]) drawRect(x + segWidth - segThick, y, segThick, size/2 - segHeight/2, color); // bottom right
        if (segments[digit][3]) drawRect(x, y, segWidth, segHeight, color); // bottom
        if (segments[digit][4]) drawRect(x, y, segThick, size/2 - segHeight/2, color); // bottom left
        if (segments[digit][5]) drawRect(x, y + size/2, segThick, size/2 - segHeight/2, color); // top left
        if (segments[digit][6]) drawRect(x, y + size/2 - segHeight/2, segWidth, segHeight, color); // middle
    }
    
    void drawNumber(int number, float x, float y, float size, const Color& color) {
        if (number == 0) {
            drawDigit(0, x, y, size, color);
            return;
        }
        
        // Count digits
        int temp = number;
        int digits = 0;
        while (temp > 0) {
            temp /= 10;
            digits++;
        }
        
        // Draw digits from right to left
        float digitSpacing = size * 0.9f;
        for (int i = 0; i < digits; i++) {
            int digit = number % 10;
            drawDigit(digit, x + (digits - 1 - i) * digitSpacing, y, size, color);
            number /= 10;
        }
    }
    
    void drawBlock(int x, int y, const Color& color) {
        glUseProgram(blockShaderProgram);
        float screenX = x * BLOCK_SIZE + BOARD_OFFSET_X;
        float screenY = (BOARD_HEIGHT - y - 1) * BLOCK_SIZE + BOARD_OFFSET_Y;
        GLint offsetLoc = glGetUniformLocation(blockShaderProgram, "offset");
        glUniform2f(offsetLoc, screenX, screenY);
        GLint scaleLoc = glGetUniformLocation(blockShaderProgram, "scale");
        glUniform2f(scaleLoc, BLOCK_SIZE - 1, BLOCK_SIZE - 1);
        GLint colorLoc = glGetUniformLocation(blockShaderProgram, "color");
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    void render() {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // Darker background
        
        // Draw the game board
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if (board[y][x] != 0) {
                    drawBlock(x, y, COLORS[board[y][x]]);
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
                            drawBlock(drawX, drawY, COLORS[currentPiece.shape[i][j]]);
                        }
                    }
                }
            }
        }
        
        // Draw game board border
        Color borderColor(0.7f, 0.7f, 0.7f, 1.0f);
        int borderThickness = 3;
        // Use UI shader for borders
        drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y - borderThickness, borderThickness, BOARD_HEIGHT * BLOCK_SIZE + 2 * borderThickness, borderColor);
        drawRect(BOARD_OFFSET_X + BOARD_WIDTH * BLOCK_SIZE, BOARD_OFFSET_Y - borderThickness, borderThickness, BOARD_HEIGHT * BLOCK_SIZE + 2 * borderThickness, borderColor);
        drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y - borderThickness, BOARD_WIDTH * BLOCK_SIZE + 2 * borderThickness, borderThickness, borderColor);
        drawRect(BOARD_OFFSET_X - borderThickness, BOARD_OFFSET_Y + BOARD_HEIGHT * BLOCK_SIZE, BOARD_WIDTH * BLOCK_SIZE + 2 * borderThickness, borderThickness, borderColor);
        
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
        drawRect(panelX, nextPanelY, panelWidth, 3, panelBorder); // Top
        drawRect(panelX, nextPanelY + nextPanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
        drawRect(panelX, nextPanelY, 3, nextPanelHeight, panelBorder); // Left
        drawRect(panelX + panelWidth - 3, nextPanelY, 3, nextPanelHeight, panelBorder); // Right

        // Draw "NEXT" title
        drawText("NEXT", panelX + 10, nextPanelY + nextPanelHeight - 30, 18, textColor);

        // Draw next piece preview (use block shader for blocks)
        float previewX = panelX + 60;
        float previewY = nextPanelY + 25;
        int previewSize = 18;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (nextPiece.shape[i][j] != 0) {
                    float blockX = previewX + j * previewSize;
                    float blockY = previewY + (3 - i) * previewSize;
                    glUseProgram(blockShaderProgram);
                    GLint offsetLoc = glGetUniformLocation(blockShaderProgram, "offset");
                    glUniform2f(offsetLoc, blockX, blockY);
                    GLint scaleLoc = glGetUniformLocation(blockShaderProgram, "scale");
                    glUniform2f(scaleLoc, previewSize - 2, previewSize - 2);
                    GLint colorLoc = glGetUniformLocation(blockShaderProgram, "color");
                    Color previewColor = COLORS[nextPiece.shape[i][j]];
                    glUniform4f(colorLoc, previewColor.r, previewColor.g, previewColor.b, previewColor.a);
                    glBindVertexArray(VAO);
                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                }
            }
        }

        // Score panel
        float scorePanelY = nextPanelY - 110;
        float scorePanelHeight = 70;

        drawRect(panelX, scorePanelY, panelWidth, 3, panelBorder); // Top
        drawRect(panelX, scorePanelY + scorePanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
        drawRect(panelX, scorePanelY, 3, scorePanelHeight, panelBorder); // Left
        drawRect(panelX + panelWidth - 3, scorePanelY, 3, scorePanelHeight, panelBorder); // Right

        drawText("SCORE", panelX + 10, scorePanelY + scorePanelHeight - 28, 18, textColor);
        drawNumber(score, panelX + 20, scorePanelY + 18, 22, numberColor);

        // Lines panel
        float linesPanelY = scorePanelY - 90;
        float linesPanelHeight = 70;

        drawRect(panelX, linesPanelY, panelWidth, 3, panelBorder); // Top
        drawRect(panelX, linesPanelY + linesPanelHeight - 3, panelWidth, 3, panelBorder); // Bottom
        drawRect(panelX, linesPanelY, 3, linesPanelHeight, panelBorder); // Left
        drawRect(panelX + panelWidth - 3, linesPanelY, 3, linesPanelHeight, panelBorder); // Right

        drawText("LINES", panelX + 10, linesPanelY + linesPanelHeight - 28, 18, textColor);
        drawNumber(lines, panelX + 20, linesPanelY + 18, 22, numberColor);
    }
    
    bool isGameOver() const { return gameOver; }
    int getScore() const { return score; }
    int getLines() const { return lines; }
};

// Global game instance
TetrisGame* game = nullptr;
bool gameOverPrinted = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_LEFT:
            case GLFW_KEY_A:
                game->moveLeft();
                break;
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_D:
                game->moveRight();
                break;
            case GLFW_KEY_DOWN:
            case GLFW_KEY_S:
                game->softDrop();
                break;
            case GLFW_KEY_UP:
            case GLFW_KEY_W:
                game->rotate();
                break;
            case GLFW_KEY_SPACE:
                game->drop();
                break;
            case GLFW_KEY_R:
                if (game->isGameOver()) {
                    game->restart();
                    gameOverPrinted = false; // Reset the flag so message can be shown again
                }
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Set GLFW hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Retro Tetris", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Set viewport
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Enable blending for smooth colors
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Create game instance
    game = new TetrisGame();
    
    std::cout << "=== RETRO TETRIS ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "A/Left Arrow  - Move Left" << std::endl;
    std::cout << "D/Right Arrow - Move Right" << std::endl;
    std::cout << "S/Down Arrow  - Soft Drop" << std::endl;
    std::cout << "W/Up Arrow    - Rotate" << std::endl;
    std::cout << "Space         - Hard Drop" << std::endl;
    std::cout << "R             - Restart (when game over)" << std::endl;
    std::cout << "ESC           - Exit" << std::endl;
    
    // Game loop
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        
        // Process input
        glfwPollEvents();
        
        // Update game
        game->update(currentTime);
        
        // Render
        game->render();
        
        // Swap buffers
        glfwSwapBuffers(window);
        
        // Check for game over
        if (game->isGameOver()) {
            if (!gameOverPrinted) {
                std::cout << "\n=== GAME OVER ===" << std::endl;
                std::cout << "Final Score: " << game->getScore() << std::endl;
                std::cout << "Lines Cleared: " << game->getLines() << std::endl;
                std::cout << "Press R to restart or ESC to quit" << std::endl;
                gameOverPrinted = true;
            }
        }
    }
    
    // Cleanup
    delete game;
    glfwTerminate();
    return 0;
}