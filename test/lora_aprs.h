#ifndef LORA_APRS_H
#define LORA_APRS_H

#include <TinyGPSPlus.h>
#include <RadioLib.h>
#include <APRSPacketLib.h>
#include <SPI.h>
#include <string>

#define RADIO_SCLK_PIN 9
#define RADIO_MISO_PIN 11
#define RADIO_MOSI_PIN 10

#define LORA_FREQ 869618000
#define LORA_SF 8
#define LORA_BW 62500
#define LORA_CR 5
#define LORA_POWER 10 // values available: 10, 17, 22

using namespace std;

class LoRaAPRS
{
public:
    LoRaAPRS(TinyGPSPlus *gps, SX1262 *radio);
    bool init(const string &callsign, const string &symbol, const string &status);
    bool send_position_report();

private:
    void change_freq();
    void set_flag();
    string get_time();

    TinyGPSPlus *gps;
    SX1262 *radio;
    APRSClient aprsClient;
    string callsign;
    string symbol;
    string status;
    bool operation_done;
    bool lora_initialized;
};

#endif // LORA_APRS_H