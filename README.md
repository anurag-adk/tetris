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
├── 📁 src/
│   ├── 📁 headers/
│   ├── main.cpp
│   ├── GameConstants.cpp
│   ├── Renderer.cpp
│   ├── TetrisPiece.cpp
│   └── TetrisGame.cpp
├── .gitignore
├── sample.tasks.json
└── README.md
</pre>

## ⚙️ Setup on Your Device (VS Code - Windows)

1. Setup OpenGL on Your Device:

   ```bash
   git clone https://github.com/anurag-adk/opengl-setup.git
   cd opengl-setup
   ```

2. Clone the Tetris project:

   ```bash
   git clone https://github.com/anurag-adk/tetris.git
   ```

3. Copy the game files:

   ```bash
   # Move all Tetris game files from tetris/src/ to src/
   mv tetris/src/* src/
   ```

4. Copy the contents of `sample.tasks.json` into `.vscode/tasks.json`:

   ```bash
   mv tetris/sample.tasks.json .vscode/tasks.json
   ```

5. Replace `REPLACE_WITH_YOUR_PATH_TO_g++.exe` in `tasks.json` with the full path to your `g++.exe`
6. Remove tetris directory (_optional_):
   ```bash
   rm -rf tetris
   ```
7. Build the project, simply press `Ctrl + Shift + B` in VS Code from root directory.
8. If compilation is successful, an executable `main.exe` will be created. Run it and play Tetris!

## 🙏 Thank You
