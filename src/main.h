#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

#include "HT_st7735.h"
#include "HT_TinyGPS++.h"
#include "gpx.h"

// Board definitions
#define REGION_EU868
#define WIRELESS_TRACKER_V2

// Pin definitions
#define PAD_UP_PIN 43
#define PAD_DOWN_PIN 44
#define PAD_LEFT_PIN 45
#define PAD_RIGHT_PIN 46
#define PAD_MIDDLE_PIN 4
#define SD_CS 5
#define GPS_ENABLE_PIN 3
#define BATT_ADC A0 // ADC1_CH0

#define CYCLE_TIME 100 // ms

enum PadAction
{
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    MIDDLE
};

struct BoardConfig
{
    int tracking_interval = 5000;   // ms
    float tracking_distance = 20.0; // meters
};

// Tracking and UI methods
bool begin_tracking();
bool track_point(float, float, float, String);
bool end_tracking();
void show_menu();

// Control methods
PadAction get_action();

void setup();
void loop();

#endif // MAIN_H