#include "SensorData.h"
#include "SensorSimulator.h"
#include "Parser.h"
#include "DashboardUI.h"
#include "Logger.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

int main()
{
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  SensorData         data{};
  SensorSimulator    sim;
  Parser             Parser;
  Logger             Logger;
  DashboardUI        ui(1280, 720);

  data.temperature = 24.0f;
  data.humidity    = 58.0f;
  data.distance    = 120.f;
  data.imu         = {0.0f, 0.0f, -1.0f};

  const int   TARGET_FPS     = 60;
  const int   SENSOR_MS      = 350;
  const Uint32 FRAME_MS      = 1000 / TARGET_FPS;

  Uint32 last_sensor_time = SDL_GetTicks();

  while (ui.poll_events())
  {
    Uint32 frame_start = SDL_GetTicks();

    if(SDL_GetTicks() - last_sensor_time >= SENSOR_MS)
    {
      sim.update(data);

      std::string pkt = parser.encode(data);
      logger.log_packet(pkt);

      if(data.temp_status == SensorStatus::WARN)
        logger.log(LogLevel::WARN, "Temperature out of range")
 
      if(data.dist_status == SensorStatus::WARN)
        logger.log(LogLevel::WARN, "Distance spike detected");

      lost _sensor_time = SDL_GetTicks();
  
    }
    
    ui.render(data, logger);

    Uint32 elapsed = SDL_GetTicks() - frame_start;
    if (elapsed < FRAME_MS)
      SDL_Delay(FRAME_MS - elapsed);
  }
TTF_Quit();
SDL_Quit();
return 0;

  }
