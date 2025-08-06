#pragma once
#include <glad/glad.h>
#include <string>
#include "GameConstants.h"

class Renderer {
private:
    GLuint blockShaderProgram; // bevel effect for blocks
    GLuint uiShaderProgram;    // plain color for UI
    GLuint VAO, VBO;

public:
    Renderer();
    ~Renderer();
    
    void initOpenGL();
    void drawRect(float x, float y, float width, float height, const Color& color);
    void drawText(const std::string& text, float x, float y, float size, const Color& color);
    void drawDigit(int digit, float x, float y, float size, const Color& color);
    void drawNumber(int number, float x, float y, float size, const Color& color);
    void drawBlock(int x, int y, const Color& color);
    
    GLuint getBlockShaderProgram() const { return blockShaderProgram; }
    GLuint getUIShaderProgram() const { return uiShaderProgram; }
    GLuint getVAO() const { return VAO; }
};
