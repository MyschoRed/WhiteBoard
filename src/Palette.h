#pragma once

#include <raylib.h>
#include <array>

class Palette {
public:
    static constexpr int COLOR_COUNT = 5;

    Palette();

    Color GetColor(int index) const;
    Color GetCurrentColor() const;
    int GetCurrentIndex() const { return currentIndex; }
    void SetCurrentIndex(int index);

    const std::array<Color, COLOR_COUNT>& GetColors() const { return colors; }

private:
    std::array<Color, COLOR_COUNT> colors;
    int currentIndex;
};
