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
    display_text(0, 0, time_str, ST77XX_BLUE, 2);
    display_text(125, 0, String(read_battery_voltage(), 1) + "V");
    display_text(0, 20, "Sat:" + String(gps.satellites.value()) + " Alt: " + String(gps.altitude.meters()) + "m", ST77XX_GREEN);
    display_text(0, 30, "Lat:" + String(gps.location.lat(), 5) + " Lon:" + String(gps.location.lng(), 5), ST77XX_CYAN);
    display_text(0, 40, "HDOP:"+ String(gps.hdop.hdop(), 1), ST77XX_ORANGE);
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
      disp.drawRect(2, 2, DISP_WIDTH - 4, DISP_HEIGHT - 4, ST77XX_ORANGE);
      disp.fillRect(8, 8, 8, 8, ST77XX_ORANGE);
      disp.fillRect(8, 22, 8, 22, ST77XX_ORANGE);
      display_text(22, 6, "Info", ST77XX_ORANGE, 2);      
      display_text(22, 26, message_str, ST77XX_ORANGE);
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

void display_text(int x, int y, const String &text, uint16_t text_color, int text_size, uint16_t bg_color)
{
  disp.setCursor(x, y);
  disp.setTextColor(text_color, bg_color);
  disp.setTextSize(text_size);
  disp.print(text);
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

  display_text(20, 0, "Welcome,", ST77XX_GREEN);
  display_text(20, 30, (boardConfig.callsign != "NOCALL") ? String(boardConfig.callsign.c_str()) : "User", ST77XX_GREEN);
  delay(1000);
  disp.fillScreen(ST77XX_BLACK);
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
