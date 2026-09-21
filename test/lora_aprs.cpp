#include "lora_aprs.h"

LoRaAPRS::LoRaAPRS(TinyGPSPlus *gps, SX1262 *radio) : gps(gps), radio(radio), callsign(""), symbol(""), status(""), aprsClient(radio), operation_done(false), lora_initialized(false) {}

bool LoRaAPRS::init(const string &callsign, const string &symbol, const string &status)
{
    this->callsign = callsign;
    this->symbol = symbol;
    this->status = status;

    SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
    float freq = (float)LORA_FREQ / 1000000;
    int state = radio->begin(freq);

    // Set LoRa parameters
    // radio->setDio1Action(LoRaAPRS::set_flag);
    radio->setSpreadingFactor(LORA_SF);
    float signalBandwidth = LORA_BW / 1000;
    radio->setBandwidth(signalBandwidth);
    radio->setCodingRate(LORA_CR);
    radio->setCRC(true);
    state = radio->setOutputPower(LORA_POWER + 2); // values available: 10, 17, 22 --> if 20 in tracker_conf.json it will be updated to 22.
    radio->setCurrentLimit(140);
    radio->setRxBoostedGainMode(true);

    lora_initialized = (state == RADIOLIB_ERR_NONE);

    aprsClient.begin(symbol[0], callsign.c_str(), 1, false);

    return state == RADIOLIB_ERR_NONE;
}

bool LoRaAPRS::send_position_report()
{
    if (!lora_initialized)
    {
        return false;
    }

    if (gps->location.isValid())
    {
        change_freq();
        char *lat = (char *)String(String(gps->location.lat()) + ((gps->location.lat() > 0) ? "N" : "S")).c_str();
        char *lon = (char *)String(String(gps->location.lng()) + ((gps->location.lng() > 0) ? "E" : "W")).c_str();
        char *dest = (char *)String("GPS").c_str();
        char *status = (char *)this->status.c_str();
        char *time = (char *)this->get_time().c_str();
        int state = this->aprsClient.sendPosition(dest, 1, lat, lon, status, time);

        return state == RADIOLIB_ERR_NONE;
    }
    else
    {
        return false;
    }
}

void LoRaAPRS::change_freq()
{
    float freq = (float)LORA_FREQ / 1000000;
    radio->setFrequency(freq);
    radio->setSpreadingFactor(LORA_SF);
    float signalBandwidth = LORA_BW / 1000;
    radio->setBandwidth(signalBandwidth);
    radio->setCodingRate(LORA_CR);
    radio->setOutputPower(LORA_POWER);
}

void LoRaAPRS::set_flag()
{
    operation_done = true;
}

string LoRaAPRS::get_time()
{
    if (gps->time.isValid())
    {
        return to_string(gps->time.hour()) + to_string(gps->time.minute()) + to_string(gps->time.second()) + "z";
    }
    else
    {
        return "";
    }
}