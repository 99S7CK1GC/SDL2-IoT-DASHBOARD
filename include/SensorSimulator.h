#pragma once

#include "SensorData.h"

class SensorSimulator 
{
  public 
    SensorSimulator();
    void update(sensordata& data);

  private:
    float drift(float val, float speed, float lo, float hi);
    float noise(float amplitude);
    bool should_spike();

    int spike_cooldown_ = 0;
};
