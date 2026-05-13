#include "main.h"

TinyGPSPlus GPS;
GPSTracker tracker(&GPS);
LoRaAPRS aprs;
HT_st7735 disp;
BoardConfig boardConfig;

bool sd_card_init = false;
int menu_item = 0;
int cursor_pos = 0;
unsigned long prev_millis = 0;
DateTime last_gps_time;
String time_str;

void render_screen()
{
  switch (menu_item)
  {
  case 0: // draw main screen
    disp.st7735_fill_screen(ST7735_BLACK);
    last_gps_time = tracker.get_current_time();
    time_str = String(last_gps_time.hour) + ":" + String(last_gps_time.minute) + ":" + String(last_gps_time.second);
    disp.st7735_write_str(0, 0, time_str);

    disp.st7735_write_str(0, 80, String(read_battery_voltage(), 1) + "V");
    disp.st7735_write_str(20, 0, "Sat: " + String(GPS.satellites.value()));
    break;

  case 1: // draw tracking screen

    break;

  case 2: // draw settings screen
    for (int i = 0; i < 3; i++)
    {
      if (i == cursor_pos)
      {
        disp.st7735_write_str(0, 20 * i, "> " + String((MenuItems)i));
      }
      else
      {
        disp.st7735_write_str(0, 20 * i, "  " + String((MenuItems)i));
      }
    }
    break;

  default:
    break;
  }
}

float read_battery_voltage()
{
  return analogRead(BATT_ADC) * 4.9;
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

void run_tasks(uint16_t interval_ms)
{
  unsigned long start = millis();

  do
  {
    while (Serial1.available())
      GPS.encode(Serial1.read());
  } while (millis() - start < interval_ms);
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
  Serial1.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
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
  tracker.set_sd_card_init(true);

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
        else if (line.startsWith("SYMBOL="))
        {
          boardConfig.symbol = line.substring(7);
        }
        else if (line.startsWith("STATUS="))
        {
          boardConfig.status = line.substring(7);
        }
        else if (line.startsWith("POSITION_REPORT_INTERVAL="))
        {
          boardConfig.position_report_interval = line.substring(24).toInt();
        }
      }
      configFile.close();
      boardConfig.position_reports_enabled = ((boardConfig.position_report_interval > 0) && (boardConfig.callsign != "NOCALL"));
      Serial.println("Config loaded.");
      tracker.load_config(boardConfig.tracking_distance, boardConfig.tracking_interval, boardConfig.track_desc);
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

  aprs.init(boardConfig.callsign, boardConfig.symbol, boardConfig.status);
  aprs.assign_gps(&GPS);

  disp.st7735_write_str(20, 0, "Welcome,", Font_16x26);
  disp.st7735_write_str(20, 30, (boardConfig.callsign != "NOCALL") ? boardConfig.callsign : "User");
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
      if (action == UP && GPS.location.isValid())
      {
        aprs.send_location();
      }
    }
  }
  // render_screen();

  if (GPS.location.isUpdated())
  {
    disp.st7735_fill_screen(ST7735_BLACK);
    disp.st7735_write_str(0, 0, "GPS_test");
    String time_str = String(GPS.time.hour()) + ":" + String(GPS.time.minute()) + ":" + String(GPS.time.second()) + ":" + String(GPS.time.centisecond());
    disp.st7735_write_str(0, 20, time_str);
    String latitude = "LAT: " + String(GPS.location.lat());
    disp.st7735_write_str(0, 40, latitude);
    String longitude = "LON: " + String(GPS.location.lng());
    disp.st7735_write_str(0, 60, longitude);

    Serial.printf("GPS Data: %f %f %f\n", GPS.location.lat(), GPS.location.lng(), GPS.altitude.meters());
  }

  run_tasks(1000); // Run GPS encoding and other tasks for 10 ms
}
