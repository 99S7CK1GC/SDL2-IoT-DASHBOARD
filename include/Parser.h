#pragma once

#include "SensorData.h"
#include <string>

class Parser
{
  public:
    std::string encode(const SensorData& d) const;

    bool   decode(const std::string& pkt, SensorData& out) const;
};
