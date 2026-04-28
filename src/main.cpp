#include "main.h"

TinyGPSPlus GPS;
GPX myGPX;
File GpxFile;
HT_st7735 disp;
BoardConfig boardConfig;

bool sd_card_init = false;
int menu_item = 0;
unsigned long prev_millis = 0;
RoutePoint *last_point;

bool begin_tracking()
{
  // set metadata for GPX file
  myGPX.setMetaDesc("FW v0.1");
  myGPX.setName("track-" + String(GPS.date.year()) + String(GPS.date.month()) + String(GPS.date.day()) + "-" + String(GPS.time.hour()) + "_" + String(GPS.time.minute()));
  myGPX.setDesc(boardConfig.track_desc);
  // myGPX.setSrc("GPS Tracker");

  if (sd_card_init)
  {
    // create track file
    GpxFile = SD.open(myGPX.getName() + ".gpx", "w");
    if (GpxFile)
    {
      // write header to the file
      GpxFile.println(myGPX.getOpen());
      GpxFile.println(myGPX.getMetaData());
      GpxFile.println(myGPX.getTrakOpen());
      GpxFile.println(myGPX.getInfo());
      GpxFile.println(myGPX.getTrakSegOpen());
      GpxFile.close();
      return true;
    }
  }

  return false;
}

bool track_point(float lat, float lon, float ele)
{
  if (sd_card_init)
  {
    GpxFile = SD.open(myGPX.getName() + ".gpx", "a");
    if (GpxFile)
    {
      if (last_point != nullptr)
      {
        float distance = GPS.distanceBetween(last_point->lat, last_point->lon, lat, lon);
        if (distance < boardConfig.tracking_distance || (time_between(last_point->time, get_current_time()) < boardConfig.tracking_interval))
        {
          // skip point if it's too close to the last one
          GpxFile.close();
          return true; // successfully "tracked" (skipped) the point, so return true
        }
      }

      last_point = new RoutePoint{lat, lon, ele, get_current_time()}; // store last point for distance/time checks
      // write a track point to the file
      GpxFile.println(myGPX.getPt(GPX_TRKPT, String(lat), String(lon), String(ele), format_time(last_point->time)));
      GpxFile.close();
      return true;
    }
  }

  return false;
}

bool end_tracking()
{
  if (sd_card_init)
  {
    GpxFile = SD.open(myGPX.getName() + ".gpx", "a");
    if (GpxFile)
    {
      // write footer to the file and close it
      GpxFile.println(myGPX.getTrakSegClose());
      GpxFile.println(myGPX.getTrakClose());
      GpxFile.println(myGPX.getClose());
      GpxFile.close();

      delete last_point; // clean up last point memory
      last_point = nullptr;
      return true;
    }
  }
  return false;
}

int time_between(DateTime start, DateTime end)
{
  // This function calculates the time difference in seconds between two DateTime structs
  tm *tm_start = new tm{start.second, start.minute, start.hour, start.day, start.month - 1, start.year - 1900};
  tm *tm_end = new tm{end.second, end.minute, end.hour, end.day, end.month - 1, end.year - 1900};
  time_t time_start = mktime(tm_start);
  time_t time_end = mktime(tm_end);
  return difftime(time_end, time_start);
}

DateTime parse_time(String time_str)
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

String format_time(DateTime dt)
{
  // This function formats a DateTime struct into an ISO 8601 time string
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
  return String(buffer);
}

DateTime get_current_time()
{
  // This function gets the current time from the GPS and returns it as a DateTime struct
  DateTime dt;
  dt.year = GPS.date.year();
  dt.month = GPS.date.month();
  dt.day = GPS.date.day();
  dt.hour = GPS.time.hour();
  dt.minute = GPS.time.minute();
  dt.second = GPS.time.second();
  return dt;
}

void show_menu()
{
  switch (menu_item)
  {
  case 0:
    // draw main screen
    break;

  case 1:
    // draw tracking screen
    break;

  default:
    break;
  }
}

PadAction get_action()
{
  if (digitalRead(PAD_UP_PIN) == LOW)
  {
    return UP;
  }
  else if (digitalRead(PAD_DOWN_PIN) == LOW)
  {
    return DOWN;
  }
  else if (digitalRead(PAD_LEFT_PIN) == LOW)
  {
    return LEFT;
  }
  else if (digitalRead(PAD_RIGHT_PIN) == LOW)
  {
    return RIGHT;
  }
  else if (digitalRead(PAD_MIDDLE_PIN) == LOW)
  {
    return MIDDLE;
  }
  return NONE;
}

void setup()
{
  // Setup buttons
  pinMode(PAD_UP_PIN, INPUT_PULLUP);
  pinMode(PAD_DOWN_PIN, INPUT_PULLUP);
  pinMode(PAD_LEFT_PIN, INPUT_PULLUP);
  pinMode(PAD_RIGHT_PIN, INPUT_PULLUP);
  pinMode(PAD_MIDDLE_PIN, INPUT_PULLUP);

  // Setup ADC for battery monitoring
  pinMode(BATT_ADC, INPUT);

  // Setup GPS
  pinMode(GPS_ENABLE_PIN, OUTPUT);
  digitalWrite(GPS_ENABLE_PIN, HIGH);
  Serial1.begin(115200, SERIAL_8N1, 33, 34);
  Serial.begin(115200);

  // Setup display
  disp.st7735_init();
  disp.st7735_fill_screen(ST7735_BLACK);

  // Setup SD card
  if (!SD.begin(SD_CS))
  {
    disp.st7735_write_str(0, 0, "SD init failed!");
    return;
  }

  Serial.println("SD card success.");
  sd_card_init = true;

  if (SD.exists("config.txt"))
  {
    File configFile = SD.open("config.txt", "r");
    if (configFile)
    {
      // Read configuration from file
      while (configFile.available())
      {
        String line = configFile.readStringUntil('\n');
        line.trim();
        if (line.startsWith("TRACKING_INTERVAL="))
        {
          boardConfig.tracking_interval = line.substring(18).toInt();
        }
        else if (line.startsWith("TRACKING_DISTANCE="))
        {
          boardConfig.tracking_distance = line.substring(18).toFloat();
        }
        else if (line.startsWith("TRACK_DESC="))
        {
          boardConfig.track_desc = line.substring(11);
        }
        else if (line.startsWith("CALLSIGN="))
        {
          boardConfig.callsign = line.substring(9);
        }
      }
      configFile.close();
      Serial.println("Config loaded.");
    }
    else
    {
      Serial.println("Failed to open config file.");
    }
  }
  else
  {
    Serial.println("Config file not found. Using default settings.");
  }
}

void loop()
{
  if (millis() - prev_millis > CYCLE_TIME)
  {
    prev_millis = millis();
    PadAction action = get_action();
    if (action != NONE)
    {
      // Handle button actions here
      Serial.println("Button pressed: " + String(action));
    }
  }
  show_menu();

  if (Serial1.available() > 0)
  {
    if (Serial1.peek() != '\n')
    {
      GPS.encode(Serial1.read());
    }
    else
    {
      Serial1.read();

      disp.st7735_fill_screen(ST7735_BLACK);
      disp.st7735_write_str(0, 0, "GPS_test");
      String time_str = String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ":" + String(GPS.time.centisecond());
      disp.st7735_write_str(0, 20, time_str);
      String latitude = "LAT: " + String(GPS.location.lat());
      disp.st7735_write_str(0, 40, latitude);
      String longitude = "LON: " + String(GPS.location.lng());
      disp.st7735_write_str(0, 60, longitude);

      Serial.printf("GPS Data: %f %f %f\n", GPS.location.lat(), GPS.location.lng(), GPS.altitude.meters());

      while (Serial1.read() > 0)
        ;
    }
  }
}
