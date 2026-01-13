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

    // Kresliace nastroje
    void DrawPencilLine(Vector2 start, Vector2 end, Color color, float thickness);
    void DrawRectangleShape(Vector2 start, Vector2 end, Color color, bool filled);
    void DrawCircleShape(Vector2 center, float radius, Color color, bool filled);

    // Historia
    void SaveState();
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;

    // Subory
    void SaveToPNG(const char* filename);
    void LoadFromPNG(const char* filename);

    // Resize
    void Resize(int newWidth, int newHeight);

    // Pristup k texture
    Texture2D GetTexture() const;
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    RenderTexture2D renderTexture;
    int width;
    int height;

    // Historia pre undo/redo
    std::vector<Image> undoStack;
    std::vector<Image> redoStack;
    static constexpr int MAX_HISTORY = 50;

    void ClearRedoStack();
    void TrimUndoStack();
};
