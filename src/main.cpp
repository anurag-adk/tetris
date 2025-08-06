#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "headers/TetrisGame.h"

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
            case GLFW_KEY_ENTER:
                game->drop();
                break;
            case GLFW_KEY_R:
                game->restart();
                gameOverPrinted = false; // Reset the flag so message can be shown again
                break;
            case GLFW_KEY_SPACE:
                if (!game->hasStarted()) {
                    game->startGame();
                } else {
                    game->togglePause();
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
    std::cout << "Enter         - Hard Drop" << std::endl;
    std::cout << "Space         - Pause/Resume" << std::endl;
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
        
        // Check for pause state
        static bool pausePrinted = false;
        if (!game->isGameOver()) {
            if (game->isPaused() && !pausePrinted) {
                std::cout << "\n=== GAME PAUSED ===" << std::endl;
                std::cout << "Press SPACE to resume" << std::endl;
                pausePrinted = true;
            } else if (!game->isPaused() && pausePrinted) {
                std::cout << "Game resumed!" << std::endl;
                pausePrinted = false;
            }
        }
        
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
