#include "gpx.h"

GPX::GPX()
{
}

// Get methods

string GPX::getOpen()
{
  return string(_GPX_HEAD);
}

string GPX::getClose()
{
  return string(_GPX_TAIL);
}

string GPX::getMetaData()
{
  string localStr(_GPX_META_HEAD);
  if (_metaName.length() > 0)
  {
    localStr += string(_GPX_NAME_HEAD);
    localStr += _metaName;
    localStr += string(_GPX_NAME_TAIL);
  }
  if (_metaDesc.length() > 0)
  {
    localStr += string(_GPX_DESC_HEAD);
    localStr += _metaDesc;
    localStr += string(_GPX_DESC_TAIL);
  }
  localStr += string(_GPX_META_TAIL);
  return localStr;
}

string GPX::getTrakOpen()
{
  return string(_GPX_TRAK_HEAD);
}

string GPX::getTrakClose()
{
  return string(_GPX_TRAK_TAIL);
}

string GPX::getTrakSegOpen()
{
  return string(_GPX_TRKSEG_HEAD);
}

string GPX::getTrakSegClose()
{
  return string(_GPX_TRKSEG_TAIL);
}

string GPX::getInfo()
{
  string localStr("");
  if (_name.length() > 0)
  {
    localStr += _GPX_NAME_HEAD;
    localStr += _name;
    localStr += _GPX_NAME_TAIL;
  }
  if (_desc.length() > 0)
  {
    localStr += _GPX_DESC_HEAD;
    localStr += _desc;
    localStr += _GPX_DESC_TAIL;
  }
  return localStr;
}

string GPX::getName()
{
  return _name;
}

string GPX::getPt(string typ, float lon, float lat)
{
  string localStr(_GPX_PT_HEAD);
  localStr.replace(localStr.find("TYPE"), 4, typ);
  localStr += to_string(lat) + "\" lon=\"";
  localStr += to_string(lon) + "\">";
  if (_ele.length() > 0)
  {
    localStr += _GPX_ELE_HEAD;
    localStr += _ele;
    localStr += _GPX_ELE_TAIL;
  }
  if (_time.length() > 0)
  {
    localStr += _GPX_TIME_HEAD;
    localStr += _time;
    localStr += _GPX_TIME_TAIL;
  }
  if (_num_sat.length() > 0)
  {
    localStr += _GPX_NUM_SAT_HEAD;
    localStr += _num_sat;
    localStr += _GPX_NUM_SAT_TAIL;
  }
  if (_horizontal_accuracy.length() > 0)
  {
    localStr += _GPX_HORIZONTAL_ACCURACY_HEAD;
    localStr += _horizontal_accuracy;
    localStr += _GPX_HORIZONTAL_ACCURACY_TAIL;
  }
  if (_sym.length() > 0)
  {
    localStr += _GPX_SYM_HEAD;
    localStr += _sym;
    localStr += _GPX_SYM_TAIL;
  }
  if (_src.length() > 0)
  {
    localStr += _GPX_SRC_HEAD;
    localStr += _src;
    localStr += _GPX_SRC_TAIL;
  }
  string ptTail(_GPX_PT_TAIL);
  ptTail.replace(ptTail.find("TYPE"), 4, typ);
  localStr += ptTail;
  return localStr;
}

string GPX::getPt(string typ, float lon, float lat, float ele)
{
  setEle(ele);
  return getPt(typ, lon, lat);
}

string GPX::getPt(string typ, float lon, float lat, float ele, string time)
{
  setEle(ele);
  setTime(time);
  return getPt(typ, lon, lat);
}

string GPX::getPt(string typ, float lon, float lat, float ele, string time, int num_sat)
{
  setEle(ele);
  setTime(time);
  setNumSat(num_sat);
  return getPt(typ, lon, lat);
}

string GPX::getPt(string typ, float lon, float lat, float ele, string time, int num_sat, float horizontal_accuracy)
{
  setEle(ele);
  setTime(time);
  setNumSat(num_sat);
  setHorizontalAccuracy(horizontal_accuracy);
  return getPt(typ, lon, lat);
}

// Set Methods
void GPX::setMetaName(string name)
{
  _metaName = name;
}
void GPX::setMetaDesc(string desc)
{
  _metaDesc = desc;
}
void GPX::setName(string name)
{
  _name = name;
}
void GPX::setDesc(string desc)
{
  _desc = desc;
}
void GPX::setEle(float ele)
{
  _ele = to_string(ele);
}
void GPX::setSym(string sym)
{
  _sym = sym;
}
void GPX::setSrc(string src)
{
  _src = src;
}

void GPX::setNumSat(int num_sat)
{
  _num_sat = to_string(num_sat);
}

void GPX::setHorizontalAccuracy(float horizontal_accuracy)
{
  _horizontal_accuracy = to_string(horizontal_accuracy);
}

void GPX::setTime(string time)
{
  _time = time;
}

// Private Functions
string GPX::wrapCDATA(string input)
{
  string localStr("<![CDATA[");
  localStr += input;
  localStr += "]]>";

  return localStr;
}
