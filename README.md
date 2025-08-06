# 🎮 Tetris

## 📌 Project Overview

WARNING! YOU'LL GET ADDICTED AND CAN'T STOP PLAYING<br>
Tetris is a classic block-stacking game recreated using C++ and OpenGL as a part of our Computer Graphics course. This version aims to showcase foundational concepts of computer graphics such as transformations, rendering pipelines and real-time interactivity. It was built as fun project but now I can't stop playing it!

## 🔧 Tech Stack

- **C++** (Core game logic and application structure)
- **GLAD** (function loader for OpenGL)
- **GLFW** (for windowing and context handling)

## 🚀 Features

- 🎲 Classic Tetris gameplay with real-time keyboard controls
- 🔄 Animated Tetris blocks with smooth rotation and movement
- 🔮 Preview of upcomming tetris block
- ⚡ Line-clearing logic with increasing speed
- 🏆 Score tracking functionality
- ⏸️ Pause/Resume and Restart functionality
- 🕹️ Retro UI

## 📂 Folder Structure

<pre>📁 tetris/
├── 📁 include/
│   ├── 📁 glad/
│   ├── 📁 GLFW/
│   └── 📁 KHR/
├── 📁 lib/
├── 📁 src/
│   ├── 📁 headers/
│   ├── glad.c
│   ├── main.cpp
│   ├── GameConstants.cpp
│   ├── Renderer.cpp
│   ├── TetrisPiece.cpp
│   └── TetrisGame.cpp
├── .gitignore
├── glfw3.dll
├── sample.tasks.json
└── README.md
</pre>

## ⚙️ Setup on Your Device (VS Code - Windows)

1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/anurag-adk/tetris.git
   cd tetris
   ```
2. Create a `.vscode` directory in the root of the project:
   ```bash
   mkdir .vscode
   ```
3. Copy the contents of `sample.tasks.json` into `.vscode/tasks.json`:
   ```bash
   mv sample.tasks.json .vscode/tasks.json
   ```
4. Replace `REPLACE_WITH_YOUR_PATH_TO_g++.exe` with the full path to your `g++.exe`
5. Press `Ctrl + Shift + B` in VS Code
6. If compilation is successful, an executable `main.exe` will be created. Run it and play Tetris!

## 🙏 Thank You
