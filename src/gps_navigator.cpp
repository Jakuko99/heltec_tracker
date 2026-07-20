#include "gps_navigator.h"

GPSNavigator::GPSNavigator(TinyGPSPlus *gps) : gps(gps), instruction_interval(0), waypoint_interval(0), last_instruction_time(0), last_waypoint_time(0) {}

void GPSNavigator::init(int instruction_interval, int waypoint_interval)
{
    this->instruction_interval = instruction_interval;
    this->waypoint_interval = waypoint_interval;
}

bool GPSNavigator::load_route(string route_file)
{
    // Implementation for loading route
    return true;
}

string GPSNavigator::fetch_next_instruction()
{
    // Implementation for fetching next instruction
    return "";
}

float GPSNavigator::distance_to_next_waypoint()
{
    // Implementation for calculating distance to next waypoint
    return 0.0f;
}

bool GPSNavigator::close_route()
{
    // Implementation for closing route
    return true;
}