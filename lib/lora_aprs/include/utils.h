#ifndef UTILS_H_
#define UTILS_H_

#include <Arduino.h>

namespace Utils
{
    char *getMaidenheadLocator(double lat, double lon, uint8_t size);
    String createDateString(time_t t);
    String createTimeString(time_t t);
    void checkStatus();
    void setCallsign(String _callsign);
    void setStatus(String _status);
    void setSymbol(String _symbol);

    String getCallsign();
    String getStatus();
    String getSymbol();
    String getPath();
}

#endif