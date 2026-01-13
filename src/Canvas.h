#pragma once

#include <raylib.h>
#include <vector>

class Canvas {
public:
    Canvas(int width, int height);
    ~Canvas();

    void Clear(Color color);
    void BeginDrawing();
    void EndDrawing();

    // Drawing tools
    void DrawPencilLine(Vector2 start, Vector2 end, Color color, float thickness);
    void DrawRectangleShape(Vector2 start, Vector2 end, Color color, bool filled);
    void DrawCircleShape(Vector2 center, float radius, Color color, bool filled);

    // History
    void SaveState();
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;

    // Files
    void SaveToPNG(const char* filename);
    void LoadFromPNG(const char* filename);

    // Resize
    void Resize(int newWidth, int newHeight);

    // Texture access
    Texture2D GetTexture() const;
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    RenderTexture2D renderTexture;
    int width;
    int height;

    // History for undo/redo
    std::vector<Image> undoStack;
    std::vector<Image> redoStack;
    static constexpr int MAX_HISTORY = 50;

    void ClearRedoStack();
    void TrimUndoStack();
};
