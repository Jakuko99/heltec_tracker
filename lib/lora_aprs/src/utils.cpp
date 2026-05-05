#include <APRSPacketLib.h>
#include "board_pinout.h"
#include "lora_utils.h"
#include "utils.h"

extern uint32_t lastTx;
extern uint32_t lastTxTime;

extern bool displayEcoMode;
extern uint32_t displayTime;
extern bool displayState;
extern int menuDisplay;
extern String versionDate;
extern bool flashlight;

extern bool statusUpdate;

uint32_t statusTime = millis();
uint8_t wxModuleAddress = 0x00;
uint8_t keyboardAddress = 0x00;
uint8_t touchModuleAddress = 0x00;
String path = "WIDE1-1";
String callsign = "NOCALL";
String symbol = "[";
String status = "";

namespace Utils
{

    static char locator[11]; // letterize and getMaidenheadLocator functions are Copyright (c) 2021 Mateusz Salwach - MIT License

    static char letterize(int x)
    {
        return (char)x + 65;
    }

    char *getMaidenheadLocator(double lat, double lon, uint8_t size)
    {
        double LON_F[] = {20, 2.0, 0.083333, 0.008333, 0.0003472083333333333};
        double LAT_F[] = {10, 1.0, 0.0416665, 0.004166, 0.0001735833333333333};
        int i;
        lon += 180;
        lat += 90;

        if (size <= 0 || size > 10)
            size = 6;
        size /= 2;
        size *= 2;

        for (i = 0; i < size / 2; i++)
        {
            if (i % 2 == 1)
            {
                locator[i * 2] = (char)(lon / LON_F[i] + '0');
                locator[i * 2 + 1] = (char)(lat / LAT_F[i] + '0');
            }
            else
            {
                locator[i * 2] = letterize((int)(lon / LON_F[i]));
                locator[i * 2 + 1] = letterize((int)(lat / LAT_F[i]));
            }
            lon = fmod(lon, LON_F[i]);
            lat = fmod(lat, LAT_F[i]);
        }
        locator[i * 2] = 0;
        return locator;
    }

    static String padding(unsigned int number, unsigned int width)
    {
        String result;
        String num(number);
        if (num.length() > width)
            width = num.length();
        for (unsigned int i = 0; i < width - num.length(); i++)
        {
            result.concat('0');
        }
        result.concat(num);
        return result;
    }

    /*String createDateString(time_t t)
    {
        String dateString = padding(year(t), 4);
        dateString += "-";
        dateString += padding(month(t), 2);
        dateString += "-";
        dateString += padding(day(t), 2);
        return dateString;
    }

    String createTimeString(time_t t)
    {
        String timeString = padding(hour(t), 2);
        timeString += ":";
        timeString += padding(minute(t), 2);
        timeString += ":";
        timeString += padding(second(t), 2);
        return timeString;
    }*/

    void checkStatus()
    {
        if (statusUpdate)
        {
            uint32_t currentTime = millis();
            uint32_t statusTx = currentTime - statusTime;
            lastTx = currentTime - lastTxTime;
            if (statusTx > 10 * 60 * 1000 && lastTx > 10 * 1000)
            {
                LoRa_Utils::sendNewPacket(APRSPacketLib::generateStatusPacket(callsign, "APLRT1", path, status));
                statusUpdate = false;
            }
        }
    }

    void setCallsign(String _callsign)
    {
        callsign = _callsign;
    }

    void setStatus(String _status)
    {
        status = _status;
    }

    void setSymbol(String _symbol)
    {
        symbol = _symbol;
    }

    String getCallsign()
    {
        return callsign;
    }

    String getStatus()
    {
        return status;
    }

    String getSymbol()
    {
        return symbol;
    }

    String getPath()
    {
        return path;
    }
}