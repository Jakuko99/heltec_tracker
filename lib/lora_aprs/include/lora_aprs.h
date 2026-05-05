#ifndef LORA_APRS_H
#define LORA_APRS_H

#include <Arduino.h>

#include "board_pinout.h"
#include "lora_utils.h"
#include "msg_utils.h"
#include "utils.h"

class LoRaAPRS
{
public:
    LoRaAPRS();
    void init(String, String, String);
    void assign_gps(TinyGPSPlus *_gps) { this->gps = _gps; }
    bool send_location();

private:
    TinyGPSPlus *gps;
    APRSClient aprsClient;
    bool loraInitialized;
    String _get_time();
};

#endif // LORA_APRS_H