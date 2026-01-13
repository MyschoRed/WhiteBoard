#include "Editor.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <cmath>
#include <ctime>
#include <cstdio>

static std::string GetTimestampFilename() {
    std::time_t now = std::time(nullptr);
    std::tm* t = std::localtime(&now);
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "whiteboard_%02d%02d%02d_%02d%02d%02d.png",
                  t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
                  t->tm_hour, t->tm_min, t->tm_sec);
    return buffer;
}

Editor::Editor(int windowWidth, int windowHeight)
    : windowWidth(windowWidth)
    , windowHeight(windowHeight)
    , currentTool(Tool::PENCIL)
    , brushSize(2.0f)
    , fillShapes(false)
    , isDrawing(false)
    , startPos({0, 0})
    , lastPos({0, 0})
    , currentPos({0, 0})
    , showSaveDialog(false)
    , showLoadDialog(false)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(windowWidth, windowHeight, "WhiteBoard");
    SetTargetFPS(60);

    // Canvas velkost = okno minus menu
    int canvasWidth = windowWidth - MENU_WIDTH;
    int canvasHeight = windowHeight;
    canvas = std::make_unique<Canvas>(canvasWidth, canvasHeight);

    // Nastavenie GUI stylu
    GuiSetStyle(DEFAULT, TEXT_SIZE, 14);
}

Editor::~Editor() {
    CloseWindow();
}

void Editor::Run() {
    while (!WindowShouldClose()) {
        Update();
        Draw();
    }
}

void Editor::Update() {
    // Aktualizuj rozmery okna ak sa zmenili
    int newWidth = GetScreenWidth();
    int newHeight = GetScreenHeight();

    if (newWidth != windowWidth || newHeight != windowHeight) {
        windowWidth = newWidth;
        windowHeight = newHeight;

        // Resize canvas
        int canvasWidth = windowWidth - MENU_WIDTH;
        int canvasHeight = windowHeight;
        canvas->Resize(canvasWidth, canvasHeight);
    }

    // Skry kurzor iba pri aktivnom kresleni
    if (isDrawing && IsMouseOnCanvas()) {
        HideCursor();
    } else {
        ShowCursor();
    }

    HandleInput();
}

void Editor::Draw() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Vykresli canvas (posunuty o sirku menu)
    Texture2D canvasTex = canvas->GetTexture();
    // RenderTexture je prevrateny, potrebujeme ho vykreslit spravne
    Rectangle source = { 0, 0, (float)canvasTex.width, -(float)canvasTex.height };
    Rectangle dest = { (float)MENU_WIDTH, 0, (float)canvasTex.width, (float)canvasTex.height };
    DrawTexturePro(canvasTex, source, dest, {0, 0}, 0, WHITE);

    // Preview pri kresleni tvarov
    if (isDrawing && (currentTool == Tool::RECTANGLE || currentTool == Tool::CIRCLE)) {
        Color previewColor = palette.GetCurrentColor();
        previewColor.a = 128; // Polpriehladna

        if (currentTool == Tool::RECTANGLE) {
            float x = std::min(startPos.x, currentPos.x);
            float y = std::min(startPos.y, currentPos.y);
            float w = std::abs(currentPos.x - startPos.x);
            float h = std::abs(currentPos.y - startPos.y);

            if (fillShapes) {
                DrawRectangle((int)(x + MENU_WIDTH), (int)y, (int)w, (int)h, previewColor);
            } else {
                DrawRectangleLines((int)(x + MENU_WIDTH), (int)y, (int)w, (int)h, previewColor);
            }
        } else if (currentTool == Tool::CIRCLE) {
            float radius = std::sqrt(
                std::pow(currentPos.x - startPos.x, 2) +
                std::pow(currentPos.y - startPos.y, 2)
            );

            if (fillShapes) {
                DrawCircle((int)(startPos.x + MENU_WIDTH), (int)startPos.y, radius, previewColor);
            } else {
                DrawCircleLines((int)(startPos.x + MENU_WIDTH), (int)startPos.y, radius, previewColor);
            }
        }
    }

    // GUI na lavej strane
    DrawGUI();

    EndDrawing();
}

void Editor::DrawGUI() {
    // Pozadie menu
    DrawRectangle(0, 0, MENU_WIDTH, windowHeight, LIGHTGRAY);

    int yPos = BUTTON_PADDING;

    // === NASTROJE ===
    GuiLabel({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), 20}, "NASTROJE");
    yPos += 25;

    // Pencil
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT},
                  currentTool == Tool::PENCIL ? "> Pencil" : "Pencil")) {
        currentTool = Tool::PENCIL;
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Eraser
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT},
                  currentTool == Tool::ERASER ? "> Eraser" : "Eraser")) {
        currentTool = Tool::ERASER;
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Rectangle
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT},
                  currentTool == Tool::RECTANGLE ? "> Rectangle" : "Rectangle")) {
        currentTool = Tool::RECTANGLE;
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Circle
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT},
                  currentTool == Tool::CIRCLE ? "> Circle" : "Circle")) {
        currentTool = Tool::CIRCLE;
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Fill checkbox (len pre tvary)
    GuiCheckBox({(float)BUTTON_PADDING, (float)yPos, 20, 20}, "Fill shapes", &fillShapes);
    yPos += 30;

    // Separator
    DrawLine(BUTTON_PADDING, yPos, MENU_WIDTH - BUTTON_PADDING, yPos, GRAY);
    yPos += 10;

    // === FARBY ===
    GuiLabel({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), 20}, "FARBY");
    yPos += 25;

    // Paleta farieb (horizontalne)
    const auto& colors = palette.GetColors();
    int colorX = BUTTON_PADDING;
    for (int i = 0; i < Palette::COLOR_COUNT; i++) {
        Rectangle colorBtn = {(float)colorX, (float)yPos, (float)COLOR_BTN_SIZE, (float)COLOR_BTN_SIZE};
        DrawRectangleRec(colorBtn, colors[i]);

        // Oramovanie pre vybranu farbu
        if (i == palette.GetCurrentIndex()) {
            DrawRectangleLinesEx(colorBtn, 3, BLACK);
        } else {
            DrawRectangleLinesEx(colorBtn, 1, DARKGRAY);
        }

        // Kliknutie na farbu
        if (CheckCollisionPointRec(GetMousePosition(), colorBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            palette.SetCurrentIndex(i);
        }

        colorX += COLOR_BTN_SIZE + 2;
        if (colorX + COLOR_BTN_SIZE > MENU_WIDTH - BUTTON_PADDING) {
            colorX = BUTTON_PADDING;
            yPos += COLOR_BTN_SIZE + 2;
        }
    }
    yPos += COLOR_BTN_SIZE + 15;

    // Separator
    DrawLine(BUTTON_PADDING, yPos, MENU_WIDTH - BUTTON_PADDING, yPos, GRAY);
    yPos += 10;

    // === AKCIE ===
    GuiLabel({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), 20}, "AKCIE");
    yPos += 25;

    // Undo
    GuiSetState(canvas->CanUndo() ? STATE_NORMAL : STATE_DISABLED);
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT}, "Undo (Ctrl+Z)")) {
        canvas->Undo();
    }
    GuiSetState(STATE_NORMAL);
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Redo
    GuiSetState(canvas->CanRedo() ? STATE_NORMAL : STATE_DISABLED);
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT}, "Redo (Ctrl+Y)")) {
        canvas->Redo();
    }
    GuiSetState(STATE_NORMAL);
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Clear All
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT}, "Clear All")) {
        canvas->Clear(BLACK);
        canvas->SaveState();
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Separator
    DrawLine(BUTTON_PADDING, yPos, MENU_WIDTH - BUTTON_PADDING, yPos, GRAY);
    yPos += 10;

    // === SUBORY ===
    GuiLabel({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), 20}, "SUBORY");
    yPos += 25;

    // Save PNG
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT}, "Save PNG")) {
        std::string filename = GetTimestampFilename();
        canvas->SaveToPNG(filename.c_str());
        TraceLog(LOG_INFO, "Saved to %s", filename.c_str());
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // Open PNG
    if (GuiButton({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), (float)BUTTON_HEIGHT}, "Open PNG")) {
        canvas->LoadFromPNG("whiteboard.png");
        TraceLog(LOG_INFO, "Loaded from whiteboard.png");
    }
    yPos += BUTTON_HEIGHT + BUTTON_PADDING;

    // === BRUSH SIZE ===
    yPos += 10;
    GuiLabel({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING), 20}, "BRUSH SIZE");
    yPos += 25;

    GuiSliderBar({(float)BUTTON_PADDING, (float)yPos, (float)(MENU_WIDTH - 2*BUTTON_PADDING - 30), 20},
                 "1", "50", &brushSize, 1.0f, 50.0f);
    yPos += 30;
}

void Editor::HandleInput() {
    Vector2 mousePos = GetMousePosition();

    // Klavesove skratky
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        if (IsKeyPressed(KEY_Z)) {
            canvas->Undo();
        }
        if (IsKeyPressed(KEY_Y)) {
            canvas->Redo();
        }
        if (IsKeyPressed(KEY_S)) {
            std::string filename = GetTimestampFilename();
            canvas->SaveToPNG(filename.c_str());
        }
        if (IsKeyPressed(KEY_O)) {
            canvas->LoadFromPNG("whiteboard.png");
        }
    }

    // Prepinanie nastrojov klavesami
    if (IsKeyPressed(KEY_ONE)) currentTool = Tool::PENCIL;
    if (IsKeyPressed(KEY_TWO)) currentTool = Tool::ERASER;
    if (IsKeyPressed(KEY_THREE)) currentTool = Tool::RECTANGLE;
    if (IsKeyPressed(KEY_FOUR)) currentTool = Tool::CIRCLE;

    // Kreslenie
    if (IsMouseOnCanvas()) {
        Vector2 canvasPos = GetCanvasMousePos();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            isDrawing = true;
            startPos = canvasPos;
            lastPos = canvasPos;
        }

        if (isDrawing) {
            currentPos = canvasPos;

            if (currentTool == Tool::PENCIL) {
                // Pencil kresli priebezne
                canvas->DrawPencilLine(lastPos, currentPos, palette.GetCurrentColor(), brushSize);
                lastPos = currentPos;
            } else if (currentTool == Tool::ERASER) {
                // Eraser kresli ciernou (farba pozadia)
                canvas->DrawPencilLine(lastPos, currentPos, BLACK, brushSize);
                lastPos = currentPos;
            }
            // Rectangle a Circle kreslia az pri pusteni tlacidla
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDrawing) {
            isDrawing = false;

            if (currentTool == Tool::PENCIL || currentTool == Tool::ERASER) {
                canvas->SaveState();
            } else if (currentTool == Tool::RECTANGLE) {
                canvas->DrawRectangleShape(startPos, currentPos, palette.GetCurrentColor(), fillShapes);
                canvas->SaveState();
            } else if (currentTool == Tool::CIRCLE) {
                float radius = std::sqrt(
                    std::pow(currentPos.x - startPos.x, 2) +
                    std::pow(currentPos.y - startPos.y, 2)
                );
                canvas->DrawCircleShape(startPos, radius, palette.GetCurrentColor(), fillShapes);
                canvas->SaveState();
            }
        }
    } else {
        // Ak mys opusti canvas pocas kreslenia
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDrawing) {
            isDrawing = false;
            if (currentTool == Tool::PENCIL || currentTool == Tool::ERASER) {
                canvas->SaveState();
            }
        }
    }
}

bool Editor::IsMouseOnCanvas() const {
    Vector2 mousePos = GetMousePosition();
    return mousePos.x >= MENU_WIDTH && mousePos.x < windowWidth &&
           mousePos.y >= 0 && mousePos.y < windowHeight;
}

Vector2 Editor::GetCanvasMousePos() const {
    Vector2 mousePos = GetMousePosition();
    return { mousePos.x - MENU_WIDTH, mousePos.y };
}
