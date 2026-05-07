#include "DashboardUI.h"

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <cstring>

// ─── Colour Palette ───────────────────────────────────────────────────────────
static constexpr SDL_Color BG_COLOR    = {13,  15,  18,  255};
static constexpr SDL_Color PANEL_COLOR = {19,  23,  32,  255};
static constexpr SDL_Color BORDER_COL  = {30,  37,  53,  255};
static constexpr SDL_Color CYAN        = {0,   229, 255, 255};
static constexpr SDL_Color GREEN       = {57,  255, 20,  255};
static constexpr SDL_Color YELLOW      = {255, 214, 0,   255};
static constexpr SDL_Color RED         = {255, 68,  68,  255};
static constexpr SDL_Color PURPLE      = {162, 89,  255, 255};
static constexpr SDL_Color DIM_TEXT    = {90,  106, 126, 255};
static constexpr SDL_Color BODY_TEXT   = {176, 190, 197, 255};

// ─── Layout constants (1280x720) ──────────────────────────────────────────────
static constexpr int HEADER_H   = 40;
static constexpr int CARD_X     = 10;
static constexpr int CARD_W     = 220;
static constexpr int CARD_H     = 78;
static constexpr int CARD_GAP   = 8;
static constexpr int GRAPH_X    = 240;
static constexpr int GRAPH_W    = 730;
static constexpr int GRAPH_H    = 120;
static constexpr int IMU_X      = 980;
static constexpr int IMU_W      = 290;
static constexpr int CONSOLE_H  = 160;
static constexpr int PAD        = 10;

// ─── Constructor ─────────────────────────────────────────────────────────────
DashboardUI::DashboardUI(int w, int h) : win_w_(w), win_h_(h)
{
  window_ = SDL_CreateWindow(
    "IoT Telemetry Dashboard",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    w, h,
    SDL_WINDOW_SHOWN
  );

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  if (TTF_Init() == -1)
    std::cerr << "TTF_Init Error: " << TTF_GetError() << "\n";

  font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 13);
  if (!font_)
    std::cerr << "Font Load Error: " << TTF_GetError() << "\n";

  font_lg_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 20);
  if (!font_lg_)
    font_lg_ = font_; // fallback
}

// ─── Destructor ───────────────────────────────────────────────────────────────
DashboardUI::~DashboardUI()
{
  if (font_lg_ && font_lg_ != font_) TTF_CloseFont(font_lg_);
  if (font_)     TTF_CloseFont(font_);
  if (renderer_) SDL_DestroyRenderer(renderer_);
  if (window_)   SDL_DestroyWindow(window_);
}

// ─── Events ───────────────────────────────────────────────────────────────────
bool DashboardUI::poll_events()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
    if (e.type == SDL_QUIT) running_ = false;
    return running_;
}

// ─── draw_text ────────────────────────────────────────────────────────────────
void DashboardUI::draw_text(const char* txt, int x, int y, SDL_Color col, bool large)
{
  if (!font_ || !txt || !txt[0]) return;

  TTF_Font* f = (large && font_lg_) ? font_lg_ : font_;
  SDL_Surface* surf = TTF_RenderText_Blended(f, txt, col);
  if (!surf) return;
  SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer_, surf);
  SDL_Rect dst = {x, y, surf->w, surf->h};
  SDL_RenderCopy(renderer_, tex, nullptr, &dst);
  SDL_FreeSurface(surf);
  SDL_DestroyTexture(tex);
}

// ─── draw_filled_rect (helper) ────────────────────────────────────────────────
static void fill_rect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c)
{
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
  SDL_Rect rect = {x, y, w, h};
  SDL_RenderFillRect(r, &rect);
}

static void draw_rect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c)
{
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
  SDL_Rect rect = {x, y, w, h};
  SDL_RenderDrawRect(r, &rect);
}

// ─── draw_background ─────────────────────────────────────────────────────────
void DashboardUI::draw_background()
{
  // Full background
  fill_rect(renderer_, 0, 0, win_w_, win_h_, BG_COLOR);

  // Header bar
  fill_rect(renderer_, 0, 0, win_w_, HEADER_H, {10, 12, 18, 255});
  SDL_SetRenderDrawColor(renderer_, CYAN.r, CYAN.g, CYAN.b, 60);
  SDL_RenderDrawLine(renderer_, 0, HEADER_H, win_w_, HEADER_H);

  // Left accent stripe (thin vertical line beside cards)
  SDL_SetRenderDrawColor(renderer_, BORDER_COL.r, BORDER_COL.g, BORDER_COL.b, 255);
  SDL_RenderDrawLine(renderer_, GRAPH_X - 5, HEADER_H, GRAPH_X - 5, win_h_ - CONSOLE_H - PAD);

  // Console separator line
  int console_y = win_h_ - CONSOLE_H - PAD;
  SDL_SetRenderDrawColor(renderer_, BORDER_COL.r, BORDER_COL.g, BORDER_COL.b, 255);
  SDL_RenderDrawLine(renderer_, 0, console_y, win_w_, console_y);

  // IMU panel separator
  SDL_RenderDrawLine(renderer_, IMU_X - PAD, HEADER_H, IMU_X - PAD, win_h_ - CONSOLE_H - PAD);
}

// ─── draw_sensor_card ────────────────────────────────────────────────────────
void DashboardUI::draw_sensor_card(int x, int y, int w, int h,
                                   const char* label, float value, const char* unit, SensorStatus status)
{
  // Panel bg
  fill_rect(renderer_, x, y, w, h, PANEL_COLOR);

  // Status colour
  SDL_Color bar = GREEN;
  if (status == SensorStatus::WARN) bar = YELLOW;
  if (status == SensorStatus::CRIT) bar = RED;

  // Left accent bar
  fill_rect(renderer_, x, y, 3, h, bar);

  // Warning pulse bg (semi-transparent tint)
  if (status == SensorStatus::WARN) {
    SDL_SetRenderDrawColor(renderer_, YELLOW.r, YELLOW.g, YELLOW.b, 12);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(renderer_, &r);
  }

  // Label
  draw_text(label, x + 12, y + 8, DIM_TEXT);

  // Value
  char val_str[32];
  std::snprintf(val_str, sizeof(val_str), "%.1f", value);
  draw_text(val_str, x + 12, y + 28, bar, true);

  // Unit
  char unit_pos_x = x + 12 + (int)(std::strlen(val_str) * 12);
  draw_text(unit, x + 12 + (int)(std::strlen(val_str) * 12), y + 36, DIM_TEXT);

  // Status dot (top right)
  SDL_SetRenderDrawColor(renderer_, bar.r, bar.g, bar.b, 220);
  SDL_Rect dot = {x + w - 14, y + 10, 6, 6};
  SDL_RenderFillRect(renderer_, &dot);

  // Border
  draw_rect(renderer_, x, y, w, h, BORDER_COL);
}

// ─── draw_graph ──────────────────────────────────────────────────────────────
void DashboardUI::draw_graph(int x, int y, int w, int h,
                             const std::deque<float>& history, SDL_Color col, float lo, float hi)
{
  if (history.size() < 2) return;

  // Graph panel bg
  fill_rect(renderer_, x, y, w, h, {8, 11, 16, 255});
  draw_rect(renderer_, x, y, w, h, BORDER_COL);

  // Grid lines (3 horizontal)
  for (int g = 1; g <= 3; g++) {
    int gy = y + (h / 4) * g;
    SDL_SetRenderDrawColor(renderer_, 30, 37, 53, 120);
    SDL_RenderDrawLine(renderer_, x + 1, gy, x + w - 1, gy);
  }

  float range = (hi - lo < 0.001f) ? 1.0f : hi - lo;
  int n = (int)history.size();

  // Fill under the graph
  for (int i = 1; i < n; i++) {
    float x0 = x + (float)(i - 1) / (n - 1) * w;
    float x1 = x + (float)i       / (n - 1) * w;
    float y0 = y + h - (history[i-1] - lo) / range * h;
    float y1 = y + h - (history[i]   - lo) / range * h;

    SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 18);
    for (int fy = (int)std::min(y0, y1); fy < y + h; fy++)
      SDL_RenderDrawLine(renderer_, (int)x0, fy, (int)x1, fy);

    // Line on top
    SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 220);
    SDL_RenderDrawLine(renderer_, (int)x0, (int)y0, (int)x1, (int)y1);
  }

  // Current value dot at the right edge
  float last_val = history.back();
  int dot_y = y + h - (int)((last_val - lo) / range * h);
  SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 255);
  SDL_Rect dot = {x + w - 4, dot_y - 3, 6, 6};
  SDL_RenderFillRect(renderer_, &dot);
}

// ─── draw_graph_label ────────────────────────────────────────────────────────
void DashboardUI::draw_graph_label(int x, int y, const char* label, SDL_Color col)
{
  // Small label box
  fill_rect(renderer_, x, y - 2, (int)(std::strlen(label) * 8) + 10, 18, {10, 12, 18, 200});
  draw_text(label, x + 5, y, col);
}

// ─── draw_imu_bubble ─────────────────────────────────────────────────────────
void DashboardUI::draw_imu_bubble(int cx, int cy, int radius, float ix, float iy)
{
  // Draw concentric circles
  for (int r = radius; r > 0; r -= radius / 3) {
    SDL_SetRenderDrawColor(renderer_, BORDER_COL.r, BORDER_COL.g, BORDER_COL.b, 180);
    // Approximate circle with lines
    for (int deg = 0; deg < 360; deg += 4) {
      float a0 = deg       * M_PI / 180.0f;
      float a1 = (deg + 4) * M_PI / 180.0f;
      int x0 = cx + (int)(std::cos(a0) * r);
      int y0 = cy + (int)(std::sin(a0) * r);
      int x1 = cx + (int)(std::cos(a1) * r);
      int y1 = cy + (int)(std::sin(a1) * r);
      SDL_RenderDrawLine(renderer_, x0, y0, x1, y1);
    }
  }

  // Crosshair lines
  SDL_SetRenderDrawColor(renderer_, BORDER_COL.r, BORDER_COL.g, BORDER_COL.b, 120);
  SDL_RenderDrawLine(renderer_, cx - radius, cy, cx + radius, cy);
  SDL_RenderDrawLine(renderer_, cx, cy - radius, cx, cy + radius);

  // Tilt dot position
  int dot_x = cx + (int)(ix * (radius - 8));
  int dot_y = cy - (int)(iy * (radius - 8));

  // Glow ring
  SDL_SetRenderDrawColor(renderer_, CYAN.r, CYAN.g, CYAN.b, 40);
  SDL_Rect glow = {dot_x - 10, dot_y - 10, 20, 20};
  SDL_RenderFillRect(renderer_, &glow);

  // Dot
  SDL_SetRenderDrawColor(renderer_, CYAN.r, CYAN.g, CYAN.b, 230);
  SDL_Rect dot = {dot_x - 5, dot_y - 5, 10, 10};
  SDL_RenderFillRect(renderer_, &dot);
}

// ─── draw_console ────────────────────────────────────────────────────────────
void DashboardUI::draw_console(int x, int y, int w, int h, const Logger& log)
{
  fill_rect(renderer_, x, y, w, h, {8, 10, 14, 255});
  draw_rect(renderer_, x, y, w, h, BORDER_COL);

  // Console header
  fill_rect(renderer_, x, y, w, 20, {13, 16, 22, 255});
  draw_text("SERIAL CONSOLE", x + 10, y + 4, DIM_TEXT);
  SDL_SetRenderDrawColor(renderer_, BORDER_COL.r, BORDER_COL.g, BORDER_COL.b, 255);
  SDL_RenderDrawLine(renderer_, x, y + 20, x + w, y + 20);

  const auto& entries = log.entries();
  int line_y = y + h - 22;
  for (auto it = entries.rbegin(); it != entries.rend() && line_y > y + 20; ++it) {
    SDL_Color col = {50, 80, 100, 255};
    if (it->level == LogLevel::WARN) col = YELLOW;
    if (it->level == LogLevel::ERR)  col = RED;
    if (it->level == LogLevel::OK)   col = GREEN;

    std::string msg = "[" + it->timestamp + "] " + it->message;
    draw_text(msg.c_str(), x + 10, line_y, col);
    line_y -= 19;
  }
}

// ─── render ──────────────────────────────────────────────────────────────────
void DashboardUI::render(const SensorData& data, const Logger& log)
{
  // ── Clear + background ──
  SDL_SetRenderDrawColor(renderer_, 13, 15, 18, 255);
  SDL_RenderClear(renderer_);
  draw_background();

  // ── Header ──
  draw_text("ESP32  TELEMETRY  DASHBOARD", 16, 12, CYAN);

  // Connection status dot
  SDL_SetRenderDrawColor(renderer_, GREEN.r, GREEN.g, GREEN.b, 255);
  SDL_Rect status_dot = {win_w_ - 20, 17, 8, 8};
  SDL_RenderFillRect(renderer_, &status_dot);
  draw_text("LIVE", win_w_ - 56, 14, GREEN);

  // Frame counter (top right)
  static int frame = 0;
  char frame_str[32];
  std::snprintf(frame_str, sizeof(frame_str), "350ms | %d FPS", 60);
  draw_text(frame_str, win_w_ - 200, 14, DIM_TEXT);

  // ── Layout geometry ──
  int content_y  = HEADER_H + PAD;
  int content_h  = win_h_ - HEADER_H - CONSOLE_H - PAD * 3;
  int console_y  = win_h_ - CONSOLE_H - PAD;

  // ── Sensor Cards (left column) ──
  int card_y = content_y;
  draw_sensor_card(CARD_X, card_y, CARD_W, CARD_H,
                   "TEMPERATURE", data.temperature, "C", data.temp_status);

  card_y += CARD_H + CARD_GAP;
  draw_sensor_card(CARD_X, card_y, CARD_W, CARD_H,
                   "HUMIDITY", data.humidity, "%", data.hum_status);

  card_y += CARD_H + CARD_GAP;
  draw_sensor_card(CARD_X, card_y, CARD_W, CARD_H,
                   "DISTANCE", data.distance, "cm", data.dist_status);

  card_y += CARD_H + CARD_GAP;
  draw_sensor_card(CARD_X, card_y, CARD_W, CARD_H,
                   "IMU Z-AXIS", data.imu.z, "g", SensorStatus::OK);

  // IMU X/Y sub-label
  char imu_sub[32];
  std::snprintf(imu_sub, sizeof(imu_sub), "X:%.2f  Y:%.2f", data.imu.x, data.imu.y);
  draw_text(imu_sub, CARD_X + 12, card_y + CARD_H + 6, DIM_TEXT);

  // ── Graphs (centre column) ──
  int graph_y  = content_y;
  int g_gap    = 8;
  int g_h      = (content_h - g_gap * 2) / 3;

  // TEMP graph
  draw_graph(GRAPH_X, graph_y, GRAPH_W, g_h,
             data.temp_history, CYAN, -10.0f, 80.0f);
  draw_graph_label(GRAPH_X + 6, graph_y + 4, "TEMP C", CYAN);

  graph_y += g_h + g_gap;

  // HUM graph
  draw_graph(GRAPH_X, graph_y, GRAPH_W, g_h,
             data.hum_history, GREEN, 0.0f, 100.0f);
  draw_graph_label(GRAPH_X + 6, graph_y + 4, "HUM %", GREEN);

  graph_y += g_h + g_gap;

  // DIST graph
  draw_graph(GRAPH_X, graph_y, GRAPH_W, g_h,
             data.dist_history, PURPLE, 0.0f, 400.0f);
  draw_graph_label(GRAPH_X + 6, graph_y + 4, "DIST cm", PURPLE);

  // ── IMU Bubble (right column) ──
  int imu_panel_h = content_h / 2;
  fill_rect(renderer_, IMU_X, content_y, IMU_W, imu_panel_h, PANEL_COLOR);
  draw_rect(renderer_, IMU_X, content_y, IMU_W, imu_panel_h, BORDER_COL);
  draw_text("IMU TILT", IMU_X + 10, content_y + 8, DIM_TEXT);

  int bubble_cx = IMU_X + IMU_W / 2;
  int bubble_cy = content_y + imu_panel_h / 2 + 10;
  int bubble_r  = std::min(IMU_W, imu_panel_h) / 2 - 20;
  draw_imu_bubble(bubble_cx, bubble_cy, bubble_r, data.imu.x, data.imu.y);

  // IMU values text below bubble
  char imu_txt[64];
  std::snprintf(imu_txt, sizeof(imu_txt), "X: %+.2f", data.imu.x);
  draw_text(imu_txt, IMU_X + 10, content_y + imu_panel_h - 52, CYAN);
  std::snprintf(imu_txt, sizeof(imu_txt), "Y: %+.2f", data.imu.y);
  draw_text(imu_txt, IMU_X + 10, content_y + imu_panel_h - 36, CYAN);
  std::snprintf(imu_txt, sizeof(imu_txt), "Z: %+.2f", data.imu.z);
  draw_text(imu_txt, IMU_X + 10, content_y + imu_panel_h - 20, CYAN);

  // ── UART Packet panel (below IMU bubble) ──
  int pkt_y = content_y + imu_panel_h + PAD;
  int pkt_h = content_h - imu_panel_h - PAD;
  fill_rect(renderer_, IMU_X, pkt_y, IMU_W, pkt_h, PANEL_COLOR);
  draw_rect(renderer_, IMU_X, pkt_y, IMU_W, pkt_h, BORDER_COL);
  draw_text("UART PACKET", IMU_X + 10, pkt_y + 8, DIM_TEXT);

  // Simulated packet string
  char pkt[128];
  std::snprintf(pkt, sizeof(pkt), "TEMP:%.1f", data.temperature);
  draw_text(pkt, IMU_X + 10, pkt_y + 28, {39, 100, 60, 255});
  std::snprintf(pkt, sizeof(pkt), "HUM:%.1f", data.humidity);
  draw_text(pkt, IMU_X + 10, pkt_y + 46, {39, 100, 60, 255});
  std::snprintf(pkt, sizeof(pkt), "DIST:%.0f", data.distance);
  draw_text(pkt, IMU_X + 10, pkt_y + 64, {39, 100, 60, 255});
  std::snprintf(pkt, sizeof(pkt), "IMU:%.2f,%.2f,%.2f", data.imu.x, data.imu.y, data.imu.z);
  draw_text(pkt, IMU_X + 10, pkt_y + 82, {39, 100, 60, 255});

  // ── Console ──
  draw_console(PAD, console_y, win_w_ - PAD * 2, CONSOLE_H, log);

  SDL_RenderPresent(renderer_);
}
