#include "main.h"

TinyGPSPlus gps;
Adafruit_ST7735 disp(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
SX1262 radio = new Module(RADIO_CS_PIN, RADIO_DIO1_PIN, RADIO_RST_PIN, RADIO_BUSY_PIN);
LoRaAPRS aprs(&gps, &radio);
GPSTracker tracker(&gps);
BoardConfig boardConfig;

bool sd_card_init = false;
int screen_id = 0;
int cursor_pos = 0;
unsigned long prev_millis = 0;
DateTime last_gps_time;
String time_str;
String message_str;

void init_display()
{
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_CS, OUTPUT);

  disp.initR(INITR_MINI160x80_PLUGIN);
  disp.setRotation(1); // Landscape
  disp.fillScreen(ST77XX_BLACK);
  disp.setTextColor(ST77XX_BLUE, ST77XX_BLACK);

  delay(50); // Wait for screen to clear

  // Enable backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
}

void render_screen()
{
  switch (screen_id)
  {
  case 0: // draw main screen
    // disp.fillScreen(ST77XX_BLACK);
    last_gps_time = tracker.get_current_time(); // timezone is not handled, so this will be UTC time
    time_str = (last_gps_time.hour < 10 ? "0" : "") + String(last_gps_time.hour) + ":" + (last_gps_time.minute < 10 ? "0" : "") + String(last_gps_time.minute) + ":" + (last_gps_time.second < 10 ? "0" : "") + String(last_gps_time.second);
    disp.setCursor(0, 0);
    disp.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
    disp.setTextSize(2);
    disp.print(time_str);

    disp.setCursor(125, 0);
    disp.setTextSize(1);
    disp.print(String(read_battery_voltage(), 1) + "V");

    disp.setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    disp.setCursor(0, 20);
    disp.print("Sat: " + String(gps.satellites.value()));
    disp.setCursor(60, 20);
    disp.print("Alt: " + String(gps.altitude.meters()) + "m");

    disp.setCursor(0, 30);
    disp.print("Lat: " + String(gps.location.lat(), 4));
    disp.setCursor(75, 30);
    disp.print("Lon: " + String(gps.location.lng(), 4));
    break;

  case 1: // draw tracking screen

    break;

  case 2: // draw settings screen
    for (int i = 0; i < 3; i++)
    {
      if (i == cursor_pos)
      {
        disp.setCursor(0, 20 * i);
        disp.print("> " + String((MenuItems)i));
      }
      else
      {
        disp.setCursor(0, 20 * i);
        disp.print("  " + String((MenuItems)i));
      }
    }
    break;

  case 3: // draw message screen
    if (!message_str.isEmpty())
    {
      disp.fillScreen(ST77XX_WHITE);
      disp.setCursor(0, 0);
      disp.print("Info");
      disp.setCursor(0, 20);
      disp.print(message_str);
    }
    else
    {
      screen_id = 0; // return to main screen if no message to show
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
      gps.encode(Serial1.read());
  } while (millis() - start < interval_ms);

  if (gps.location.isUpdated() && tracker.is_tracking_active())
  {
    tracker.track_point();
  }
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
  init_display();

  // Setup SD card
  if (!SD.begin(SD_CS))
  {
    disp.setCursor(0, 0);
    disp.print("SD init failed!");
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
          boardConfig.track_desc = line.substring(11).c_str();
        }
        else if (line.startsWith("CALLSIGN="))
        {
          boardConfig.callsign = line.substring(9).c_str();
        }
        else if (line.startsWith("SYMBOL="))
        {
          boardConfig.symbol = line.substring(7).c_str();
        }
        else if (line.startsWith("STATUS="))
        {
          boardConfig.status = line.substring(7).c_str();
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

  if (boardConfig.position_reports_enabled) // Initialize APRS if position reports are enabled
  {
    aprs.init(boardConfig.callsign, boardConfig.symbol, boardConfig.status);
  }

  disp.setCursor(20, 0);
  disp.setTextColor(ST77XX_BLUE, ST77XX_BLACK);
  disp.print("Welcome,");
  disp.setCursor(20, 30);
  disp.print((boardConfig.callsign != "NOCALL") ? String(boardConfig.callsign.c_str()) : "User");
  delay(1000);
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
      if (action == UP && gps.location.isValid() && boardConfig.position_reports_enabled)
      {
        aprs.send_position_report();
      }
      else if (action == DOWN && gps.location.isValid())
      {
        if (!tracker.is_tracking_active())
        {
          tracker.begin_tracking();
        }
        else
        {
          tracker.end_tracking();
        }
      }
      else if (action == MIDDLE && gps.location.isValid())
      {
        tracker.save_waypoint();
        tracker.save_waypoint_csv();
      }
    }
  }
  render_screen();

  run_tasks(500); // Run GPS encoding and other tasks for 500 ms
}
