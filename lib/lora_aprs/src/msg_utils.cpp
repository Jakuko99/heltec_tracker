#include "msg_utils.h"

bool digipeaterActive = false;
uint32_t lastTxTime = 0;
uint32_t lastChallengeTime = 0;
bool wxRequestStatus = false;
uint32_t wxRequestTime = 0;

APRSPacket lastReceivedPacket;
bool SleepModeActive = false;
int winlinkStatus = 0;

String lastMessageSaved = "";
int numAPRSMessages = 0;
int numWLNKMessages = 0;
bool noAPRSMsgWarning = false;
bool noWLNKMsgWarning = false;
String lastHeardTracker = "NONE";

std::vector<String> loadedAPRSMessages;
std::vector<String> loadedWLNKMails;
std::vector<String> outputMessagesBuffer;
std::vector<String> outputAckRequestBuffer;
std::vector<Packet15SegBuffer> packet15SegBuffer;

int ackRequestNumber = random(1, 999);
bool ackRequestState = false;
String ackCallsignRequest = "";
String ackNumberRequest = "";
uint32_t lastMsgRxTime = millis();
uint32_t lastRetryTime = millis();

bool messageLed = false;
uint32_t messageLedTime = millis();

namespace MSG_Utils
{

    bool warnNoAPRSMessages()
    {
        return noAPRSMsgWarning;
    }

    bool warnNoWLNKMails()
    {
        return noWLNKMsgWarning;
    }

    const String getLastHeardTracker()
    {
        return lastHeardTracker;
    }

    int getNumAPRSMessages()
    {
        return numAPRSMessages;
    }

    int getNumWLNKMails()
    {
        return numWLNKMessages;
    }

    void loadNumMessages()
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }

        File fileToReadAPRS = SPIFFS.open("/aprsMessages.txt");
        if (!fileToReadAPRS)
        {
            Serial.println("Failed to open APRS_Msg for reading");
            return;
        }

        std::vector<String> v1;
        while (fileToReadAPRS.available())
        {
            v1.push_back(fileToReadAPRS.readStringUntil('\n'));
        }
        fileToReadAPRS.close();

        numAPRSMessages = 0;
        for (String s1 : v1)
        {
            numAPRSMessages++;
        }
        // logger.log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, "Main", "Number of APRS Messages : %s", String(numAPRSMessages));

        File fileToReadWLNK = SPIFFS.open("/winlinkMails.txt");
        if (!fileToReadWLNK)
        {
            Serial.println("Failed to open Winlink_Msg for reading");
            return;
        }

        std::vector<String> v2;
        while (fileToReadWLNK.available())
        {
            v2.push_back(fileToReadWLNK.readStringUntil('\n'));
        }
        fileToReadWLNK.close();

        numWLNKMessages = 0;
        for (String s2 : v2)
        {
            numWLNKMessages++;
        }
        // logger.log(logging::LoggerLevel::LOGGER_LEVEL_DEBUG, "Main", "Number of Winlink Mails : %s", String(numWLNKMessages));
    }

    void loadMessagesFromMemory(uint8_t typeOfMessage)
    {
        File fileToRead;
        if (typeOfMessage == 0)
        { // APRS
            noAPRSMsgWarning = false;
            if (numAPRSMessages == 0)
            {
                noAPRSMsgWarning = true;
            }
            else
            {
                loadedAPRSMessages.clear();
                fileToRead = SPIFFS.open("/aprsMessages.txt");
            }
            if (noAPRSMsgWarning)
            {
                // displayShow("   INFO", "", " NO APRS MSG SAVED", 1500);
            }
            else
            {
                if (!fileToRead)
                {
                    Serial.println("Failed to open file for reading");
                    return;
                }
                while (fileToRead.available())
                {
                    loadedAPRSMessages.push_back(fileToRead.readStringUntil('\n'));
                }
                fileToRead.close();
            }
        }
    }

    void deleteFile(uint8_t typeOfFile)
    {
        if (!SPIFFS.begin(true))
        {
            Serial.println("An Error has occurred while mounting SPIFFS");
            return;
        }
        if (typeOfFile == 0)
        { // APRS
            SPIFFS.remove("/aprsMessages.txt");
        }
        else if (typeOfFile == 1)
        { // WLNK
            SPIFFS.remove("/winlinkMails.txt");
        }
    }

    void saveNewMessage(uint8_t typeMessage, const String &station, const String &newMessage)
    {
        String message = newMessage;
        if (typeMessage == 0 && lastMessageSaved != message)
        { // APRS
            File fileToAppendAPRS = SPIFFS.open("/aprsMessages.txt", FILE_APPEND);
            if (!fileToAppendAPRS)
            {
                Serial.println("There was an error opening the file for appending");
                return;
            }
            message.trim();
            if (!fileToAppendAPRS.println(station + "," + message))
            {
                Serial.println("File append failed");
            }
            lastMessageSaved = message;
            numAPRSMessages++;
            fileToAppendAPRS.close();
        }
        else if (typeMessage == 1 && lastMessageSaved != message)
        { // WLNK
            File fileToAppendWLNK = SPIFFS.open("/winlinkMails.txt", FILE_APPEND);
            if (!fileToAppendWLNK)
            {
                Serial.println("There was an error opening the file for appending");
                return;
            }
            message.trim();
            if (!fileToAppendWLNK.println(message))
            {
                Serial.println("File append failed");
            }
            lastMessageSaved = message;
            numWLNKMessages++;
            fileToAppendWLNK.close();
        }
    }

    void sendMessage(const String &station, const String &textMessage)
    {
        String newPacket = APRSPacketLib::generateMessagePacket(Utils::getCallsign(), "APLRT1", Utils::getPath(), station, textMessage);
        if (textMessage.indexOf("ack") == 0 && station != "WLNK-1")
        { // don't show Winlink ACK
        }
        else if (station.indexOf("CA2RXU-15") == 0 && textMessage.indexOf("wrl") == 0)
        {
            wxRequestTime = millis();
            wxRequestStatus = true;
        }
        else
        {
            // displayShow((station == "WLNK-1") ? "WINLINK Tx" : " MSG Tx >", "", newPacket, 100);
        }
        LoRa_Utils::sendNewPacket(newPacket);
    }

    String getAckRequestNumber()
    {
        ackRequestNumber++;
        if (ackRequestNumber > 999)
        {
            ackRequestNumber = 1;
        }
        return String(ackRequestNumber);
    }

    void addToOutputBuffer(uint8_t typeOfMessage, const String &station, const String &textMessage)
    {
        bool alreadyInBuffer;
        if (typeOfMessage == 1)
        {
            alreadyInBuffer = false;
            if (!outputMessagesBuffer.empty())
            {
                for (int i = 0; i < outputMessagesBuffer.size(); i++)
                {
                    if (outputMessagesBuffer[i].indexOf(station + "," + textMessage) == 0)
                    {
                        alreadyInBuffer = true;
                        break;
                    }
                }
            }
            if (!outputAckRequestBuffer.empty())
            {
                for (int j = 0; j < outputAckRequestBuffer.size(); j++)
                {
                    if (outputAckRequestBuffer[j].indexOf(station + "," + textMessage) > 1)
                    {
                        alreadyInBuffer = true;
                        break;
                    }
                }
            }
            if (!alreadyInBuffer)
            {
                outputMessagesBuffer.push_back(station + "," + textMessage + "{" + getAckRequestNumber());
            }
        }
        else if (typeOfMessage == 0)
        {
            alreadyInBuffer = false;
            if (!outputMessagesBuffer.empty())
            {
                for (int k = 0; k < outputMessagesBuffer.size(); k++)
                {
                    if (outputMessagesBuffer[k].indexOf(station + "," + textMessage) == 0)
                    {
                        alreadyInBuffer = true;
                        break;
                    }
                }
            }
            if (!alreadyInBuffer)
            {
                outputMessagesBuffer.push_back(station + "," + textMessage);
            }
        }
    }

    void processOutputBuffer()
    {
        if (!outputMessagesBuffer.empty() && (millis() - lastMsgRxTime) >= 6000 && (millis() - lastTxTime) > 3000)
        {
            String addressee = outputMessagesBuffer[0].substring(0, outputMessagesBuffer[0].indexOf(","));
            String message = outputMessagesBuffer[0].substring(outputMessagesBuffer[0].indexOf(",") + 1);
            if (message.indexOf("{") > 0)
            {                                                                       // message with ack Request
                outputAckRequestBuffer.push_back("6," + addressee + "," + message); // 6 is for ack packets retries
                outputMessagesBuffer.erase(outputMessagesBuffer.begin());
            }
            else
            { // message without ack Request
                sendMessage(addressee, message);
                outputMessagesBuffer.erase(outputMessagesBuffer.begin());
                lastTxTime = millis();
            }
        }
        if (outputAckRequestBuffer.empty())
        {
            ackRequestState = false;
        }
        else if (!outputAckRequestBuffer.empty() && (millis() - lastMsgRxTime) >= 4500 && (millis() - lastTxTime) > 3000)
        {
            bool sendRetry = false;
            String triesLeft = outputAckRequestBuffer[0].substring(0, outputAckRequestBuffer[0].indexOf(","));
            switch (triesLeft.toInt())
            {
            case 6:
                sendRetry = true;
                ackRequestState = true;
                break;
            case 5:
                if (millis() - lastRetryTime > 30 * 1000)
                    sendRetry = true;
                break;
            case 4:
                if (millis() - lastRetryTime > 60 * 1000)
                    sendRetry = true;
                break;
            case 3:
                if (millis() - lastRetryTime > 120 * 1000)
                    sendRetry = true;
                break;
            case 2:
                if (millis() - lastRetryTime > 120 * 1000)
                    sendRetry = true;
                break;
            case 1:
                if (millis() - lastRetryTime > 120 * 1000)
                    sendRetry = true;
                break;
            case 0:
                if (millis() - lastRetryTime > 30 * 1000)
                {
                    ackRequestNumber = false;
                    outputAckRequestBuffer.erase(outputAckRequestBuffer.begin());
                    if (winlinkStatus > 0 && winlinkStatus < 5)
                    { // if not complete Winlink Challenge Process it will reset Login process
                        winlinkStatus = 0;
                    }
                }
                break;
            }
            if (sendRetry)
            {
                String rest = outputAckRequestBuffer[0].substring(outputAckRequestBuffer[0].indexOf(",") + 1);
                ackCallsignRequest = rest.substring(0, rest.indexOf(","));
                String payload = rest.substring(rest.indexOf(",") + 1);
                ackNumberRequest = payload.substring(payload.indexOf("{") + 1);
                sendMessage(ackCallsignRequest, payload);
                lastTxTime = millis();
                lastRetryTime = millis();
                outputAckRequestBuffer[0] = String(triesLeft.toInt() - 1) + "," + ackCallsignRequest + "," + payload;
            }
        }
    }

    void cleanOutputAckRequestBuffer(const String &station)
    {
        if (!outputAckRequestBuffer.empty())
        {
            for (int i = outputAckRequestBuffer.size() - 1; i >= 0; i--)
            {
                if (outputAckRequestBuffer[i].indexOf(station) == 0)
                    outputAckRequestBuffer.erase(outputAckRequestBuffer.begin() + i);
            }
        }
    }

    void clean15SegBuffer()
    {
        if (!packet15SegBuffer.empty() && (millis() - packet15SegBuffer[0].receivedTime) > 15 * 1000)
            packet15SegBuffer.erase(packet15SegBuffer.begin());
    }

    bool check15SegBuffer(const String &station, const String &textMessage)
    {
        if (!packet15SegBuffer.empty())
        {
            for (int i = 0; i < packet15SegBuffer.size(); i++)
            {
                if (packet15SegBuffer[i].station == station && packet15SegBuffer[i].payload == textMessage)
                    return false;
            }
        }
        Packet15SegBuffer packet;
        packet.receivedTime = millis();
        packet.station = station;
        packet.payload = textMessage;
        packet15SegBuffer.push_back(packet);
        return true;
    }

    void checkReceivedMessage(ReceivedLoRaPacket packet)
    {
        if (packet.text.isEmpty())
        {
            return;
        }
        if (packet.text.substring(0, 3) == "\x3c\xff\x01")
        { // its an APRS packet
            // Serial.println(packet.text); // only for debug
            lastReceivedPacket = APRSPacketLib::processReceivedPacket(packet.text.substring(3), packet.rssi, packet.snr, packet.freqError);
            if (lastReceivedPacket.sender != Utils::getCallsign())
            {

                if (lastReceivedPacket.payload.indexOf("\x3c\xff\x01") != -1)
                {
                    lastReceivedPacket.payload = lastReceivedPacket.payload.substring(0, lastReceivedPacket.payload.indexOf("\x3c\xff\x01"));
                }

                if (check15SegBuffer(lastReceivedPacket.sender, lastReceivedPacket.payload))
                {

                    if (digipeaterActive && lastReceivedPacket.addressee != Utils::getCallsign())
                    {
                        String digipeatedPacket = APRSPacketLib::generateDigipeatedPacket(packet.text, Utils::getCallsign(), Utils::getPath());
                        if (digipeatedPacket == "X")
                        {
                            // logger.log(logging::LoggerLevel::LOGGER_LEVEL_WARN, "Main", "%s", "Packet won't be Repeated (Missing WIDEn-N)");
                        }
                        else
                        {
                            delay(500);
                            LoRa_Utils::sendNewPacket(digipeatedPacket);
                        }
                    }
                    lastHeardTracker = lastReceivedPacket.sender;

                    if (lastReceivedPacket.type == 1 && lastReceivedPacket.addressee == Utils::getCallsign())
                    {

                        if (ackRequestState && lastReceivedPacket.payload.indexOf("ack") == 0)
                        {
                            if (ackCallsignRequest == lastReceivedPacket.sender && ackNumberRequest == lastReceivedPacket.payload.substring(lastReceivedPacket.payload.indexOf("ack") + 3))
                            {
                                // outputAckRequestBuffer.erase(outputAckRequestBuffer.begin());
                                ackRequestState = false;
                            }
                        }
                        if (lastReceivedPacket.payload.indexOf("{") >= 0)
                        {
                            MSG_Utils::addToOutputBuffer(0, lastReceivedPacket.sender, "ack" + lastReceivedPacket.payload.substring(lastReceivedPacket.payload.indexOf("{") + 1));
                            lastMsgRxTime = millis();
                            lastReceivedPacket.payload = lastReceivedPacket.payload.substring(0, lastReceivedPacket.payload.indexOf("{"));
                        }

                        if (lastReceivedPacket.sender == "CA2RXU-15" && lastReceivedPacket.payload.indexOf("WX") == 0)
                        { // WX = WeatherReport
                            Serial.println("Weather Report Received");
                            const String &wxCleaning = lastReceivedPacket.payload.substring(lastReceivedPacket.payload.indexOf("WX ") + 3);
                            const String &place = wxCleaning.substring(0, wxCleaning.indexOf(","));
                            const String &placeCleaning = wxCleaning.substring(wxCleaning.indexOf(",") + 1);
                            const String &summary = placeCleaning.substring(0, placeCleaning.indexOf(","));
                            const String &sumCleaning = placeCleaning.substring(placeCleaning.indexOf(",") + 2);
                            const String &temperature = sumCleaning.substring(0, sumCleaning.indexOf("P"));
                            const String &tempCleaning = sumCleaning.substring(sumCleaning.indexOf("P") + 1);
                            const String &pressure = tempCleaning.substring(0, tempCleaning.indexOf("H"));
                            const String &presCleaning = tempCleaning.substring(tempCleaning.indexOf("H") + 1);
                            const String &humidity = presCleaning.substring(0, presCleaning.indexOf("W"));
                            const String &humCleaning = presCleaning.substring(presCleaning.indexOf("W") + 1);
                            const String &windSpeed = humCleaning.substring(0, humCleaning.indexOf(","));
                            const String &windCleaning = humCleaning.substring(humCleaning.indexOf(",") + 1);
                            const String &windDegrees = windCleaning.substring(windCleaning.indexOf(",") + 1, windCleaning.indexOf("\n"));

                            String fifthLineWR = temperature;
                            fifthLineWR += "C  ";
                            fifthLineWR += pressure;
                            fifthLineWR += "hPa  ";
                            fifthLineWR += humidity;
                            fifthLineWR += "%";

                            String sixthLineWR = "(wind ";
                            sixthLineWR += windSpeed;
                            sixthLineWR += "m/s ";
                            sixthLineWR += windDegrees;
                            sixthLineWR += "deg)";
                        }
                    }
                }
            }
        }
    }

}