#include "gps_tracker.h"

GPSTracker::GPSTracker(TinyGPSPlus *gps)
{
    this->GPS = gps;
    gpx_parser.setMetaDesc("FW v0.1");
    gpx_parser.setDesc(track_desc);
}

GPSTracker::GPSTracker(TinyGPSPlus *gps, float track_distance, int track_interval, String track_desc)
{
    this->GPS = gps;
    gpx_parser.setMetaDesc("FW v0.1");
    gpx_parser.setDesc(track_desc);
    this->tracking_distance = track_distance;
    this->tracking_interval = track_interval;
    this->track_desc = track_desc;
}

void GPSTracker::load_config(float track_distance, int track_interval, String track_desc)
{
    this->tracking_distance = track_distance;
    this->tracking_interval = track_interval;
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
            GpxFile.print(gpx_parser.getOpen());
            GpxFile.print(gpx_parser.getMetaData());
            GpxFile.print(gpx_parser.getTrakOpen());
            GpxFile.print(gpx_parser.getInfo());
            GpxFile.print(gpx_parser.getTrakSegOpen());
            GpxFile.close();
            tracking_active = true;
            return true;
        }
    }

    return false;
}

bool GPSTracker::track_point()
{
    if (sd_card_init && tracking_active)
    {
        float lat = GPS->location.lat();
        float lon = GPS->location.lng();
        float ele = GPS->altitude.meters();
        return track_point(lat, lon, ele);
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
            GpxFile.print(gpx_parser.getPt(GPX_TRKPT, lat, lon, ele, format_time(last_point->time), GPS->satellites.value()));
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
            GpxFile.print(gpx_parser.getTrakSegClose());
            GpxFile.print(gpx_parser.getTrakClose());
            GpxFile.print(gpx_parser.getClose());
            GpxFile.close();

            delete last_point; // clean up last point memory
            last_point = nullptr;
            tracking_active = false;
            return true;
        }
    }
    return false;
}

bool GPSTracker::init_waypoint_file()
{
    if (sd_card_init)
    {
        if (!SD.exists("waypoints.gpx"))
        {
            File waypoint_file = SD.open("waypoints.gpx", "w");
            gpx_parser.setMetaName("Waypoints");
            if (waypoint_file)
            {
                // write header to the file
                waypoint_file.print(gpx_parser.getOpen());
                waypoint_file.print(gpx_parser.getMetaData());
                waypoint_file.print(gpx_parser.getClose());
                waypoint_file.close();
                gpx_parser.setMetaName(""); // reset meta name for track files
                return true;
            }
            return true; // file already exists, so consider it initialized
        }
    }
    return false;
}

bool GPSTracker::save_waypoint()
{
    if (sd_card_init)
    {
        float lat = GPS->location.lat();
        float lon = GPS->location.lng();
        float ele = GPS->altitude.meters();
        return save_waypoint(lat, lon, ele);
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

bool GPSTracker::save_waypoint_gpx()
{
    if (sd_card_init)
    {
        float lat = GPS->location.lat();
        float lon = GPS->location.lng();
        float ele = GPS->altitude.meters();
        return save_waypoint_gpx(lat, lon, ele);
    }
    return false;
}

bool GPSTracker::save_waypoint_gpx(float lat, float lon, float ele)
{
    if (sd_card_init)
    {
        if (!SD.exists("waypoints.gpx"))
        {
            init_waypoint_file(); // create file with header if it doesn't exist
        }
        File waypoint_file = SD.open("waypoints.gpx", "r");
        if (waypoint_file)
        {
            String xml_content = waypoint_file.readStringUntil(EOF); // read entire file content
            std::unique_ptr<char[]> xml_buffer(new char[xml_content.length() + 1]);
            xml_content.toCharArray(xml_buffer.get(), xml_content.length() + 1);
            doc.parse<0>(xml_buffer.get()); // parse existing GPX file
            rapidxml::xml_node<> *root = doc.first_node("gpx");
            waypoint_file.close();

            // build waypoint attributes string and allocate it with RapidXML
            rapidxml::xml_node<> *node = doc.allocate_node(rapidxml::node_element, "wpt");
            root->append_node(node);
            rapidxml::xml_attribute<> *lat_attr = doc.allocate_attribute("lat", String(lat).c_str());
            node->append_attribute(lat_attr);
            rapidxml::xml_attribute<> *lon_attr = doc.allocate_attribute("lon", String(lon).c_str());
            node->append_attribute(lon_attr);
            rapidxml::xml_node<> *ele_node = doc.allocate_node(rapidxml::node_element, "ele", String(ele).c_str());
            node->append_node(ele_node);
            rapidxml::xml_node<> *time_node = doc.allocate_node(rapidxml::node_element, "time", format_time(get_current_time()).c_str());
            node->append_node(time_node);
            rapidxml::xml_node<> *name_node = doc.allocate_node(rapidxml::node_element, "name", format_time(get_current_time()).c_str());
            node->append_node(name_node);

            waypoint_file = SD.open("waypoints.gpx", "w");
            if (!waypoint_file)
            {
                return false; // failed to open file for writing
            }
            // write updated XML back to the file
            std::string xml_out;
            rapidxml::print(std::back_inserter(xml_out), doc, 0);
            waypoint_file.print(xml_out.c_str());
            waypoint_file.close();

            // free up allocated XML memory
            delete[] xml_buffer.release(); // clean up XML buffer memory
            doc.clear();

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