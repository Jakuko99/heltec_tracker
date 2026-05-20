#include <gpx.h>

GPX::GPX()
{
}

// Get methods

String GPX::getOpen()
{
  return String(_GPX_HEAD);
}

String GPX::getClose()
{
  return String(_GPX_TAIL);
}

String GPX::getMetaData()
{
  String localStr(_GPX_META_HEAD);
  if (_metaName.length() > 0)
  {
    localStr += String(_GPX_NAME_HEAD);
    localStr += wrapCDATA(_metaName);
    localStr += String(_GPX_NAME_TAIL);
  }
  if (_metaDesc.length() > 0)
  {
    localStr += String(_GPX_DESC_HEAD);
    localStr += wrapCDATA(_metaDesc);
    localStr += String(_GPX_DESC_TAIL);
  }
  localStr += String(_GPX_META_TAIL);
  return localStr;
}

String GPX::getTrakOpen()
{
  return String(_GPX_TRAK_HEAD);
}

String GPX::getTrakClose()
{
  return String(_GPX_TRAK_TAIL);
}

String GPX::getTrakSegOpen()
{
  return String(_GPX_TRKSEG_HEAD);
}

String GPX::getTrakSegClose()
{
  return String(_GPX_TRKSEG_TAIL);
}

String GPX::getInfo()
{
  String localStr("");
  if (_name.length() > 0)
  {
    localStr += _GPX_NAME_HEAD;
    localStr += wrapCDATA(_name);
    localStr += _GPX_NAME_TAIL;
  }
  if (_desc.length() > 0)
  {
    localStr += _GPX_DESC_HEAD;
    localStr += wrapCDATA(_desc);
    localStr += _GPX_DESC_TAIL;
  }
  return localStr;
}

String GPX::getName()
{
  return _name;
}

String GPX::getPt(String typ, String lon, String lat)
{
  String localStr(_GPX_PT_HEAD);
  localStr.replace("TYPE", typ);
  localStr += lat + "\" lon=\"";
  localStr += lon + "\">";
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
    localStr += wrapCDATA(_src);
    localStr += _GPX_SRC_TAIL;
  }
  String ptTail(_GPX_PT_TAIL);
  ptTail.replace("TYPE", typ);
  localStr += ptTail;
  return localStr;
}

String GPX::getPt(String typ, String lon, String lat, String ele)
{
  setEle(ele);
  return getPt(typ, lon, lat);
}

String GPX::getPt(String typ, String lon, String lat, String ele, String time)
{
  setEle(ele);
  setTime(time);
  return getPt(typ, lon, lat);
}

String GPX::getPt(String typ, String lon, String lat, String ele, String time, String num_sat)
{
  setEle(ele);
  setTime(time);
  setNumSat(num_sat);
  return getPt(typ, lon, lat);
}

String GPX::getPt(String typ, String lon, String lat, String ele, String time, String num_sat, String horizontal_accuracy)
{
  setEle(ele);
  setTime(time);
  setNumSat(num_sat);
  setHorizontalAccuracy(horizontal_accuracy);
  return getPt(typ, lon, lat);
}

// Set Methods
void GPX::setMetaName(String name)
{
  _metaName = name;
}
void GPX::setMetaDesc(String desc)
{
  _metaDesc = desc;
}
void GPX::setName(String name)
{
  _name = name;
}
void GPX::setDesc(String desc)
{
  _desc = desc;
}
void GPX::setEle(String ele)
{
  _ele = ele;
}
void GPX::setSym(String sym)
{
  _sym = sym;
}
void GPX::setSrc(String src)
{
  _src = src;
}

void GPX::setNumSat(String num_sat)
{
  _num_sat = num_sat;
}

void GPX::setHorizontalAccuracy(String horizontal_accuracy)
{
  _horizontal_accuracy = horizontal_accuracy;
}

void GPX::setTime(String time)
{
  _time = time;
}

// Private Functions
String GPX::wrapCDATA(String input)
{
  String localStr("<![CDATA[");
  localStr += input;
  localStr += "]]>";

  return localStr;
}
