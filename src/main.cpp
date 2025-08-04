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
const int WINDOW_HEIGHT = 600;

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
    std::mt19937 rng;
    std::uniform_int_distribution<int> pieceDist;
    double lastFall;
    double fallSpeed;
    int score;
    int lines;
    bool gameOver;
    
    GLuint shaderProgram;
    GLuint VAO, VBO;
    
public:
    TetrisGame() : board(BOARD_HEIGHT, std::vector<int>(BOARD_WIDTH, 0)),
                   currentPiece(0), rng(std::chrono::steady_clock::now().time_since_epoch().count()),
                   pieceDist(0, 6), lastFall(0), fallSpeed(1.0), score(0), lines(0), gameOver(false) {
        spawnNewPiece();
        initOpenGL();
    }
    
    void initOpenGL() {
        // Vertex shader source
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
        
        // Fragment shader source with enhanced bevel effect
        const char* fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec4 color;
            in vec2 fragCoord;
            void main() {
                // Enhanced bevel effect for retro look
                vec2 pos = fragCoord;
                float bevelWidth = 0.15;
                float highlightIntensity = 1.4;
                float shadowIntensity = 0.6;
                
                vec4 finalColor = color;
                
                // Create highlight on top and left edges
                if (pos.y > 1.0 - bevelWidth || pos.x < bevelWidth) {
                    finalColor.rgb = min(finalColor.rgb * highlightIntensity, vec3(1.0));
                }
                // Create shadow on bottom and right edges
                else if (pos.y < bevelWidth || pos.x > 1.0 - bevelWidth) {
                    finalColor.rgb *= shadowIntensity;
                }
                
                // Add a subtle inner glow for depth
                float dist = min(min(pos.x, 1.0 - pos.x), min(pos.y, 1.0 - pos.y));
                float glow = smoothstep(0.0, 0.3, dist);
                finalColor.rgb *= (0.9 + 0.1 * glow);
                
                FragColor = finalColor;
            }
        )";
        
        // Compile shaders
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        
        // Create shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        // Set up vertex data for a quad
        float vertices[] = {
            0.0f, 0.0f,  // bottom left
            1.0f, 0.0f,  // bottom right
            1.0f, 1.0f,  // top right
            0.0f, 1.0f   // top left
        };
        
        GLuint indices[] = {
            0, 1, 2,
            2, 3, 0
        };
        
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
        
        // Set up projection matrix
        glUseProgram(shaderProgram);
        float projection[16] = {
            2.0f/WINDOW_WIDTH, 0, 0, 0,
            0, 2.0f/WINDOW_HEIGHT, 0, 0,
            0, 0, -1, 0,
            -1, -1, 0, 1
        };
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    }
    
    void spawnNewPiece() {
        currentPiece = TetrisPiece(pieceDist(rng));
        if (checkCollision(currentPiece, 0, 0)) {
            gameOver = true;
        }
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
    }
    
    void drawBlock(int x, int y, const Color& color) {
        glUseProgram(shaderProgram);
        
        // Calculate screen position
        float screenX = x * BLOCK_SIZE + 50; // 50px offset from left
        float screenY = (BOARD_HEIGHT - y - 1) * BLOCK_SIZE + 50; // 50px offset from bottom, flip Y
        
        // Set uniforms
        GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, screenX, screenY);
        
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "scale");
        glUniform2f(scaleLoc, BLOCK_SIZE - 1, BLOCK_SIZE - 1); // -1 for grid lines
        
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    
    void render() {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray background
        
        // Draw the board
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
        
        // Draw border
        glUseProgram(shaderProgram);
        GLint colorLoc = glGetUniformLocation(shaderProgram, "color");
        glUniform4f(colorLoc, 0.8f, 0.8f, 0.8f, 1.0f); // White border
        
        // Left border
        GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLoc, 45, 45);
        GLint scaleLoc = glGetUniformLocation(shaderProgram, "scale");
        glUniform2f(scaleLoc, 5, BOARD_HEIGHT * BLOCK_SIZE + 10);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Right border
        glUniform2f(offsetLoc, 50 + BOARD_WIDTH * BLOCK_SIZE, 45);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Bottom border
        glUniform2f(offsetLoc, 45, 45);
        glUniform2f(scaleLoc, BOARD_WIDTH * BLOCK_SIZE + 10, 5);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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