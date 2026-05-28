#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../lib/gpx/src/gpx.h"
#include "../lib/rapidxml/include/rapidxml.hpp"
#include "../lib/rapidxml/include/rapidxml_print.hpp"

using namespace std;

int main()
{
    GPX gpx_parser;
    rapidxml::xml_document<> doc;
    gpx_parser.setMetaDesc("FW v0.1");
    gpx_parser.setDesc("desc");
    gpx_parser.setName("track-2026-04-10T10:00:00Z");
    gpx_parser.setMetaName("Waypoints");
    /*std::ofstream file("test1.gpx");
    file << gpx_parser.getOpen();
    file << gpx_parser.getMetaData();
    file << gpx_parser.getClose();
    file.close();*/

    // read existing GPX file content into string
    std::ifstream file_in("test1.gpx");
    std::string xml_content;
    if (file_in)
    {
        std::ostringstream ss;
        ss << file_in.rdbuf();
        xml_content = ss.str();
    }
    file_in.close();

    if (!xml_content.empty())
    {
        xml_content.push_back('\0');
        doc.parse<0>(&xml_content[0]); // parse existing GPX file (RapidXML needs mutable buffer)
    }

    rapidxml::xml_node<> *root = doc.first_node("gpx");        
    // build waypoint attributes string and allocate it with RapidXML        
    rapidxml::xml_node<> *node = doc.allocate_node(rapidxml::node_element, "wpt");    
    root->append_node(node);
    rapidxml::xml_attribute<> *lat_attr = doc.allocate_attribute("lat", "10.51");
    node->append_attribute(lat_attr);
    rapidxml::xml_attribute<> *lon_attr = doc.allocate_attribute("lon", "10.52");
    node->append_attribute(lon_attr);
    rapidxml::xml_node<> *ele_node = doc.allocate_node(rapidxml::node_element, "ele", "100.0");
    node->append_node(ele_node);
    rapidxml::xml_node<> *time_node = doc.allocate_node(rapidxml::node_element, "time", "2026-04-10T10:02:15Z");
    node->append_node(time_node);    

    std::ofstream waypoint_file("test1.gpx");
    if (!waypoint_file)
    {
        return false; // failed to open file for writing
    }
    // write updated XML back to the file
    std::string xml_out;
    rapidxml::print(std::back_inserter(xml_out), doc, 0);
    waypoint_file << xml_out;
    waypoint_file.close();
    doc.clear();

    /*file << gpx_parser.getOpen();
    file << gpx_parser.getMetaData();
    file << gpx_parser.getTrakOpen();
    file << gpx_parser.getInfo();
    file << gpx_parser.getTrakSegOpen();
    file << gpx_parser.getPt(GPX_TRKPT, 10.12, 10.13, 100.0, "2026-04-10T10:02:15Z", 12);
    file << gpx_parser.getPt(GPX_TRKPT, 10.15, 10.13, 150.0, "2026-04-10T10:02:30Z", 12);
    file << gpx_parser.getPt(GPX_TRKPT, 10.12, 10.25, 200.0, "2026-04-10T10:02:45Z", 12);
    file << gpx_parser.getPt(GPX_TRKPT, 10.16, 10.27, 100.0, "2026-04-10T10:02:50Z", 12);
    file << gpx_parser.getTrakSegClose();
    file << gpx_parser.getTrakClose();
    file << gpx_parser.getClose();
    file.close();*/

    cout << "Done." << endl;
    return 0;
}