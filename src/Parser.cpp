

#include "Parser.h"
#include <sstream>
#include <cstdio>

//encode
std::string Parser::encode(const SensorData& d)
  const
{
  // to hold the text we are about to create
  char buf[128];
  
  // string, size, format
  std::snprintf(buf, sizeof(buf),"TEMP:%1.f; HUM:%.1f;DIST:%.0f;IMU:%.2f,%.2f,%2.f\n",d.temperature, d.humidity, d.distance, d.imu.x, d.imu.y, d.imu.z);
  return buf;
}
//decode

bool Parser::decode(const std::string& pkt, SensorData& out)
  const 
{
  // we use istringstream to stream the string into different variable in a sequence of characters 
  std::istringstream ss(pkt);
  std::string token;
  bool ok = false;

  // we use getline to read a string from an input stream
  while (std::getline(ss, token, ";"))
  {
    auto sep = token.find(':');
    if(sep == std::string::npos)
      continue;
    std::string key = token.substr(0, sep);
    std::string val = token.substr(sep + 1);
    
    if (key == "TEMP")
    {
      out.temperature = std::stof(val);
      ok = true;
    }
    else if (key == "HUM") 
    {
      out.humidity = std::stof(val);
    }
    else if (key == "DIST")
    {
      out.distance = std::stof(val);
    }
    else if (key == "IMU")
    {
      std::sscanf(val.c_str(), "%f, %f, %f,",&out.imu.x, &out.imu.y, &out.imu.z);
    }
  }
  return ok;
}

