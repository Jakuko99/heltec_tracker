#ifndef LORA_UTILS_H_
#define LORA_UTILS_H_

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

#include "board_pinout.h"

struct ReceivedLoRaPacket
{
    String text;
    int rssi;
    float snr;
    int freqError;
};

namespace LoRa_Utils
{
    void setFlag();
    void changeFreq();
    bool setup();
    void sendNewPacket(const String &newPacket);
    void wakeRadio();
    ReceivedLoRaPacket receiveFromSleep();
    ReceivedLoRaPacket receivePacket();
    void sleepRadio();
    SX1262 *getRadio();
}

#endif