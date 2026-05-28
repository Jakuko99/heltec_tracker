#ifndef GPS_TRACKER_H
#define GPS_TRACKER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <string>

#include "rapidxml.hpp"
#include "rapidxml_print.hpp"
#include "gpx.h"
#include "HT_TinyGPS++.h"

using namespace std;

struct DateTime
{
    short year;
    int8_t month;
    int8_t day;
    int8_t hour;
    int8_t minute;
    int8_t second;
};

struct RoutePoint
{
    float lat;
    float lon;
    float ele;
    DateTime time;
};

class GPSTracker
{
public:
    GPSTracker(TinyGPSPlus *gps);
    GPSTracker(TinyGPSPlus *gps, float track_distance, int track_interval, string track_desc);
    void load_config(float track_distance, int track_interval, string track_desc);

    bool begin_tracking();
    bool track_point();
    bool track_point(float lat, float lon, float ele);
    bool end_tracking();

    bool init_waypoint_file();
    bool save_waypoint();
    bool save_waypoint(float lat, float lon, float ele);
    bool save_waypoint_gpx();
    bool save_waypoint_gpx(float lat, float lon, float ele);

    int time_between(DateTime start, DateTime end);
    DateTime parse_time(string time_str);
    string format_time(DateTime dt);
    DateTime get_current_time();
    void set_sd_card_init(bool _init) { sd_card_init = _init; }
    bool is_tracking_active() const { return tracking_active; }

private:
    RoutePoint *last_point = nullptr;
    bool tracking_active = false;
    string track_filename;
    bool sd_card_init = false;
    float tracking_distance; // minimum distance in meters to log a new point
    int tracking_interval;   // minimum time in seconds to log a new point
    string track_desc;       // description for the track
    GPX gpx_parser;
    rapidxml::xml_document<> doc;
    File GpxFile;
    TinyGPSPlus *GPS;
};

#endif // GPS_TRACKER_H