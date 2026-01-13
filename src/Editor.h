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
    // Okno
    int windowWidth;
    int windowHeight;

    // Komponenty
    std::unique_ptr<Canvas> canvas;
    Palette palette;

    // Nastroje
    Tool currentTool;
    float brushSize;
    bool fillShapes;

    // Stav kreslenia
    bool isDrawing;
    Vector2 startPos;
    Vector2 lastPos;
    Vector2 currentPos;

    // GUI rozmery
    static constexpr int MENU_WIDTH = 120;
    static constexpr int BUTTON_HEIGHT = 30;
    static constexpr int BUTTON_PADDING = 5;
    static constexpr int COLOR_BTN_SIZE = 30;

    // Metody
    void Update();
    void Draw();
    void DrawGUI();
    void HandleInput();

    // Pomocne
    bool IsMouseOnCanvas() const;
    Vector2 GetCanvasMousePos() const;

    // File dialog pomocne
    std::string saveFilename;
    std::string loadFilename;
    bool showSaveDialog;
    bool showLoadDialog;
};
