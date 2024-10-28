#pragma once

#include <array>

#include "core/constants.hpp"
#include "util.hpp"

namespace gbcxx {
class Mmu;

class FrameBuffer {
 public:
  constexpr Rgba pixel_color(usz x, usz y) const {
    return m_buf.at(y * SCREEN_WIDTH + x);
  }
  constexpr void set_pixel_color(usz x, usz y, Rgba color) {
    m_buf.at(y * SCREEN_WIDTH + x) = color;
  }
  constexpr void fill(Rgba color) { m_buf.fill(color); }
  constexpr const Rgba* data() const { return m_buf.data(); }

 private:
  std::array<Rgba, SCREEN_WIDTH * SCREEN_HEIGHT> m_buf;
};

struct LcdControl {
  u8 raw;

  bool bg_on() const { return (raw >> 0) & 1; }
  bool obj_on() const { return (raw >> 1) & 1; }
  bool obj_size() const { return (raw >> 2) & 1; }
  bool bg_map() const { return (raw >> 3) & 1; }
  bool bg_addr() const { return (raw >> 4) & 1; }
  bool window_on() const { return (raw >> 5) & 1; }
  bool window_map() const { return (raw >> 6) & 1; }
  bool lcd_on() const { return (raw >> 7) & 1; }
};

struct LcdStatus {
  u8 raw;

  bool compare() const { return (raw >> 2) & 1; }
  bool int_hblank() const { return (raw >> 3) & 1; }
  bool int_vblank() const { return (raw >> 4) & 1; }
  bool int_access_oam() const { return (raw >> 5) & 1; }
  bool int_compare() const { return (raw >> 6) & 1; }
};

class Ppu {
 public:
  Ppu() : m_vram(8_KiB), m_oam(0xA0) {}

  enum class Mode : u8 { hblank, vblank, oam_scan, transfer };

  u8 read(u16 address) const;
  void write(u16 address, u8 data);

  void tick();
  const FrameBuffer& buffer() const { return m_buffer; }
  bool should_redraw() const { return m_should_redraw; }
  void clear_should_redraw() { m_should_redraw = false; }
  u8 get_and_clear_interrupts() {
    const u8 interrupts = m_interrupts;
    m_interrupts = 0;
    return interrupts;
  }

 private:
  usz mode_cycles(Mode mode) const;
  void enter_mode(Mode mode);
  void check_lyc_interrupt();
  void check_stat_interrupt();
  void render_scanline();
  void render_bg_scanline();
  Rgba bg_color(u8 x, u8 y) const;

 private:
  std::vector<u8> m_vram;
  std::vector<u8> m_oam;
  FrameBuffer m_buffer{};
  Mode m_mode{Mode::oam_scan};
  usz m_cycles{};
  bool m_should_redraw{};
  u8 m_interrupts{};

  LcdControl m_lcdc{0x91};
  LcdStatus m_stat{0x85};
  u8 m_scy{0x00};
  u8 m_scx{0x00};
  u8 m_ly{0x00};
  u8 m_lyc{0x00};
  u8 m_bgp{0xFC};
  u8 m_obp0{0x00};
  u8 m_obp1{0x00};
  u8 m_wy{0x00};
  u8 m_wx{0x00};
};
}  // namespace gbcxx
