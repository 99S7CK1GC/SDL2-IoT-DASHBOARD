#pragma once

#include "SensorData.h"
#include "Logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <deque>

class DashboardUI
{
public:
  DashboardUI(int w, int h);
  ~DashboardUI();

  bool poll_events();
  void render(const SensorData& data, const Logger& log);

private:
  void draw_background();
  void draw_sensor_card(int x, int y, int w, int h,
                        const char* label, float value,
                        const char* unit, SensorStatus status);
  void draw_graph(int x, int y, int w, int h,
                  const std::deque<float>& history,
                  SDL_Color color, float lo, float hi);
  void draw_graph_label(int x, int y, const char* label, SDL_Color col);
  void draw_imu_bubble(int cx, int cy, int radius, float ix, float iy);
  void draw_console(int x, int y, int w, int h, const Logger& log);
  void draw_text(const char* txt, int x, int y, SDL_Color col, bool large = false);

  SDL_Window*   window_   = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  TTF_Font*     font_     = nullptr;
  TTF_Font*     font_lg_  = nullptr;

  int  win_w_, win_h_;
  bool running_ = true;
};
