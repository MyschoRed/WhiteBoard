#include "Canvas.h"
#include <algorithm>
#include <cmath>

Canvas::Canvas(int width, int height) : width(width), height(height) {
    renderTexture = LoadRenderTexture(width, height);

    // Initialize with black background
    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    EndTextureMode();

    // Save initial state
    SaveState();
}

Canvas::~Canvas() {
    // Unload all images in history
    for (auto& img : undoStack) {
        UnloadImage(img);
    }
    for (auto& img : redoStack) {
        UnloadImage(img);
    }
    UnloadRenderTexture(renderTexture);
}

void Canvas::Clear(Color color) {
    BeginTextureMode(renderTexture);
    ClearBackground(color);
    EndTextureMode();
}

void Canvas::BeginDrawing() {
    BeginTextureMode(renderTexture);
}

void Canvas::EndDrawing() {
    EndTextureMode();
}

void Canvas::DrawPencilLine(Vector2 start, Vector2 end, Color color, float thickness) {
    BeginTextureMode(renderTexture);

    // Calculate distance between points
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Circle radius (thickness is diameter)
    float radius = thickness / 2.0f;

    // Step between circles - smaller values = denser circles
    float step = std::max(1.0f, radius * 0.3f);

    if (distance < step) {
        // If distance is small, draw single circle
        DrawCircleV(end, radius, color);
    } else {
        // Interpolate circles between start and end
        int steps = (int)(distance / step);
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / (float)steps;
            Vector2 pos = {
                start.x + dx * t,
                start.y + dy * t
            };
            DrawCircleV(pos, radius, color);
        }
    }

    EndTextureMode();
}

void Canvas::DrawRectangleShape(Vector2 start, Vector2 end, Color color, bool filled) {
    float x = std::min(start.x, end.x);
    float y = std::min(start.y, end.y);
    float w = std::abs(end.x - start.x);
    float h = std::abs(end.y - start.y);

    BeginTextureMode(renderTexture);
    if (filled) {
        DrawRectangle((int)x, (int)y, (int)w, (int)h, color);
    } else {
        DrawRectangleLines((int)x, (int)y, (int)w, (int)h, color);
    }
    EndTextureMode();
}

void Canvas::DrawCircleShape(Vector2 center, float radius, Color color, bool filled) {
    BeginTextureMode(renderTexture);
    if (filled) {
        DrawCircleV(center, radius, color);
    } else {
        DrawCircleLines((int)center.x, (int)center.y, radius, color);
    }
    EndTextureMode();
}

void Canvas::SaveState() {
    ClearRedoStack();

    // Get current image from renderTexture
    Image img = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&img); // RenderTexture is flipped
    undoStack.push_back(img);

    TrimUndoStack();
}

void Canvas::Undo() {
    if (!CanUndo()) return;

    // Move current state to redo stack
    Image currentImg = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&currentImg);
    redoStack.push_back(currentImg);

    // Remove last state from undo stack
    undoStack.pop_back();

    // Restore previous state
    if (!undoStack.empty()) {
        Image prevImg = ImageCopy(undoStack.back());
        Texture2D tempTex = LoadTextureFromImage(prevImg);

        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        DrawTexture(tempTex, 0, 0, WHITE);
        EndTextureMode();

        UnloadTexture(tempTex);
        UnloadImage(prevImg);
    } else {
        // If stack is empty, clear canvas
        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        EndTextureMode();

        // Add empty state
        SaveState();
    }
}

void Canvas::Redo() {
    if (!CanRedo()) return;

    // Get state from redo stack
    Image redoImg = redoStack.back();
    redoStack.pop_back();

    // Save current state to undo stack
    undoStack.push_back(ImageCopy(redoImg));

    // Restore state
    Texture2D tempTex = LoadTextureFromImage(redoImg);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    DrawTexture(tempTex, 0, 0, WHITE);
    EndTextureMode();

    UnloadTexture(tempTex);
    UnloadImage(redoImg);
}

bool Canvas::CanUndo() const {
    return undoStack.size() > 1; // Always keep at least one state
}

bool Canvas::CanRedo() const {
    return !redoStack.empty();
}

void Canvas::SaveToPNG(const char* filename) {
    Image img = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&img);
    ExportImage(img, filename);
    UnloadImage(img);
}

void Canvas::LoadFromPNG(const char* filename) {
    if (!FileExists(filename)) return;

    Image img = LoadImage(filename);
    if (img.data == nullptr) return;

    // Resize if needed
    if (img.width != width || img.height != height) {
        ImageResize(&img, width, height);
    }

    ImageFlipVertical(&img);
    Texture2D tempTex = LoadTextureFromImage(img);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    DrawTexture(tempTex, 0, 0, WHITE);
    EndTextureMode();

    UnloadTexture(tempTex);
    UnloadImage(img);

    SaveState();
}

void Canvas::Resize(int newWidth, int newHeight) {
    if (newWidth == width && newHeight == height) return;
    if (newWidth <= 0 || newHeight <= 0) return;

    // Save current content
    Image currentImg = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&currentImg);

    // Create new renderTexture
    UnloadRenderTexture(renderTexture);
    renderTexture = LoadRenderTexture(newWidth, newHeight);

    // Redraw old content to new renderTexture
    ImageFlipVertical(&currentImg);
    Texture2D tempTex = LoadTextureFromImage(currentImg);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    DrawTexture(tempTex, 0, 0, WHITE);
    EndTextureMode();

    UnloadTexture(tempTex);
    UnloadImage(currentImg);

    width = newWidth;
    height = newHeight;
}

Texture2D Canvas::GetTexture() const {
    return renderTexture.texture;
}

void Canvas::ClearRedoStack() {
    for (auto& img : redoStack) {
        UnloadImage(img);
    }
    redoStack.clear();
}

void Canvas::TrimUndoStack() {
    while (undoStack.size() > MAX_HISTORY) {
        UnloadImage(undoStack.front());
        undoStack.erase(undoStack.begin());
    }
}
