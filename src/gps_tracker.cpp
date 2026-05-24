#include "gps_tracker.h"

GPSTracker::GPSTracker(TinyGPSPlus *gps)
{
    GPS = gps;
    gpx_parser.setMetaDesc("FW v0.1");
    gpx_parser.setDesc(track_desc);
}

void GPSTracker::load_config(float track_distance, int track_interval, String track_desc)
{
    tracking_distance = track_distance;
    tracking_interval = track_interval;
    this->track_desc = track_desc;
}

bool GPSTracker::begin_tracking()
{
    // set metadata for GPX file
    gpx_parser.setName("track-" + format_time(get_current_time()));
    // gpx_parser.setSrc("GPS Tracker");

    if (sd_card_init)
    {
        // create track file
        GpxFile = SD.open(gpx_parser.getName() + ".gpx", "w");
        if (GpxFile)
        {
            // write header to the file
            GpxFile.println(gpx_parser.getOpen());
            GpxFile.println(gpx_parser.getMetaData());
            GpxFile.println(gpx_parser.getTrakOpen());
            GpxFile.println(gpx_parser.getInfo());
            GpxFile.println(gpx_parser.getTrakSegOpen());
            GpxFile.close();
            tracking_active = true;
            return true;
        }
    }

    return false;
}

bool GPSTracker::track_point()
{
    if (sd_card_init)
    {
        GpxFile = SD.open(gpx_parser.getName() + ".gpx", "a");
        if (GpxFile && tracking_active)
        {
            float lat = GPS->location.lat();
            float lon = GPS->location.lng();
            float ele = GPS->altitude.meters();

            if (last_point != nullptr)
            {
                float distance = GPS->distanceBetween(last_point->lat, last_point->lon, lat, lon);
                if (distance < tracking_distance || (time_between(last_point->time, get_current_time()) < tracking_interval))
                {
                    // skip point if it's too close to the last one
                    GpxFile.close();
                    return true; // successfully "tracked" (skipped) the point, so return true
                }
            }

            last_point = new RoutePoint{lat, lon, ele, get_current_time()}; // store last point for distance/time checks
            // write a track point to the file
            GpxFile.println(gpx_parser.getPt(GPX_TRKPT, lat, lon, ele, format_time(last_point->time), GPS->satellites.value()));
            GpxFile.close();
            return true;
        }
    }

    return false;
}

bool GPSTracker::track_point(float lat, float lon, float ele)
{
    if (sd_card_init)
    {
        GpxFile = SD.open(gpx_parser.getName() + ".gpx", "a");
        if (GpxFile && tracking_active)
        {
            if (last_point != nullptr)
            {
                float distance = GPS->distanceBetween(last_point->lat, last_point->lon, lat, lon);
                if (distance < tracking_distance || (time_between(last_point->time, get_current_time()) < tracking_interval))
                {
                    // skip point if it's too close to the last one
                    GpxFile.close();
                    return true; // successfully "tracked" (skipped) the point, so return true
                }
            }

            last_point = new RoutePoint{lat, lon, ele, get_current_time()}; // store last point for distance/time checks
            // write a track point to the file
            GpxFile.println(gpx_parser.getPt(GPX_TRKPT, lat, lon, ele, format_time(last_point->time), GPS->satellites.value()));
            GpxFile.close();
            return true;
        }
    }

    return false;
}

bool GPSTracker::end_tracking()
{
    if (sd_card_init)
    {
        GpxFile = SD.open(gpx_parser.getName() + ".gpx", "a");
        if (GpxFile)
        {
            // write footer to the file and close it
            GpxFile.println(gpx_parser.getTrakSegClose());
            GpxFile.println(gpx_parser.getTrakClose());
            GpxFile.println(gpx_parser.getClose());
            GpxFile.close();

            delete last_point; // clean up last point memory
            last_point = nullptr;
            tracking_active = false;
            return true;
        }
    }
    return false;
}

bool GPSTracker::save_waypoint()
{
    if (sd_card_init)
    {
        File waypoint_file = SD.open("waypoints.csv", "a");
        if (waypoint_file)
        {
            float lat = GPS->location.lat();
            float lon = GPS->location.lng();
            float ele = GPS->altitude.meters();

            // write a waypoint to the file
            waypoint_file.print(format_time(get_current_time()));
            waypoint_file.print(",");
            waypoint_file.print(lat, 6);
            waypoint_file.print(",");
            waypoint_file.print(lon, 6);
            waypoint_file.print(",");
            waypoint_file.println(ele, 1);
            waypoint_file.close();
            return true;
        }
    }
    return false;
}

bool GPSTracker::save_waypoint(float lat, float lon, float ele)
{
    if (sd_card_init)
    {
        File waypoint_file = SD.open("waypoints.csv", "a");
        if (waypoint_file)
        {
            // write a waypoint to the file
            waypoint_file.print(format_time(get_current_time()));
            waypoint_file.print(",");
            waypoint_file.print(lat, 6);
            waypoint_file.print(",");
            waypoint_file.print(lon, 6);
            waypoint_file.print(",");
            waypoint_file.println(ele, 1);
            waypoint_file.close();
            return true;
        }
    }
    return false;
}

int GPSTracker::time_between(DateTime start, DateTime end)
{
    // This function calculates the time difference in seconds between two DateTime structs
    tm *tm_start = new tm{start.second, start.minute, start.hour, start.day, start.month - 1, start.year - 1900};
    tm *tm_end = new tm{end.second, end.minute, end.hour, end.day, end.month - 1, end.year - 1900};
    time_t time_start = mktime(tm_start);
    time_t time_end = mktime(tm_end);
    return difftime(time_end, time_start);
}

DateTime GPSTracker::parse_time(String time_str)
{
    // This function parses an ISO 8601 time string and returns a DateTime struct
    struct tm tm_time;
    strptime(time_str.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tm_time);
    DateTime dt;
    dt.year = tm_time.tm_year + 1900;
    dt.month = tm_time.tm_mon + 1;
    dt.day = tm_time.tm_mday;
    dt.hour = tm_time.tm_hour;
    dt.minute = tm_time.tm_min;
    dt.second = tm_time.tm_sec;
    return dt;
}

String GPSTracker::format_time(DateTime dt)
{
    // This function formats a DateTime struct into an ISO 8601 time string
    char buffer[25];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
    return String(buffer);
}

DateTime GPSTracker::get_current_time()
{
    // This function gets the current time from the GPS and returns it as a DateTime struct
    if (!GPS->date.isValid() || !GPS->time.isValid())
    {
        return DateTime{0, 0, 0, 0, 0, 0}; // return a default time if GPS time is not valid
    }

    DateTime dt;
    dt.year = GPS->date.year();
    dt.month = GPS->date.month();
    dt.day = GPS->date.day();
    dt.hour = GPS->time.hour();
    dt.minute = GPS->time.minute();
    dt.second = GPS->time.second();
    return dt;
}