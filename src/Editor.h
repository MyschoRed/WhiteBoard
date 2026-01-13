#pragma once

#include <raylib.h>
#include <memory>
#include <string>
#include "Canvas.h"
#include "Palette.h"

enum class Tool {
    PENCIL,
    ERASER,
    RECTANGLE,
    CIRCLE
};

class Editor {
public:
    Editor(int windowWidth, int windowHeight);
    ~Editor();

    void Run();

private:
    // Window
    int windowWidth;
    int windowHeight;

    // Components
    std::unique_ptr<Canvas> canvas;
    Palette palette;

    // Tools
    Tool currentTool;
    float brushSize;
    bool fillShapes;

    // Drawing state
    bool isDrawing;
    Vector2 startPos;
    Vector2 lastPos;
    Vector2 currentPos;

    // GUI dimensions
    static constexpr int MENU_WIDTH = 120;
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int BUTTON_PADDING = 5;
    static constexpr int COLOR_BTN_SIZE = 30;

    // Methods
    void Update();
    void Draw();
    void DrawGUI();
    void HandleInput();

    // Helpers
    bool IsMouseOnCanvas() const;
    Vector2 GetCanvasMousePos() const;

    // File dialog helpers
    std::string saveFilename;
    std::string loadFilename;
    bool showSaveDialog;
    bool showLoadDialog;
};
