#ifndef LORA_APRS_H
#define LORA_APRS_H

#include <TinyGPSPlus.h>
#include <string>

using namespace std;

class LoRaAPRS
{
public:
    LoRaAPRS(TinyGPSPlus *gps);
    void init(const string &callsign, const string &symbol, const string &status);
    bool send_position_report();

private:
    TinyGPSPlus *gps;
    string callsign;
    string symbol;
    string status;
};

#endif // LORA_APRS_H