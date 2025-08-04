// Hello World with OpenGL Outlined Block Text
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

using namespace std;

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Shader sources
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

// Global OpenGL variables
GLuint VAO, VBO, shaderProgram;
GLint projectionLoc, colorLoc;

// Compile shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cout << "Shader compilation error: " << infoLog << endl;
    }
    return shader;
}

// Initialize shaders
void initShaders() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Shader linking error: " << infoLog << endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    colorLoc = glGetUniformLocation(shaderProgram, "color");
}

// Initialize buffers
void initBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// Draw a filled rectangle
void drawRect(float x, float y, float width, float height, vector<float>& vertices) {
    // Triangle 1
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(x);
    vertices.push_back(y + height);
    
    // Triangle 2
    vertices.push_back(x + width);
    vertices.push_back(y);
    vertices.push_back(x + width);
    vertices.push_back(y + height);
    vertices.push_back(x);
    vertices.push_back(y + height);
}

// Draw outlined rectangle
void drawOutlinedRect(float x, float y, float width, float height, float thickness, vector<float>& vertices) {
    // Top border
    drawRect(x, y, width, thickness, vertices);
    // Bottom border
    drawRect(x, y + height - thickness, width, thickness, vertices);
    // Left border
    drawRect(x, y, thickness, height, vertices);
    // Right border
    drawRect(x + width - thickness, y, thickness, height, vertices);
}

// Simple bitmap font data for block letters (8x8 grid each)
bool letterH[8][8] = {
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1}
};

bool letterE[8][8] = {
    {1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1}
};

bool letterL[8][8] = {
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1}
};

bool letterO[8][8] = {
    {0,1,1,1,1,1,1,0},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {0,1,1,1,1,1,1,0}
};

bool letterW[8][8] = {
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,1},
    {1,0,0,1,1,0,0,1},
    {1,0,1,0,0,1,0,1},
    {1,0,1,0,0,1,0,1},
    {0,1,0,0,0,0,1,0}
};

bool letterR[8][8] = {
    {1,1,1,1,1,1,1,0},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,0},
    {1,0,0,1,0,0,0,0},
    {1,0,0,0,1,0,0,0},
    {1,0,0,0,0,1,0,0},
    {1,0,0,0,0,0,1,0}
};

bool letterD[8][8] = {
    {1,1,1,1,1,1,0,0},
    {1,0,0,0,0,0,1,0},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0},
    {1,1,1,1,1,1,0,0}
};

bool letterSpace[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}
};

// Draw a letter at specified position
void drawLetter(bool letter[8][8], float startX, float startY, float blockSize, vector<float>& vertices) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (letter[row][col]) {
                float x = startX + col * blockSize;
                float y = startY + row * blockSize;
                drawOutlinedRect(x, y, blockSize, blockSize, blockSize * 0.1f, vertices);
            }
        }
    }
}

// Convert screen coordinates to NDC
void screenToNDC(float screenX, float screenY, float& ndcX, float& ndcY) {
    ndcX = (2.0f * screenX) / WIDTH - 1.0f;
    ndcY = 1.0f - (2.0f * screenY) / HEIGHT;
}

// Convert all vertices to NDC
void convertVerticesNDC(vector<float>& vertices) {
    for (size_t i = 0; i < vertices.size(); i += 2) {
        float ndcX, ndcY;
        screenToNDC(vertices[i], vertices[i + 1], ndcX, ndcY);
        vertices[i] = ndcX;
        vertices[i + 1] = ndcY;
    }
}

// Create "HELLO WORLD" text
void createHelloWorldText(vector<float>& vertices) {
    vertices.clear();
    
    float blockSize = 8.0f;  // Size of each block
    float letterSpacing = blockSize * 9;  // Space between letters
    
    // Calculate total text width
    float totalWidth = letterSpacing * 11; // 11 characters including space
    float startX = (WIDTH - totalWidth) / 2.0f;  // Center horizontally
    float startY = (HEIGHT - blockSize * 8) / 2.0f;  // Center vertically
    
    // Draw each letter
    drawLetter(letterH, startX + letterSpacing * 0, startY, blockSize, vertices);
    drawLetter(letterE, startX + letterSpacing * 1, startY, blockSize, vertices);
    drawLetter(letterL, startX + letterSpacing * 2, startY, blockSize, vertices);
    drawLetter(letterL, startX + letterSpacing * 3, startY, blockSize, vertices);
    drawLetter(letterO, startX + letterSpacing * 4, startY, blockSize, vertices);
    drawLetter(letterSpace, startX + letterSpacing * 5, startY, blockSize, vertices);
    drawLetter(letterW, startX + letterSpacing * 6, startY, blockSize, vertices);
    drawLetter(letterO, startX + letterSpacing * 7, startY, blockSize, vertices);
    drawLetter(letterR, startX + letterSpacing * 8, startY, blockSize, vertices);
    drawLetter(letterL, startX + letterSpacing * 9, startY, blockSize, vertices);
    drawLetter(letterD, startX + letterSpacing * 10, startY, blockSize, vertices);
    
    // Convert to NDC coordinates
    convertVerticesNDC(vertices);
}

void render(const vector<float>& vertices) {
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!vertices.empty()) {
        glUseProgram(shaderProgram);
        
        // Set identity matrix for projection (already in NDC)
        float identity[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, identity);
        
        // Set color to white
        glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        cout << "Failed to initialize GLFW" << endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Welcome to OpenGL", NULL, NULL);
    if (!window) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    
    glViewport(0, 0, WIDTH, HEIGHT);
    
    // Initialize OpenGL components
    initShaders();
    initBuffers();
    
    // Set background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Create the text vertices
    vector<float> textVertices;
    createHelloWorldText(textVertices);
    
    cout << "Displaying 'HELLO WORLD' in outlined block text!" << endl;
    cout << "Press ESC or close window to exit." << endl;
    
    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Handle input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        
        render(textVertices);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
    return 0;
}
