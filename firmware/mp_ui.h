#pragma once
// MakerPulse screen drawing (display only). Shared by the firmware and the
// desktop preview renderer, so the preview is pixel-identical to the device.
// Everything that blends (smooth corners, anti-aliased icons) is drawn into
// small RAM sprites and then pushed, because the CYD panel is not readable.
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Smooth (anti-aliased) digit fonts, generated as digits-only VLW subsets.
// Nunito ExtraBold digits (SIL OFL 1.1, see firmware/FONT-LICENSE.txt).
#ifndef MP_NUMFONT
#define MP_NUMFONT 1
#endif
#if MP_NUMFONT != 1
#error "Only the Nunito digit font (mpf_nunito.h, MP_NUMFONT 1) ships with this repo"
#endif
#include "mpf_nunito.h"
#define MPF_NUM MPF_NUNITO_NUM
#define MPF_MID MPF_NUNITO_MID
#define MPF_SMALL MPF_NUNITO_SMALL

#define UI_RGB(r, g, b) (uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

// Firmware-side helpers (types live here so Arduino's auto-prototypes see them).
typedef void (*UiPaint)(lgfx::LovyanGFX* g, int w, int h, const void* ctx);
struct UiCardCtx {
  int idx;
  long value;
  long delta;
};

namespace ui {

// MakerWorld-style palette: neutral dark greys (#313131 family) + brand green #00AE42.
static const uint16_t BG = UI_RGB(0x14, 0x14, 0x14);
static const uint16_t CARD = UI_RGB(0x25, 0x25, 0x25);
static const uint16_t CARD_EDGE = UI_RGB(0x31, 0x31, 0x31);
static const uint16_t BAR = UI_RGB(0x1C, 0x1C, 0x1C);
static const uint16_t FG = UI_RGB(0xFF, 0xFF, 0xFF);
static const uint16_t MUTED = UI_RGB(0xA6, 0xA6, 0xA6);
static const uint16_t DIM = UI_RGB(0x3A, 0x3A, 0x3A);
static const uint16_t GREEN = UI_RGB(0x00, 0xAE, 0x42);
static const uint16_t GREEN_HI = UI_RGB(0x2E, 0xD0, 0x6A);  // icon/text green, a touch brighter on a dark panel
static const uint16_t TEAL = GREEN;
static const uint16_t RED = UI_RGB(0xF0, 0x4D, 0x4D);

static const char* const LABELS[6] = {"Downloads", "Likes", "Prints", "Boosts", "Collections", "Comments"};

// Screen geometry (landscape 320x240).
static const int HEAD_H = 42;
static const int GRID_TOP = 45;
static const int MARGIN = 7;
static const int GAP = 5;
static const int CARD_H = 54;
static const int FOOT_Y = 218;

struct Dash {
  const char* name;
  long followers, dFollowers;
  long value[6];
  long delta[6];
  bool lastOk;
  bool commentsReady;
  const char* clock;  // "20:40" or "--:--"
  int censusFound, censusTotal, poll;
  const char* error;  // shown when !lastOk
  // right side of the footer: 0 = MQTT pill, 1 = IP text, 2 = Telegram text
  int rightMode;
  bool mqttUp;
  const char* rightText;
  bool wifiUp;
  int rssi;
};

inline uint16_t blend(uint16_t a, uint16_t b, uint8_t t) {  // t=255 -> a
  int ar = (a >> 11) & 31, ag = (a >> 5) & 63, ab = a & 31;
  int br = (b >> 11) & 31, bg = (b >> 5) & 63, bb = b & 31;
  int r = (ar * t + br * (255 - t)) / 255;
  int g = (ag * t + bg * (255 - t)) / 255;
  int bl = (ab * t + bb * (255 - t)) / 255;
  return (uint16_t)((r << 11) | (g << 5) | bl);
}

// Select a smooth digit font (only digits . + - are in it). Drawn anti-aliased into sprites.
inline void numFont(lgfx::LovyanGFX* g, int which) {
  g->loadFont(which == 0 ? MPF_NUM : which == 1 ? MPF_MID : MPF_SMALL);
}

// Same grouping as fmtNum(): dots every three digits.
inline void fmtNum(long n, char* out, size_t cap) {
  char s[24];
  snprintf(s, sizeof(s), "%ld", n);
  int len = (int)strlen(s);
  size_t o = 0;
  for (int i = 0; i < len && o + 1 < cap; i++) {
    if (i > 0 && (len - i) % 3 == 0 && o + 2 < cap) out[o++] = '.';
    out[o++] = s[i];
  }
  out[o] = 0;
}

inline void fmtDelta(long d, char* out, size_t cap) {
  if (d == 0) { out[0] = 0; return; }
  char n[24];
  fmtNum(d > 0 ? d : -d, n, sizeof(n));
  snprintf(out, cap, "%c%s", d > 0 ? '+' : '-', n);
}

// ---------- icons (centre cx,cy, ~16 px) ----------
inline void iconDownload(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c) {
  g->drawWideLine(cx, cy - 7, cx, cy + 2, 1.4f, c);
  g->drawWideLine(cx - 5, cy - 2, cx, cy + 3, 1.4f, c);
  g->drawWideLine(cx + 5, cy - 2, cx, cy + 3, 1.4f, c);
  g->drawWideLine(cx - 7, cy + 4, cx - 7, cy + 7, 1.2f, c);
  g->drawWideLine(cx + 7, cy + 4, cx + 7, cy + 7, 1.2f, c);
  g->drawWideLine(cx - 7, cy + 7, cx + 7, cy + 7, 1.2f, c);
}
inline void iconHeart(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c) {
  g->fillSmoothCircle(cx - 4, cy - 3, 4, c);
  g->fillSmoothCircle(cx + 4, cy - 3, 4, c);
  g->fillTriangle(cx - 8, cy - 2, cx + 8, cy - 2, cx, cy + 7, c);
  g->drawWideLine(cx - 7, cy, cx, cy + 7, 0.9f, c);
  g->drawWideLine(cx + 7, cy, cx, cy + 7, 0.9f, c);
}
inline void iconPrinter(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c, uint16_t bg) {
  // printer: paper in, body, printed sheet out
  g->fillRect(cx - 5, cy - 8, 10, 4, c);
  g->fillSmoothRoundRect(cx - 9, cy - 4, 18, 9, 2, c);
  g->fillRect(cx - 6, cy + 1, 12, 2, bg);
  g->fillRect(cx - 5, cy + 2, 10, 7, c);
  g->drawFastHLine(cx - 3, cy + 5, 6, bg);
  g->fillRect(cx + 5, cy - 2, 2, 2, bg);  // status led
}
inline void iconBolt(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c) {
  g->fillTriangle(cx + 2, cy - 8, cx - 6, cy + 1, cx + 1, cy + 1, c);
  g->fillTriangle(cx - 1, cy - 1, cx + 6, cy - 1, cx - 2, cy + 8, c);
}
inline void iconBookmark(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c, uint16_t bg) {
  g->fillSmoothRoundRect(cx - 6, cy - 8, 12, 16, 2, c);
  g->fillTriangle(cx - 6, cy + 9, cx + 6, cy + 9, cx, cy + 3, bg);
}
inline void iconBubble(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c, uint16_t bg) {
  g->fillSmoothRoundRect(cx - 8, cy - 7, 17, 12, 4, c);
  g->fillTriangle(cx - 5, cy + 3, cx, cy + 3, cx - 6, cy + 8, c);
  g->fillSmoothCircle(cx - 4, cy - 1, 1, bg);
  g->fillSmoothCircle(cx, cy - 1, 1, bg);
  g->fillSmoothCircle(cx + 4, cy - 1, 1, bg);
}
inline void iconPeople(lgfx::LovyanGFX* g, int cx, int cy, uint16_t c) {
  g->fillSmoothCircle(cx, cy - 3, 3, c);
  g->fillSmoothRoundRect(cx - 5, cy + 1, 11, 6, 3, c);
}
inline void drawIcon(lgfx::LovyanGFX* g, int i, int cx, int cy, uint16_t c, uint16_t bg) {
  switch (i) {
    case 0: iconDownload(g, cx, cy, c); break;
    case 1: iconHeart(g, cx, cy, c); break;
    case 2: iconPrinter(g, cx, cy, c, bg); break;
    case 3: iconBolt(g, cx, cy, c); break;
    case 4: iconBookmark(g, cx, cy, c, bg); break;
    default: iconBubble(g, cx, cy, c, bg); break;
  }
}

inline void wifiBars(lgfx::LovyanGFX* g, int x, int yBottom, bool up, int rssi) {
  int lit = 0;
  if (up) lit = rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : 1;
  for (int i = 0; i < 4; i++) {
    int h = 3 + i * 2;
    g->fillRect(x + i * 4, yBottom - h, 3, h, i < lit ? FG : DIM);
  }
}

// Delta badge: solid pill (15 px high, Font2 text: between v1 Font0/12 px and v2 bold 9pt/18 px).
enum { NF_NUM = 0, NF_MID = 1, NF_SMALL = 2 };
// Anchored at its right edge, vertically centred on cy.
inline int badgeRight(lgfx::LovyanGFX* g, int xr, int cy, const char* txt, uint16_t fg, uint16_t fill) {
  numFont(g, NF_SMALL);
  int w = g->textWidth(txt) + 12;
  g->fillSmoothRoundRect(xr - w, cy - 7, w, 15, 7, fill);
  g->setTextDatum(middle_center);
  g->setTextColor(fg);
  g->drawString(txt, xr - w / 2, cy + 1);
  return w;
}

// MakerWorld-like mark: green rounded square with a white "M" stroke (a nod, not the logo).
inline void logoMark(lgfx::LovyanGFX* g, int cx, int cy, int r) {
  g->fillSmoothRoundRect(cx - r, cy - r, 2 * r, 2 * r, r / 2 + 1, GREEN);
  int a = r / 2 + 1, b = r / 2;
  g->drawWideLine(cx - a, cy + b, cx - a, cy - b, 1.3f, FG);
  g->drawWideLine(cx - a, cy - b, cx, cy + 1, 1.3f, FG);
  g->drawWideLine(cx, cy + 1, cx + a, cy - b, 1.3f, FG);
  g->drawWideLine(cx + a, cy - b, cx + a, cy + b, 1.3f, FG);
}

// ---------- one stat card, drawn at (0,0) of g ----------
inline void drawCard(lgfx::LovyanGFX* g, int w, int h, int idx, long value, long delta) {
  g->fillRect(0, 0, w, h, BG);
  g->fillSmoothRoundRect(0, 0, w, h, 10, CARD_EDGE);
  g->fillSmoothRoundRect(1, 1, w - 2, h - 2, 9, CARD);
  // round icon disc, MakerWorld green
  uint16_t disc = blend(GREEN, CARD, 50);
  int icx = 24, icy = h / 2;
  g->fillSmoothCircle(icx, icy, 15, disc);
  drawIcon(g, idx, icx, icy, GREEN_HI, disc);

  int tx = 46;

  char num[24], dl[24];
  fmtNum(value, num, sizeof(num));
  fmtDelta(delta, dl, sizeof(dl));
  int numY = h - 10;  // baseline
  int numMax = w - tx - 6;
  int labelMax = w - tx - 6;
  if (dl[0]) {
    bool up = delta > 0;
    numFont(g, NF_SMALL);
    int bw = g->textWidth(dl) + 12;
    numFont(g, NF_MID);
    int nw9 = g->textWidth(num);
    if (nw9 <= numMax - bw - 4) {
      badgeRight(g, w - 6, numY - 7, dl, FG, up ? GREEN : RED);  // big badge beside the number
      numMax -= bw + 4;
    } else {
      // very large number + delta: small badge in the label row instead
      numFont(g, NF_SMALL);
      int sw = g->textWidth(dl) + 8;
      g->fillSmoothRoundRect(w - 6 - sw, 5, sw, 15, 7, up ? GREEN : RED);
      g->setTextDatum(middle_center);
      g->setTextColor(FG);
      g->drawString(dl, w - 6 - sw / 2, 13);
      labelMax = w - 6 - sw - 4 - tx;
    }
  }
  char lab[16];
  strncpy(lab, LABELS[idx], sizeof(lab) - 1);
  lab[sizeof(lab) - 1] = 0;
  g->setFont(&fonts::DejaVu12);
  while (strlen(lab) > 1 && g->textWidth(lab) > labelMax) lab[strlen(lab) - 1] = 0;
  g->setTextDatum(top_left);
  g->setTextColor(MUTED);
  g->drawString(lab, tx, 7);
  numFont(g, NF_NUM);
  if (g->textWidth(num) > numMax) numFont(g, NF_MID);
  g->setTextDatum(baseline_left);
  g->setTextColor(FG);
  g->drawString(num, tx, numY);
  g->setFont(&fonts::Font0);  // releases the runtime font
}

inline void drawHeader(lgfx::LovyanGFX* g, int w, const Dash& d) {
  g->fillRect(0, 0, w, HEAD_H, BG);
  g->fillRect(0, 0, w, 2, GREEN);

  const char* name = (d.name && d.name[0]) ? d.name : "Maker";
  int ay = 22;
  logoMark(g, 21, ay, 13);

  // follower pill (right)
  char fans[24], plus[24];
  fmtNum(d.followers, fans, sizeof(fans));
  fmtDelta(d.dFollowers, plus, sizeof(plus));
  numFont(g, NF_MID);
  int fansW = g->textWidth(fans);
  numFont(g, NF_SMALL);
  int plusW = plus[0] ? g->textWidth(plus) + 12 + 4 : 0;
  g->setFont(&fonts::DejaVu9);
  int wordW = g->textWidth("followers");
  int pillW = 8 + 14 + 4 + fansW + 5 + wordW + 9 + plusW;
  int px = w - 7 - pillW, py = ay - 13;
  g->fillSmoothRoundRect(px, py, pillW, 26, 13, CARD_EDGE);
  g->fillSmoothRoundRect(px + 1, py + 1, pillW - 2, 24, 12, CARD);
  iconPeople(g, px + 15, ay, GREEN_HI);
  int cx = px + 8 + 14 + 4;
  numFont(g, NF_MID);
  g->setTextDatum(middle_left);
  g->setTextColor(FG);
  g->drawString(fans, cx, ay);
  cx += fansW + 5;
  g->setFont(&fonts::DejaVu9);
  g->setTextColor(MUTED);
  g->drawString("followers", cx, ay + 1);
  if (plus[0]) {
    bool up = d.dFollowers > 0;
    badgeRight(g, px + pillW - 5, ay, plus, FG, up ? GREEN : RED);
  }

  // name + small "MakerWorld" caption, trimmed to fit
  char nm[32];
  strncpy(nm, name, sizeof(nm) - 1);
  nm[sizeof(nm) - 1] = 0;
  int nameMax = px - 42 - 6;
  g->setFont(&fonts::FreeSansBold12pt7b);
  while (strlen(nm) > 1 && g->textWidth(nm) > nameMax) nm[strlen(nm) - 1] = 0;
  g->setTextDatum(baseline_left);
  g->setTextColor(FG);
  g->drawString(nm, 42, 21);
  g->setFont(&fonts::DejaVu9);
  g->setTextDatum(top_left);
  g->setTextColor(GREEN_HI);
  g->drawString("MakerWorld", 42, 28);
}

// Left part of the footer. Returns the x where it ends; draws only when draw=true.
inline int footerLeft(lgfx::LovyanGFX* g, const Dash& d, int cy, const lgfx::IFont* font, bool draw) {
  g->setFont(font);
  g->setTextDatum(middle_left);
  int x = 8;
  if (!d.lastOk) {
    const char* e = d.error && d.error[0] ? d.error : "check failed";
    if (draw) {
      g->fillSmoothCircle(x + 3, cy, 3, RED);
      g->setTextColor(RED);
      g->drawString(e, x + 11, cy);
    }
    return x + 11 + g->textWidth(e);
  }
  uint16_t sc = d.commentsReady ? MUTED : RED;
  if (draw) {  // clock glyph
    g->fillSmoothCircle(x + 5, cy, 5, MUTED);
    g->fillSmoothCircle(x + 5, cy, 4, BAR);
    g->drawFastVLine(x + 5, cy - 3, 4, MUTED);
    g->drawFastHLine(x + 5, cy, 3, MUTED);
  }
  x += 14;
  char buf[24];
  if (d.censusTotal > 0) {
    if (draw) { g->setTextColor(FG); g->drawString(d.clock, x, cy); }
    x += g->textWidth(d.clock) + 9;
    snprintf(buf, sizeof(buf), "%d/%d", d.censusFound, d.censusTotal);
    if (draw) { g->setTextColor(sc); g->drawString(buf, x, cy); }
    x += g->textWidth(buf) + 9;
  } else {
    if (draw) { g->setTextColor(sc); g->drawString("check", x, cy); }
    x += g->textWidth("check") + 5;
    if (draw) { g->setTextColor(FG); g->drawString(d.clock, x, cy); }
    x += g->textWidth(d.clock) + 9;
  }
  snprintf(buf, sizeof(buf), "%ds", d.poll);
  if (draw) { g->setTextColor(sc); g->drawString(buf, x, cy); }
  return x + g->textWidth(buf);
}

// Footer: drawn at (0,0) of g; height 240-FOOT_Y. progress 0..1000.
inline void drawFooter(lgfx::LovyanGFX* g, int w, const Dash& d, int progress) {
  int h = 240 - FOOT_Y;
  g->fillRect(0, 0, w, h, BAR);
  g->fillRect(0, 0, w, 2, DIM);
  if (progress > 0) g->fillRect(0, 0, (w * (progress > 1000 ? 1000 : progress)) / 1000, 2, GREEN);
  int cy = 2 + (h - 2) / 2;

  // right side first, so the left side knows how much room it has
  int xr = w - 8;
  g->setFont(&fonts::DejaVu12);
  if (d.rightMode == 0) {
    uint16_t c = d.mqttUp ? GREEN : RED;
    int pw = g->textWidth(d.rightText) + 22;
    g->fillSmoothRoundRect(xr - pw, cy - 8, pw, 16, 8, blend(c, BAR, 50));
    g->fillSmoothCircle(xr - pw + 8, cy, 3, c);
    g->setTextDatum(middle_left);
    g->setTextColor(c == GREEN ? GREEN_HI : c);
    g->drawString(d.rightText, xr - pw + 14, cy);
    xr -= pw + 8;
  } else {
    g->setTextDatum(middle_right);
    g->setTextColor(MUTED);
    g->drawString(d.rightText, xr, cy);
    xr -= g->textWidth(d.rightText) + 8;
  }
  const lgfx::IFont* font = &fonts::DejaVu12;
  bool bars = true;
  if (footerLeft(g, d, cy, font, false) > xr - 15 - 6) {
    bars = false;
    if (footerLeft(g, d, cy, font, false) > xr - 4) font = &fonts::DejaVu9;
  }
  if (bars) wifiBars(g, xr - 15, cy + 5, d.wifiUp, d.rssi);
  footerLeft(g, d, cy, font, true);
}

inline void cardRect(int i, int W, int& x, int& y, int& w, int& h) {
  w = (W - MARGIN * 2 - GAP) / 2;
  h = CARD_H;
  x = MARGIN + (i % 2) * (w + GAP);
  y = GRID_TOP + (i / 2) * (CARD_H + GAP);
}

// ---------- full-screen status / portal / connected screens ----------
// These are drawn in horizontal bands: oy is the screen row that maps to y=0
// of the target, so a 320x48 sprite can be filled band by band.
inline void topBar(lgfx::LovyanGFX* g, int W, int oy) {
  g->fillScreen(BG);
  g->fillRect(0, 0 - oy, W, 2, GREEN);
}

struct Screen {
  int kind;  // 0 status, 1 portal, 2 connected
  const char* a;
  const char* b;
};

inline void drawStatusScreen(lgfx::LovyanGFX* g, int W, int H, int oy, const char* title, const char* detail) {
  topBar(g, W, oy);
  int cw = W - 60, ch = 84, cx = 30, cy = (H - ch) / 2 - oy;
  g->fillSmoothRoundRect(cx, cy, cw, ch, 12, CARD_EDGE);
  g->fillSmoothRoundRect(cx + 1, cy + 1, cw - 2, ch - 2, 11, CARD);
  g->setTextDatum(middle_center);
  g->setFont(&fonts::FreeSansBold12pt7b);
  g->setTextColor(FG);
  g->drawString(title, W / 2, cy + 30);
  g->setFont(&fonts::DejaVu12);
  g->setTextColor(MUTED);
  g->drawString(detail, W / 2, cy + 58);
}

inline void stepRow(lgfx::LovyanGFX* g, int x, int y, int w, const char* n, const char* head, const char* val,
                    const char* hint) {
  g->fillSmoothRoundRect(x, y, w, 62, 10, CARD_EDGE);
  g->fillSmoothRoundRect(x + 1, y + 1, w - 2, 60, 9, CARD);
  g->fillSmoothCircle(x + 22, y + 31, 13, GREEN);
  g->setFont(&fonts::FreeSansBold9pt7b);
  g->setTextDatum(middle_center);
  g->setTextColor(FG);
  g->drawString(n, x + 22, y + 32);
  g->setTextDatum(top_left);
  g->setFont(&fonts::DejaVu9);
  g->setTextColor(MUTED);
  g->drawString(head, x + 46, y + 8);
  g->setFont(&fonts::FreeSansBold12pt7b);
  g->setTextColor(FG);
  g->drawString(val, x + 46, y + 20);
  g->setFont(&fonts::DejaVu9);
  g->setTextColor(MUTED);
  g->drawString(hint, x + 46, y + 46);
}

inline void drawPortalScreen(lgfx::LovyanGFX* g, int W, int H, int oy, const char* apSsid) {
  topBar(g, W, oy);
  logoMark(g, 21, 24 - oy, 13);
  g->setTextDatum(top_left);
  g->setFont(&fonts::DejaVu9);
  g->setTextColor(GREEN_HI);
  g->drawString("MAKERPULSE", 44, 10 - oy);
  g->setFont(&fonts::FreeSansBold12pt7b);
  g->setTextColor(FG);
  g->drawString("Setup", 44, 21 - oy);

  stepRow(g, 8, 50 - oy, W - 16, "1", "WiFi", apSsid, "password   makerpulse");
  stepRow(g, 8, 118 - oy, W - 16, "2", "Browser", "192.168.4.1", "paste a link to your model");
  if (H > 200) {
    g->setFont(&fonts::DejaVu9);
    g->setTextDatum(middle_center);
    g->setTextColor(MUTED);
    g->drawString("Then this screen restarts.", W / 2, H - 16 - oy);
  }
}

inline void drawConnectedScreen(lgfx::LovyanGFX* g, int W, int H, int oy, const char* ip) {
  topBar(g, W, oy);
  int cy = H / 2 - 30 - oy;
  g->fillSmoothCircle(W / 2, cy, 20, blend(GREEN, BG, 70));
  g->drawWideLine(W / 2 - 9, cy, W / 2 - 3, cy + 7, 2.0f, GREEN);
  g->drawWideLine(W / 2 - 3, cy + 7, W / 2 + 10, cy - 7, 2.0f, GREEN);
  g->setTextDatum(middle_center);
  g->setFont(&fonts::FreeSansBold12pt7b);
  g->setTextColor(FG);
  g->drawString("Connected", W / 2, cy + 38);
  char buf[40];
  g->setFont(&fonts::DejaVu12);
  g->setTextColor(MUTED);
  g->drawString(ip, W / 2, cy + 62);
  snprintf(buf, sizeof(buf), "%s/setup", ip);
  g->setTextColor(GREEN);
  g->drawString(buf, W / 2, cy + 80);
}

inline void drawScreen(lgfx::LovyanGFX* g, int W, int H, int oy, const Screen& s) {
  if (s.kind == 1) drawPortalScreen(g, W, H, oy, s.a);
  else if (s.kind == 2) drawConnectedScreen(g, W, H, oy, s.a);
  else drawStatusScreen(g, W, H, oy, s.a, s.b);
}

static const int BAND_H = 48;

// Render a full screen through one band sprite (W x BAND_H, 16-bit, ~30 KB).
// Falls back to drawing straight on the panel if the sprite cannot be allocated.
inline void renderScreen(lgfx::LovyanGFX* dst, const Screen& s) {
  int W = dst->width(), H = dst->height();
  LGFX_Sprite band(dst);
  band.setColorDepth(16);
  if (!band.createSprite(W, BAND_H)) {
    drawScreen(dst, W, H, 0, s);
    return;
  }
  for (int oy = 0; oy < H; oy += BAND_H) {
    drawScreen(&band, W, H, oy, s);
    band.pushSprite(0, oy);
  }
  band.deleteSprite();
}

}  // namespace ui
