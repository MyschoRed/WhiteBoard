#include "Palette.h"

Palette::Palette() : currentIndex(0) {
    // 5 colors: white, red, green, blue, yellow
    colors[0] = WHITE;
    colors[1] = RED;
    colors[2] = GREEN;
    colors[3] = BLUE;
    colors[4] = YELLOW;
}

Color Palette::GetColor(int index) const {
    if (index >= 0 && index < COLOR_COUNT) {
        return colors[index];
    }
    return WHITE;
}

Color Palette::GetCurrentColor() const {
    return colors[currentIndex];
}

void Palette::SetCurrentIndex(int index) {
    if (index >= 0 && index < COLOR_COUNT) {
        currentIndex = index;
    }
}
