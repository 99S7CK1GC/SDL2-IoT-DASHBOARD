#pragma once 

#include <string>
#include <deque>

enum class LogLevel
{
  INFO, OK, WARN, ERR
};

struct LogEntry
{
  std::string timestamp;
  std::string message;
  LogLevel level;
};

class Logger
{
  public:
    void log(LogLevel level, const std::string& msg);
    void log_packet(const std::string& pkt);
    const std::deque<LogEntry>& entries() 
      const
    {
      return entries_;
    }
  private:
    std::deque<LogEntry> entries_;
    static constexpr size_t MAX_ENTRIES = 50;
    std::string timestamp() const;
};
