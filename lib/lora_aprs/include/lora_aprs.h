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
    void init(String, String, String);
};

#endif // LORA_APRS_H