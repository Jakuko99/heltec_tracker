#ifndef GPX_h
#define GPX_h

#include <string>

#define _GPX_HEAD "<gpx version=\"1.1\" creator=\"Arduino GPX Lib\"\n xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n xmlns=\"http://www.topografix.com/GPX/1/1\"\n xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\n"
#define _GPX_TAIL "</gpx>\n"
#define _GPX_META_HEAD "<metadata>"
#define _GPX_META_TAIL "</metadata>\n"
#define _GPX_TRAK_HEAD "<trk>"
#define _GPX_TRAK_TAIL "</trk>\n"
#define _GPX_TRKSEG_HEAD "<trkseg>"
#define _GPX_TRKSEG_TAIL "</trkseg>\n"
#define _GPX_PT_HEAD "<TYPE lat=\""
#define _GPX_PT_TAIL "</TYPE>\n"

// Property Tags
#define _GPX_NAME_HEAD "<name>"
#define _GPX_NAME_TAIL "</name>\n"
#define _GPX_DESC_HEAD "<desc>"
#define _GPX_DESC_TAIL "</desc>\n"
#define _GPX_SYM_HEAD "<sym>"
#define _GPX_SYM_TAIL "</sym>\n"
#define _GPX_ELE_HEAD "<ele>"
#define _GPX_ELE_TAIL "</ele>\n"
#define _GPX_SRC_HEAD "<src>"
#define _GPX_SRC_TAIL "</src>\n"
#define _GPX_TIME_HEAD "<time>"
#define _GPX_TIME_TAIL "</time>\n"
#define _GPX_NUM_SAT_HEAD "<sat>"
#define _GPX_NUM_SAT_TAIL "</sat>\n"
#define _GPX_HORIZONTAL_ACCURACY_HEAD "<hdop>"
#define _GPX_HORIZONTAL_ACCURACY_TAIL "</hdop>\n"

// 'Public' Tags
#define GPX_TRKPT "trkpt"
#define GPX_WPT "wpt"
#define GPX_RTEPT "rtept"

using namespace std;

class GPX
{
public:
  GPX();
  string getOpen();
  string getClose();
  string getMetaData();
  string getTrakOpen();
  string getTrakClose();
  string getTrakSegOpen();
  string getTrakSegClose();
  string getInfo();
  string getName();
  string getPt(string typ, float lon, float lat);
  string getPt(string typ, float lon, float lat, float ele);
  string getPt(string typ, float lon, float lat, float ele, string time);
  string getPt(string typ, float lon, float lat, float ele, string time, int num_sat);
  string getPt(string typ, float lon, float lat, float ele, string time, int num_sat, float horizontal_accuracy);
  void setMetaName(string name);
  void setMetaDesc(string desc);
  void setName(string name);
  void setDesc(string desc);
  void setEle(float ele);
  void setSym(string sym);
  void setSrc(string src);
  void setNumSat(int num_sat);
  void setHorizontalAccuracy(float horizontal_accuracy);
  void setTime(string time);

private:
  // Variables
  string _metaName;
  string _metaDesc;
  string _name;
  string _desc;
  string _ele;
  string _sym;
  string _src;
  string _time;
  string _num_sat;
  string _horizontal_accuracy;

  // Functions
  string wrapCDATA(string input);
};

#endif
