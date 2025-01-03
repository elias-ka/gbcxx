#include "core/ppu.hpp"

#include "core/interrupt.hpp"
#include "util.hpp"

namespace gbcxx {
void Ppu::tick() {
  if (!(m_lcdc & Lcd_Control::lcd_display)) {
    return;
  }
  m_dots++;
  switch (m_mode) {
    case Mode::hblank: on_hblank(); break;
    case Mode::vblank: on_vblank(); break;
    case Mode::oam_scan: on_oam_scan(); break;
    case Mode::transfer: on_transfer(); break;
  }
}

void Ppu::switch_mode(Mode new_mode) {
  m_mode = new_mode;
  m_stat = (m_stat & 0xfc) | std::to_underlying(new_mode);

  const auto check_and_set_interrupt = [&](bool cond) -> void {
    if (cond) {
      m_interrupts |= Interrupt::stat;
    }
  };

  switch (new_mode) {
    case Mode::hblank:
      check_and_set_interrupt(m_stat & Lcd_Status::mode_0_condition);
      break;
    case Mode::vblank:
      check_and_set_interrupt(m_stat & Lcd_Status::mode_1_condition);
      break;
    case Mode::oam_scan:
      check_and_set_interrupt(m_stat & Lcd_Status::mode_2_condition);
      break;
    case Mode::transfer: break;
  }
}

uint8_t Ppu::read(uint16_t address) const {
  if (vram_start <= address && address <= vram_end) {
    return m_vram.at(address - vram_start);
  }

  if (oam_start <= address && address <= oam_end) {
    return m_oam.at(address - oam_start);
  }

  switch (address) {
    case reg_lcdc: return m_lcdc;
    case reg_stat: return (m_stat & ~0x3) | std::to_underlying(m_mode);
    case reg_scy: return m_scy;
    case reg_scx: return m_scx;
    case reg_ly: return m_ly;
    case reg_lyc: return m_lyc;
    case reg_bgp: return m_bgp;
    case reg_obp0: return m_obp0;
    case reg_obp1: return m_obp1;
    case reg_wy: return m_wy;
    case reg_wx: return m_wx;
    default: {
      LOG_ERROR("PPU: Unmapped read {:X}", address);
      return 0;
    }
  }
}

void Ppu::write(uint16_t address, uint8_t value) {
  if (vram_start <= address && address <= vram_end) {
    m_vram.at(address - vram_start) = value;
    return;
  }

  if (oam_start <= address && address <= oam_end) {
    m_oam.at(address - oam_start) = value;
    return;
  }

  switch (address) {
    case reg_lcdc: m_lcdc = Lcd_Control{value}; break;
    case reg_stat: m_stat = Lcd_Status(value & ~0x3); break;
    case reg_scy: m_scy = value; break;
    case reg_scx: m_scx = value; break;
    case reg_ly: m_ly = value; break;
    case reg_lyc: m_lyc = value; break;
    case reg_bgp: m_bgp = value; break;
    case reg_obp0: m_obp0 = value; break;
    case reg_obp1: m_obp1 = value; break;
    case reg_wy: m_wy = value; break;
    case reg_wx: m_wx = value; break;
    default: {
      LOG_ERROR("PPU: Unmapped write {:X} <- {:X}", address, value);
    }
  }
}

void Ppu::increment_ly() {
  if (++m_ly == m_lyc) {
    m_stat |= Lcd_Status::lyc_eq_ly_interrupt;
    if (m_stat & Lcd_Status::lyc_eq_ly_enable) {
      m_interrupts |= Interrupt::stat;
    }
  } else {
    m_stat &= ~Lcd_Status::lyc_eq_ly_interrupt;
  }
}

void Ppu::on_hblank() {
  if (m_dots >= dots_hblank) {
    m_dots -= dots_hblank;
    increment_ly();

    if (m_ly == 144) {
      switch_mode(Mode::vblank);
      m_interrupts |= Interrupt::vblank;
      m_should_redraw = true;
    } else {
      switch_mode(Mode::oam_scan);
    }
  }
}

void Ppu::on_vblank() {
  if (m_dots >= dots_vblank) {
    m_dots -= dots_vblank;
    increment_ly();

    if (m_ly == 154) {
      switch_mode(Mode::oam_scan);
      m_ly = 0;
    }
  }
}

void Ppu::on_oam_scan() {
  if (m_dots >= dots_oam_scan) {
    m_dots -= dots_oam_scan;
    switch_mode(Mode::transfer);
  }
}

void Ppu::on_transfer() {
  if (m_dots >= dots_transfer) {
    m_dots -= dots_transfer;
    switch_mode(Mode::hblank);
    render_scanline();
  }
}

void Ppu::render_scanline() {
  render_bg_scanline();
}

void Ppu::render_bg_scanline() {
  const uint8_t scrolled_y = m_ly + m_scy;
  for (m_lx = 0; m_lx < lcd_width; m_lx++) {
    const uint8_t scrolled_x = m_lx + m_scx;
    m_frame_buffer.set_pixel_color(m_lx, m_ly,
                                   bg_color(scrolled_x, scrolled_y));
  }
}

Rgba Ppu::bg_color(uint8_t x, uint8_t y) const {
  if (!(m_lcdc & Lcd_Control::bg_and_win_display)) {
    return m_dmg_palette[0];
  }

  uint16_t address = ((m_lcdc & Lcd_Control::bg_tile_map) ? 0x1C00 : 0x1800) +
                     (((y / 8) % 32) * 32) + ((x / 8) % 32);
  const uint8_t tile_id = m_vram.at(address);
  if (m_lcdc & Lcd_Control::bg_and_win_tile_data) {
    address = uint16_t((tile_id * 16) + ((y % 8) * 2));
  } else {
    address = uint16_t((int8_t(tile_id) * 16) + ((y % 8) * 2) + 0x1000);
  }
  const auto mask = uint8_t(1 << (7 - (x & 7)));
  const uint8_t byte1 = m_vram.at(address);
  const uint8_t byte2 = m_vram.at(address + 1);
  const auto color =
      uint8_t(((byte1 & mask) ? 1 : 0) + ((byte2 & mask) ? 2 : 0));
  return m_dmg_palette[color];
}

}  // namespace gbcxx
