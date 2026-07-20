#include "lora_aprs.h"

LoRaAPRS::LoRaAPRS(TinyGPSPlus *gps) : gps(gps), callsign(""), symbol(""), status("") {}

void LoRaAPRS::init(const string &callsign, const string &symbol, const string &status)
{
    this->callsign = callsign;
    this->symbol = symbol;
    this->status = status;
}

bool LoRaAPRS::send_position_report()
{
    return false;
}