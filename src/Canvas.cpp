#include "Canvas.h"
#include <algorithm>
#include <cmath>

Canvas::Canvas(int width, int height) : width(width), height(height) {
    renderTexture = LoadRenderTexture(width, height);

    // Inicializuj s ciernym pozadim
    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    EndTextureMode();

    // Uloz pociatocny stav
    SaveState();
}

Canvas::~Canvas() {
    // Uvolni vsetky obrazky v historii
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

    // Vypocitaj vzdialenost medzi bodmi
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Polomer kruhu (thickness je priemer)
    float radius = thickness / 2.0f;

    // Krok medzi kruhmi - mensie hodnoty = hustejsie kruhy
    float step = std::max(1.0f, radius * 0.3f);

    if (distance < step) {
        // Ak je vzdialenost mala, nakresli jeden kruh
        DrawCircleV(end, radius, color);
    } else {
        // Interpoluj kruhy medzi start a end
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

    // Ziskaj aktualny obrazok z renderTexture
    Image img = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&img); // RenderTexture je prevrateny
    undoStack.push_back(img);

    TrimUndoStack();
}

void Canvas::Undo() {
    if (!CanUndo()) return;

    // Aktualny stav presun do redo stacku
    Image currentImg = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&currentImg);
    redoStack.push_back(currentImg);

    // Odstran posledny stav z undo stacku
    undoStack.pop_back();

    // Obnov predchadzajuci stav
    if (!undoStack.empty()) {
        Image prevImg = ImageCopy(undoStack.back());
        ImageFlipVertical(&prevImg); // Flip pre renderTexture
        Texture2D tempTex = LoadTextureFromImage(prevImg);

        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        DrawTexture(tempTex, 0, 0, WHITE);
        EndTextureMode();

        UnloadTexture(tempTex);
        UnloadImage(prevImg);
    } else {
        // Ak je stack prazdny, vycisti canvas
        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        EndTextureMode();

        // Pridaj prazdny stav
        SaveState();
    }
}

void Canvas::Redo() {
    if (!CanRedo()) return;

    // Ziskaj stav z redo stacku
    Image redoImg = redoStack.back();
    redoStack.pop_back();

    // Uloz aktualny stav do undo stacku
    undoStack.push_back(ImageCopy(redoImg));

    // Obnov stav
    ImageFlipVertical(&redoImg);
    Texture2D tempTex = LoadTextureFromImage(redoImg);

    BeginTextureMode(renderTexture);
    ClearBackground(BLACK);
    DrawTexture(tempTex, 0, 0, WHITE);
    EndTextureMode();

    UnloadTexture(tempTex);
    UnloadImage(redoImg);
}

bool Canvas::CanUndo() const {
    return undoStack.size() > 1; // Vzdy ponechaj aspon jeden stav
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

    // Zmen velkost ak treba
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

    // Uloz aktualny obsah
    Image currentImg = LoadImageFromTexture(renderTexture.texture);
    ImageFlipVertical(&currentImg);

    // Vytvor novu renderTexture
    UnloadRenderTexture(renderTexture);
    renderTexture = LoadRenderTexture(newWidth, newHeight);

    // Prekresli stary obsah do novej renderTexture
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
