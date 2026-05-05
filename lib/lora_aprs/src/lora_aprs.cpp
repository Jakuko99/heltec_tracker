#include "lora_aprs.h"

void LoRaAPRS::init(String _callsign, String _symbol, String _status = "")
{
    Utils::setCallsign(_callsign);
    Utils::setSymbol(_symbol);
    Utils::setStatus(_status);

    loraInitialized = LoRa_Utils::setup();
    aprsClient = APRSClient(&LoRa_Utils::getRadio());
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
        char* lat = (char*)String(gps->location.lat()).c_str();
        char* lon = (char*)String(gps->location.lng()).c_str();
        char* callsign = (char*)Utils::getCallsign().c_str();
        char* status = (char*)Utils::getStatus().c_str();
        char* time = (char*)String(gps->time.value()).c_str();
        int state = aprsClient.sendPosition(callsign, 1, lat, lon, status, time);
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
/*
void loop()
{
    currentBeacon = &Config.beacons[myBeaconsIndex];
    if (statusUpdate)
    {
        if (APRSPacketLib::checkNocall(currentBeacon->callsign))
        {
            logger.log(logging::LoggerLevel::LOGGER_LEVEL_ERROR, "Config", "Change your callsigns in WebConfig");
            displayShow("ERROR", "Callsigns = NOCALL!", "---> change it !!!", 2000);
            KEYBOARD_Utils::rightArrow();
            currentBeacon = &Config.beacons[myBeaconsIndex];
        }
        miceActive = APRSPacketLib::validateMicE(currentBeacon->micE);
    }

    SMARTBEACON_Utils::checkSettings(currentBeacon->smartBeaconSetting);
    SMARTBEACON_Utils::checkState();

    BATTERY_Utils::monitor();
    Utils::checkDisplayEcoMode();

#ifdef BUTTON_PIN
    BUTTON_Utils::loop();
#endif
    KEYBOARD_Utils::read();
#ifdef HAS_JOYSTICK
    JOYSTICK_Utils::loop();
#endif
#ifdef HAS_TOUCHSCREEN
    TOUCH_Utils::loop();
#endif

    ReceivedLoRaPacket packet = LoRa_Utils::receivePacket();

    MSG_Utils::checkReceivedMessage(packet);
    MSG_Utils::processOutputBuffer();
    MSG_Utils::clean15SegBuffer();

    if (bluetoothActive && bluetoothConnected)
    {
        if (Config.bluetooth.useBLE)
        {
            BLE_Utils::sendToPhone(packet.text.substring(3));
            BLE_Utils::sendToLoRa();
        }
        else
        {
#ifdef HAS_BT_CLASSIC
            BLUETOOTH_Utils::sendToPhone(packet.text.substring(3));
            BLUETOOTH_Utils::sendToLoRa();
#endif
        }
    }

    MSG_Utils::ledNotification();
    Utils::checkFlashlight();
    STATION_Utils::checkListenedStationsByTimeAndDelete();

    lastTx = millis() - lastTxTime;
    if (gpsIsActive)
    {
        GPS_Utils::getData();
        bool gps_time_update = gps.time.isUpdated();
        bool gps_loc_update = gps.location.isUpdated();
        GPS_Utils::setDateFromData();

        int currentSpeed = (int)gps.speed.kmph();

        if (gps_loc_update)
            Utils::checkStatus();

        if (!sendUpdate && gps_loc_update && smartBeaconActive)
        {
            GPS_Utils::calculateDistanceTraveled();
            if (!sendUpdate)
                GPS_Utils::calculateHeadingDelta(currentSpeed);
            STATION_Utils::checkStandingUpdateTime();
        }
        SMARTBEACON_Utils::checkFixedBeaconTime();
        if (sendUpdate && gps_loc_update)
            STATION_Utils::sendBeacon();
        if (gps_time_update)
            SMARTBEACON_Utils::checkInterval(currentSpeed);

        if (millis() - refreshDisplayTime >= 1000 || gps_time_update)
        {
            GPS_Utils::checkStartUpFrames();
            MENU_Utils::showOnScreen();
            refreshDisplayTime = millis();
        }
        SLEEP_Utils::checkIfGPSShouldSleep();
    }
    else
    {
        if (millis() - lastGPSTime > txInterval)
        {
            SLEEP_Utils::gpsWakeUp();
        }
        STATION_Utils::checkStandingUpdateTime();
        if (millis() - refreshDisplayTime >= 1000)
        {
            MENU_Utils::showOnScreen();
            refreshDisplayTime = millis();
        }
    }
}*/