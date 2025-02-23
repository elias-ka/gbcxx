#pragma once

#include <cstdint>
#include <tuple>

#include "core/constants.hpp"

namespace gb
{
class Window
{
public:
    [[nodiscard]] std::tuple<uint8_t, uint8_t> GetPixelOffsets(uint8_t lx, uint8_t ly) const
    {
        const uint8_t x_off = (x_ - lx) % 8;
        const uint8_t y_off = ((ly - y_) % 8) * 2;
        return {x_off, y_off};
    }

    [[nodiscard]] uint8_t GetX() const { return x_; }
    [[nodiscard]] uint8_t GetY() const { return y_; }

    [[nodiscard]] bool ContainsPixel(uint8_t lx, uint8_t ly) const
    {
        const bool contains_x = (lx >= x_ - 7);
        const bool contains_y = ly >= y_;
        return contains_x && contains_y;
    }

    [[nodiscard]] std::tuple<uint8_t, uint8_t> GetTileMapCoords(uint8_t lx) const
    {
        return {lx - x_ - 7, line_};
    }

    void ResetLine() { line_ = 0; }

    void SetX(uint8_t v)
    {
        if (v < 7)
            return;
        x_ = v;
    }

    void SetY(uint8_t v) { y_ = v; }

    void IncrementLine(uint8_t ly)
    {
        if ((ly >= y_) && (y_ < kLcdHeight) && (x_ - 7 < kLcdWidth))
        {
            if (line_ == 255)
                return;
            line_++;
        }
    }

private:
    uint8_t line_{};
    uint8_t x_{};
    uint8_t y_{};
};
}  // namespace gb
