#include "lora_aprs.h"

LoRaAPRS::LoRaAPRS() : gps(nullptr), loraInitialized(false), aprsClient(LoRa_Utils::getRadio())
{
    loraInitialized = LoRa_Utils::setup();
}

void LoRaAPRS::init(String _callsign, String _symbol, String _status = "")
{
    Utils::setCallsign(_callsign);
    Utils::setSymbol(_symbol);
    Utils::setStatus(_status);

    loraInitialized = LoRa_Utils::setup();
    aprsClient = APRSClient(LoRa_Utils::getRadio());
    aprsClient.begin(_symbol.charAt(0), _callsign.c_str(), 1, false);
}

bool LoRaAPRS::send_location()
{
    if (!loraInitialized)
    {
        return false;
    }

    if (gps->location.isValid())
    {

        LoRa_Utils::changeFreq();
        char *lat = (char *)String(String(gps->location.lat()) + ((gps->location.lat() > 0) ? "N" : "S")).c_str();
        char *lon = (char *)String(String(gps->location.lng()) + ((gps->location.lng() > 0) ? "E" : "W")).c_str();
        char *dest = (char *)String("GPS").c_str();
        char *status = (char *)Utils::getStatus().c_str();
        char *time = (char *)_get_time().c_str();
        int state = aprsClient.sendPosition(dest, 1, lat, lon, status, time);
        if (state == RADIOLIB_ERR_NONE)
        {
            // Serial.println(F("success!"));
        }
        else
        {
            Serial.print(F("Tx failed, code "));
            Serial.println(state);
        }
        return state == RADIOLIB_ERR_NONE;
    }
    else
    {
        Serial.println("GPS location not valid. Cannot send APRS packet.");
        return false;
    }
}

String LoRaAPRS::_get_time()
{
    if (gps->time.isValid())
    {
        return String(gps->time.hour()) + String(gps->time.minute()) + String(gps->time.second()) + "z";
    }
    else
    {
        return "0z";
    }
}