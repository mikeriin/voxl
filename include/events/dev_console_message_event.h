#ifndef VOXL_DEV_CONSOLE_MESSAGE_EVENT_H
#define VOXL_DEV_CONSOLE_MESSAGE_EVENT_H


#include <string>


enum DebugLevel
{
  NONE,
  INFO,
  WARNING,
  ERROR,
  FATAL
};


inline std::string GetDebugLevel(const DebugLevel& level)
{
  std::string levelStr;
  switch (level) 
  {
    case INFO:
      levelStr = "INFO";
    break;

    case WARNING:
      levelStr = "WARNING";
    break;

    case ERROR:
      levelStr = "ERROR";
    break;

    case FATAL:
      levelStr = "FATAL";
    break;

    default:
    break;
  }
  return levelStr;
}


struct DevConsoleMessageEvent
{
  const char* name = "DEV_CONSOLE_MESSAGE_EVENT";
  DebugLevel level;
  std::string buffer;
};


#endif // !VOXL_DEV_CONSOLE_MESSAGE_EVENT_H