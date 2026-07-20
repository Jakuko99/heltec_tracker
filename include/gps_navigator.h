#ifndef GPS_NAVIGATOR_H
#define GPS_NAVIGATOR_H

#include <TinyGPSPlus.h>
#include <string>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

using namespace std;

class GPSNavigator
{
public:
    GPSNavigator(TinyGPSPlus *gps);
    void init(int instruction_interval, int waypoint_interval);
    bool load_route(string route_file);
    string fetch_next_instruction();
    float distance_to_next_waypoint();
    bool close_route();

private:
    TinyGPSPlus *gps;
    int instruction_interval; // in seconds
    int waypoint_interval;    // in seconds
    unsigned long last_instruction_time;
    unsigned long last_waypoint_time;    
};

#endif // GPS_NAVIGATOR_H