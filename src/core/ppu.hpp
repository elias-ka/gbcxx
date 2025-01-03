#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "core/constants.hpp"
#include "util.hpp"

namespace gbcxx {
struct Rgba {
  uint8_t a{};
  uint8_t b{};
  uint8_t g{};
  uint8_t r{};

  constexpr Rgba() = default;
  constexpr Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
      : a(a), b(b), g(g), r(r) {}
};

class Frame_Buffer {
 public:
  [[nodiscard]] constexpr Rgba pixel_color(size_t x, size_t y) const {
    return m_buf.at((y * lcd_width) + x);
  }
  constexpr void set_pixel_color(size_t x, size_t y, Rgba color) {
    m_buf.at((y * lcd_width) + x) = color;
  }
  constexpr void fill(Rgba color) { m_buf.fill(color); }
  [[nodiscard]] constexpr const Rgba* data() const { return m_buf.data(); }

 private:
  std::array<Rgba, lcd_width * lcd_height> m_buf;
};

class Ppu {
 public:
  void tick();

  [[nodiscard]] uint8_t read(uint16_t address) const;
  void write(uint16_t address, uint8_t value);

  [[nodiscard]] const Frame_Buffer& frame_buffer() const {
    return m_frame_buffer;
  }

  [[nodiscard]] bool should_redraw() const { return m_should_redraw; }
  void clear_should_redraw() { m_should_redraw = false; }

  uint8_t get_and_clear_interrupts() {
    const uint8_t ints = m_interrupts;
    m_interrupts = 0;
    return ints;
  }

 private:
  static const size_t dots_oam_scan = 80;
  static const size_t dots_transfer = 172;
  static const size_t dots_vblank = 456;
  static const size_t dots_hblank = 204;

  enum class Mode : uint8_t {
    hblank = 0,
    vblank = 1,
    oam_scan = 2,
    transfer = 3
  };

  static constexpr std::string_view mode_to_string(Mode mode) {
    switch (mode) {
      case Mode::hblank: return "HBlank";
      case Mode::vblank: return "VBlank";
      case Mode::oam_scan: return "OAM Scan";
      case Mode::transfer: return "Transfer";
    }
  }

  enum Lcd_Control : uint8_t {
    bg_and_win_display = 1 << 0,
    obj_display = 1 << 1,
    obj_size = 1 << 2,
    bg_tile_map = 1 << 3,
    bg_and_win_tile_data = 1 << 4,
    window_display = 1 << 5,
    window_tile_map = 1 << 6,
    lcd_display = 1 << 7,
  };

  enum Lcd_Status : uint8_t {
    lyc_eq_ly_interrupt = 1 << 2,
    mode_0_condition = 1 << 3,
    mode_1_condition = 1 << 4,
    mode_2_condition = 1 << 5,
    lyc_eq_ly_enable = 1 << 6,
  };

  struct Pixel {
    uint8_t color{};
    uint8_t palette{};
    uint8_t bg_prio{};
  };

  void switch_mode(Mode new_mode);
  void increment_ly();

  void on_hblank();
  void on_vblank();
  void on_oam_scan();
  void on_transfer();

  void render_scanline();
  void render_bg_scanline();

  [[nodiscard]] Rgba bg_color(uint8_t x, uint8_t y) const;

  std::array<uint8_t, 8_KiB> m_vram;
  std::array<uint8_t, 0xa0> m_oam;
  Frame_Buffer m_frame_buffer;
  std::array<Rgba, 4> m_dmg_palette = {{
      {0xff, 0xff, 0xff},
      {0xaa, 0xaa, 0xaa},
      {0x55, 0x55, 0x55},
      {0x00, 0x00, 0x00},
  }};
  Mode m_mode{Mode::oam_scan};
  bool m_should_redraw{};

  uint8_t m_interrupts{};
  uint16_t m_dots{};
  Pixel m_current_bg_pixel{};
  Pixel m_current_window_pixel{};

  uint8_t m_lcdc{0x91};
  uint8_t m_stat{0x85};
  uint8_t m_bgp{0xfc};
  uint8_t m_lx{0x00};
  uint8_t m_ly{0x00};
  uint8_t m_lyc{0x00};
  uint8_t m_scx{0x00};
  uint8_t m_scy{0x00};
  uint8_t m_obp0{0x00};
  uint8_t m_obp1{0x00};
  uint8_t m_wy{0x00};
  uint8_t m_wx{0x00};
};
}  // namespace gbcxx
