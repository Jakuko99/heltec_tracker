#include "lora_utils.h"

uint8_t loraIndex = 1;
int loraIndexSize = 1;

bool operationDone = true;
bool transmitFlag = true;

#if defined(HAS_SX1262)
LoraType *currentLoRaType = new LoraType{869618000, 8, 62500, 5, 10}; // default LoRa settings for SX1262
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
#endif

namespace LoRa_Utils
{
    SX1262 *getRadio()
    {
        return &radio;
    }

    void setFlag(void)
    {
        operationDone = true;
    }

    void changeFreq()
    {
        float freq = (float)currentLoRaType->frequency / 1000000;
        radio.setFrequency(freq);
        radio.setSpreadingFactor(currentLoRaType->spreadingFactor);
        float signalBandwidth = currentLoRaType->signalBandwidth / 1000;
        radio.setBandwidth(signalBandwidth);
        radio.setCodingRate(currentLoRaType->codingRate4);
#if (defined(HAS_SX1268) || defined(HAS_SX1262)) && !defined(HAS_1W_LORA)
        radio.setOutputPower(currentLoRaType->power + 2); // values available: 10, 17, 22 --> if 20 in tracker_conf.json it will be updated to 22.
#endif

        String loraCountryFreq = "EU/UK";
        String currentLoRainfo = "LoRa ";
        currentLoRainfo += loraCountryFreq;
        currentLoRainfo += " / Freq: ";
        currentLoRainfo += String(currentLoRaType->frequency);
        currentLoRainfo += " / SF:";
        currentLoRainfo += String(currentLoRaType->spreadingFactor);
        currentLoRainfo += " / CR: ";
        currentLoRainfo += String(currentLoRaType->codingRate4);
    }

    bool setup()
    {

#if defined(LIGHTTRACKER_PLUS_1_0)
        loraSPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);
#else
        SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
#endif
        float freq = (float)currentLoRaType->frequency / 1000000;
        int state = radio.begin(freq);
        if (state == RADIOLIB_ERR_NONE)
        {
#if defined(HAS_SX1262) || defined(HAS_SX1268)
            // logger.log(logging::LoggerLevel::LOGGER_LEVEL_INFO, "LoRa", "Initializing SX126X ...");
#endif
        }
        else
        {
            /// logger.log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, "LoRa", "Starting LoRa failed! State: %d", state);
            while (true)
                ;
        }
#if defined(HAS_SX1262) || defined(HAS_SX1268) || defined(HAS_LLCC68)
        radio.setDio1Action(setFlag);
#endif
        radio.setSpreadingFactor(currentLoRaType->spreadingFactor);
        float signalBandwidth = currentLoRaType->signalBandwidth / 1000;
        radio.setBandwidth(signalBandwidth);
        radio.setCodingRate(currentLoRaType->codingRate4);
        radio.setCRC(true);

#if (defined(HAS_SX1268) || defined(HAS_SX1262)) && !defined(HAS_1W_LORA)
        state = radio.setOutputPower(currentLoRaType->power + 2); // values available: 10, 17, 22 --> if 20 in tracker_conf.json it will be updated to 22.
        radio.setCurrentLimit(140);
#endif

#if defined(HAS_SX1262) || defined(HAS_SX1268) || defined(HAS_LLCC68)
        radio.setRxBoostedGainMode(true);
#endif
        return (state == RADIOLIB_ERR_NONE);
    }

    void sendNewPacket(const String &newPacket)
    {
        /*if (Config.ptt.active)
        {
            digitalWrite(Config.ptt.io_pin, Config.ptt.reverse ? LOW : HIGH);
            delay(Config.ptt.preDelay);
        }*/
        int state = radio.transmit("\x3c\xff\x01" + newPacket);
        transmitFlag = true;
        if (state == RADIOLIB_ERR_NONE)
        {
            // Serial.println(F("success!"));
        }
        else
        {
            Serial.print(F("Tx failed, code "));
            Serial.println(state);
        }
    }

    void wakeRadio()
    {
        radio.startReceive();
    }

    ReceivedLoRaPacket receiveFromSleep()
    {
        ReceivedLoRaPacket receivedLoraPacket;
        String packet = "";
        int state = radio.readData(packet);
        if (state == RADIOLIB_ERR_NONE)
        {
            receivedLoraPacket.text = packet;
            receivedLoraPacket.rssi = radio.getRSSI();
            receivedLoraPacket.snr = radio.getSNR();
            receivedLoraPacket.freqError = radio.getFrequencyError();
        }
        else
        {
            //
        }
        return receivedLoraPacket;
    }

    ReceivedLoRaPacket receivePacket()
    {
        ReceivedLoRaPacket receivedLoraPacket;
        String packet = "";
        if (operationDone)
        {
            operationDone = false;
            if (transmitFlag)
            {
                radio.startReceive();
                transmitFlag = false;
            }
            else
            {
                int state = radio.readData(packet);
                if (state == RADIOLIB_ERR_NONE)
                {
                    if (!packet.isEmpty())
                    {
                        receivedLoraPacket.text = packet;
                        receivedLoraPacket.rssi = radio.getRSSI();
                        receivedLoraPacket.snr = radio.getSNR();
                        receivedLoraPacket.freqError = radio.getFrequencyError();
                    }
                }
                else
                {
                    Serial.print(F("Rx failed, code ")); // 7 = CRC mismatch
                    Serial.println(state);
                }
            }
        }
        return receivedLoraPacket;
    }

    void sleepRadio()
    {
        radio.sleep();
    }
}