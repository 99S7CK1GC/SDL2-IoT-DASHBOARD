#pragma once

#include <string>
#include <deque> //here we use deque to push and pop the front and back of the element
#include <cstdint>

struct IMUdata { float x, y, z};

enum class status {OK, WARN, CRIT };

struct sensordata
{
  float        temperature; 
  float        humidity;
  float        distance; 
  
  IMUdata      imu;
  
  status       temp_status;
  status       hum_status;
  status       dist_status;
  
  std::deque<float> temp_history;
  std::deque<float> hum_history;
  std::deque<float> dist_history;

  static constexpr size_t HISTORY_LEN = 80; //here HISTORY LENGTH means to save the last 80 datas 
}

