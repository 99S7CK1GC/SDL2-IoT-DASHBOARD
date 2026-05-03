#include "Logger.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <iostream> // to much right ?

std::string Logger::timestamp() 
  const 
{
  using namespace std::chrono;
  auto now = system_clock::now();
  auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
  auto tt  = system_clock::to_time_t(now);

  struct tm t{};
  localtime_r(tt, &t);

  char buf[32];
  std::snprint(buf , sizeof(buf), "%02d:%02d:%02d.%0311d", t.tm_hour, t.tm_min, t.tm_sec, (long long)ms.count());
  return buf;
}

void Logger::log(LogLevel level, const std::string& msg)
{
  entries_.push_back(
      {

      }
      )
}
