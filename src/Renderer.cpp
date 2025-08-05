#include "headers/Renderer.h"
#include <GLFW/glfw3.h>

Renderer::Renderer() : blockShaderProgram(0), uiShaderProgram(0), VAO(0), VBO(0) {
    initOpenGL();
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(blockShaderProgram);
    glDeleteProgram(uiShaderProgram);
}

void Renderer::initOpenGL() {
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

void Renderer::drawRect(float x, float y, float width, float height, const Color& color) {
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

void Renderer::drawText(const std::string& text, float x, float y, float size, const Color& color) {
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
            case 'P':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - strokeWidth, strokeWidth, color); // top
                drawRect(charX + charWidth - strokeWidth, y + charHeight/2, strokeWidth, charHeight/2 - strokeWidth, color); // right top
                drawRect(charX + strokeWidth, y + charHeight/2 - strokeWidth/2, charWidth - strokeWidth, strokeWidth, color); // middle
                break;
            case 'A':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight, color); // right
                drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - 2*strokeWidth, strokeWidth, color); // top
                drawRect(charX + strokeWidth, y + charHeight/2 - strokeWidth/2, charWidth - 2*strokeWidth, strokeWidth, color); // middle
                break;
            case 'U':
                drawRect(charX, y + strokeWidth, strokeWidth, charHeight - strokeWidth, color); // left
                drawRect(charX + charWidth - strokeWidth, y + strokeWidth, strokeWidth, charHeight - strokeWidth, color); // right
                drawRect(charX + strokeWidth, y, charWidth - 2*strokeWidth, strokeWidth, color); // bottom
                break;
            case 'D':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - strokeWidth, strokeWidth, color); // top
                drawRect(charX + strokeWidth, y, charWidth - strokeWidth, strokeWidth, color); // bottom
                drawRect(charX + charWidth - strokeWidth, y + strokeWidth, strokeWidth, charHeight - 2*strokeWidth, color); // right
                break;
            case 'G':
                drawRect(charX, y + strokeWidth, strokeWidth, charHeight - 2*strokeWidth, color); // left
                drawRect(charX, y, charWidth, strokeWidth, color); // bottom
                drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight/2, color); // right bottom
                drawRect(charX + charWidth/2, y + charHeight/2 - strokeWidth/2, charWidth/2, strokeWidth, color); // middle right
                break;
            case 'M':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight, color); // right
                drawRect(charX + charWidth/2 - strokeWidth/2, y + charHeight/2, strokeWidth, charHeight/2, color); // middle
                drawRect(charX + strokeWidth, y + charHeight - strokeWidth, strokeWidth, strokeWidth, color); // top left diag
                drawRect(charX + charWidth - 2*strokeWidth, y + charHeight - strokeWidth, strokeWidth, strokeWidth, color); // top right diag
                break;
            case 'V':
                drawRect(charX, y + charHeight/3, strokeWidth, 2*charHeight/3, color); // left
                drawRect(charX + charWidth - strokeWidth, y + charHeight/3, strokeWidth, 2*charHeight/3, color); // right
                drawRect(charX + charWidth/2 - strokeWidth/2, y, strokeWidth, charHeight/3, color); // bottom middle
                break;
            case 'F':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX, y + charHeight - strokeWidth, charWidth, strokeWidth, color); // top
                drawRect(charX, y + charHeight/2 - strokeWidth/2, charWidth * 0.75f, strokeWidth, color); // middle
                break;
            case 'H':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight, color); // right
                drawRect(charX + strokeWidth, y + charHeight/2 - strokeWidth/2, charWidth - 2*strokeWidth, strokeWidth, color); // middle
                break;
            case 'W':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + charWidth - strokeWidth, y, strokeWidth, charHeight, color); // right
                drawRect(charX + charWidth/2 - strokeWidth/2, y, strokeWidth, charHeight/2, color); // middle bottom
                drawRect(charX + charWidth/4 - strokeWidth/2, y + charHeight/3, strokeWidth, 2*charHeight/3, color); // left middle
                drawRect(charX + 3*charWidth/4 - strokeWidth/2, y + charHeight/3, strokeWidth, 2*charHeight/3, color); // right middle
                break;
            case 'B':
                drawRect(charX, y, strokeWidth, charHeight, color); // left
                drawRect(charX + strokeWidth, y + charHeight - strokeWidth, charWidth - strokeWidth, strokeWidth, color); // top
                drawRect(charX + strokeWidth, y, charWidth - strokeWidth, strokeWidth, color); // bottom
                drawRect(charX + strokeWidth, y + charHeight/2 - strokeWidth/2, charWidth - strokeWidth, strokeWidth, color); // middle
                drawRect(charX + charWidth - strokeWidth, y + charHeight/2, strokeWidth, charHeight/2 - strokeWidth, color); // right top
                drawRect(charX + charWidth - strokeWidth, y + strokeWidth, strokeWidth, charHeight/2 - strokeWidth, color); // right bottom
                break;
            case 'Y':
                drawRect(charX, y + charHeight/2, strokeWidth, charHeight/2, color); // left top
                drawRect(charX + charWidth - strokeWidth, y + charHeight/2, strokeWidth, charHeight/2, color); // right top
                drawRect(charX + charWidth/2 - strokeWidth/2, y, strokeWidth, charHeight/2, color); // middle bottom
                break;
            case ' ':
                // Space - do nothing
                break;
        }
    }
}

void Renderer::drawDigit(int digit, float x, float y, float size, const Color& color) {
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

void Renderer::drawNumber(int number, float x, float y, float size, const Color& color) {
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

void Renderer::drawBlock(int x, int y, const Color& color) {
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
