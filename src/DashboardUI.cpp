#include "DashboardUI.h"

#include <iostream>
#include <algorithm>
#include <cstdio>

static constexpr SDL_Color BG_COLOR     = {13, 15, 18, 255};
static constexpr SDL_Color PANEL_COLOR  = {19, 23, 32, 255};
static constexpr SDL_Color BORDER_COL   = {30, 37, 53, 255};
static constexpr SDL_Color CYAN         = {0, 229, 255, 255};
static constexpr SDL_Color GREEN        = {57, 255, 20, 255};
static constexpr SDL_Color YELLOW       = {255, 214, 0, 255};
static constexpr SDL_Color RED          = {255, 68, 68, 255};
static constexpr SDL_Color DIM_TEXT     = {90, 106, 126, 255};
static constexpr SDL_Color BODY_TEXT    = {176, 190, 197, 255};

DashboardUI::DashboardUI(int w, int h) : win_w_(w), win_h_(h)
{
 
  window_ = SDL_CreateWindow
    (
      "IoT Telemetry Dashboard",
      SDL_WINDOWPOS_CENETERED, SDL_WINDOWPOS_CENETERED,
      w,
      h,
      SDL_WINDOW_SHOWN
    );

  renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
  
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  if(TTF_Init() == -1)
  {
    std::cerr <<"TTF_Init Error: " << TTF_GetError() << "\n";
  }
  font_ = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16);

    if(!font_)
    {
      std::cerr << "Font Load Error: " << TTF_GetError() << "\n";
    }
}
DashboardUI::~DashboardUI()
{
  if(font_) 
  TTF_CloseFont(font_);
  if(renderer_)
  SDL_DestroyRenderer(renderer_);
  if(window_)
  SDL_DestroyWindow(window_);
}

bool DashboardUI::poll_events()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    if(e.type == SDL_QUIT)
      running_ = false;
  }
  return running_;
}
void DashboardUI::draw_text(const char* txt, int x, int y, SDL_Color col, bool bold)
{
  if(!font_ || !txt[0])
    return;

  SDL_Surface* surf = TTF_RenderText_Blended(font_, txt, col);
  SDL_Texture* tex  = SDL_CreateTextureFromSurface(renderer_, surf);

  SDL_Rect dst = { x, y, surf -> w, surf -> h };
  SDL_RenderCopy(renderer_, tex, nullptr, &dst);

  SDL_FreeSurface(surf);
  SDL_DestroyTexture(tex);

}

void DashboardUI::draw_sensor_card(int x, int y, int w, int h, const char* label, float value, const char* unit, SensorStatus status)
{

  SDL_SetRenderDrawColor(renderer_, 19, 23, 32, 255);
  SDL_Rect r = {x, y, w, h};
  SDL_RenderFillRect(renderer_, &r);

  SDL_Color bar = GREEN;
  if (status == SensorStatus::WARN)
    bar = YELLOW;
  if (status == SensorStatus::CRIT)
    bar = RED;
  
  SDL_SetRenderDrawColor(renderer_, bar.r, bar.g, bar.b, 255);
  SDL_Rect accent = {x, y, 4, h};
  SDL_RenderFillRect(renderer_, &accent);

  draw_text(label, x + 12, y + 10, DIM_TEXT);
  char val_str[32];
  std::snprintf(val_str, sizeof(val_str), "%.1f %s", value, unit);
  draw_text(val_str, x + 12, y + 34, bar, true);

  SDL_SetRenderDrawColor(renderer_, 30, 37, 53, 255);
  SDL_RenderDrawRect(renderer_, &r);

}

void DashboardUI::draw_graph(int x, int y, int w, int h, const std::deque<float>& history, SDL_Color col, float lo, float hi)
{
  if (history.size() < 2)
    return;

  float range = (hi - lo == 0.0f) ? 1.0f : hi - lo;
  int n = (int)history.size();

  for (int i = 1; i < n; i++)
  {
    float x0 = x + (float)(i - 1) / (n - 1) * w;
    float x1 = x + (float)i / (n - 1) * w;
    float y0 = y + h - (history[i - 1] - lo) / range * h;
    float y1 = y + h - (history[i] - lo) / range * h;
  
  SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 20);
  for (int fy = (int)std::min(y0, y1); fy < y + h; fy++)
  {
    SDL_RenderDrawLine(renderer_, (int)x0, fy, (int)x1, fy);
  }

  SDL_SetRenderDrawColor(renderer_, col.r, col.g, col.b, 200);
  SDL_RenderDrawLine(renderer_, (int)x0, (int)y0, (int)x1, (int)y1);
  
  }
}

void DashboardUI::draw_console(int x, int y, int w, int h, const Logger& log)
{
  SDL_SetRenderDrawColor(renderer_, 13, 15, 18, 255);
  SDL_Rect r = {x, y, w, h};
  SDL_RenderFillRect(renderer_, &r);
  SDL_SetRenderDrawColor(renderer_, 30, 37, 53, 255);
  SDL_RenderDrawRect(renderer_, &r);

  const auto& entries = log.entries();
  int line_y = y + h - 25;
  for (auto it = entries.rbegin(); it != entries.rend() && line_y > y; ++it)
  {
    SDL_Color col = BODY_TEXT;
    if (it -> level == LogLevel::WARN)
      col = YELLOW;
    if (it -> level == LogLevel::ERR)
      col = RED;

    std::string full_msg = "[" + it -> timestamp + "] " + it -> message;
    draw_text(full_msg.c_str(), x + 10, line_y, col);
    line_y -= 22; 
  }
}

void DashboardUI::render(const SensorData& data, const Logger& log)
{
  SDL_SetRenderDrawColor(renderer_, 13, 15, 18, 255);
  SDL_RenderClear(renderer_);

  draw_text("EMBEDDED TELEMETRY MONITOR V1.0", 20, 10, CYAN, true);

  draw_sensor_card(20, 60, 290, 80, "TEMPERATURE", data.temperature, "C", data.temp_status);
  draw_sensor_card(20, 150, 290, 80, "HUMIDIRY", data.humidity, "%", data.hum_status);
  draw_sensor_card(20, 240, 290, 80, "DISTANCE", data.distance, "cm", data.distance_status);
  
  draw_graph(20, 160, 910, 150, data.temp_history, CYAN, 10.0f, 50.0f);
  draw_graph(20, 330, 910, 150, data.dist_history, GREEN, 0.0f, 400.0f);

  draw_console(20, 520, 1240, 180, log);

  SDL_RenderPresent(renderer_);
  
}
