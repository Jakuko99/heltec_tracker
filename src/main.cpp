#include "main.h"

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
  screen_id = message_str.empty() ? screen_id : 3; // If there's a message, show the message screen

  switch (screen_id)
  {
  case 0:
  { // draw main screen
    // convert GPS time to local time
    last_gps_time = tracker.get_current_time(); // timezone is not handled, so this will be UTC time
    time_t utc_time = makeTime(last_gps_time);
    time_t local_time = timezone_obj.toLocal(utc_time);

    time_str =
        string(hour(local_time) < 10 ? "0" : "") + to_string(hour(local_time)) + ":" +
        string(minute(local_time) < 10 ? "0" : "") + to_string(minute(local_time)) + ":" +
        string(second(local_time) < 10 ? "0" : "") + to_string(second(local_time));

    display_text(0, 0, time_str, ST77XX_BLUE, 2);
    display_text(0, 19, to_string_rounded(gps.speed.kmph(), 1) + " km/h", ST77XX_RED, 2);

    display_text(110, 19, "Sat: " + to_string(gps.satellites.value()), ST77XX_GREEN);
    display_text(110, 29, "HP:" + to_string_rounded(gps.hdop.hdop(), 1), ST77XX_ORANGE);

    display_text(0, 39, to_string_rounded(gps.altitude.meters(), 1) + " m", ST77XX_CYAN, 2);

    display_text(0, 60, "Lat:" + to_string_rounded(gps.location.lat(), 5) + " Lon:" + to_string_rounded(gps.location.lng(), 5), ST77XX_GREEN);

    if (tracker.is_tracking_active())
    {
      disp.fillCircle(125, 6, 4, ST77XX_RED);
      display_text(133, 4, to_string(tracker.get_recorded_points()), ST77XX_WHITE);
    }

    if (boardConfig.position_reports_enabled)
    {
      disp.fillCircle(112, 6, 4, ST77XX_ORANGE);
    }
    break;
  }

  case 1: // draw navigation screen

    break;

  case 2: // draw settings screen
    for (int i = 0; i < 4; i++)
    {
      if (i == cursor_pos)
      {
        display_text(3, (11 * i) + 3, "> " + menu_items[i], ST77XX_BLUE);
      }
      else
      {
        display_text(3, (11 * i) + 3, "  " + menu_items[i], ST77XX_BLUE);
      }
    }
    break;

  case 3: // draw warning screen
    if (!message_str.empty())
    {
      disp.drawRect(2, 2, DISP_WIDTH - 4, DISP_HEIGHT - 4, ST77XX_ORANGE);
      disp.fillRect(8, 8, 8, 22, ST77XX_ORANGE);
      disp.fillCircle(12, 38, 4, ST77XX_ORANGE);
      display_text(22, 7, "Warning", ST77XX_ORANGE, 2);
      display_wrapped_text(22, 26, message_str, DISP_WIDTH - 5, ST77XX_ORANGE);
      display_text(24, DISP_HEIGHT - 14, "Press OK to dismiss", ST77XX_ORANGE);
    }
    else
    {
      screen_id = 0; // return to main screen if no message to show
      disp.fillScreen(ST77XX_BLACK);
    }
    break;

  default:
    break;
  }
}

void exit_menu()
{
  screen_id = 0;
  cursor_pos = 0;
  disp.fillScreen(ST77XX_BLACK);
}

void IRAM_ATTR button_handler(int btn_id)
{
  switch (btn_id)
  {
  case UP:

    if (screen_id == 2)
    {
      cursor_pos = (cursor_pos - 1 + 4) % 4; // Wrap around the menu items
    }
    break;
  case DOWN:
    if (screen_id == 2)
    {
      cursor_pos = (cursor_pos + 1) % 4; // Wrap around the menu items
    }
    break;
  case MIDDLE:
    if (!message_str.empty())
    {
      message_str = ""; // Dismiss message
    }
    else if (screen_id == 0)
    {
      screen_id = 2;
      disp.fillScreen(ST77XX_BLACK);
    }
    else if (screen_id == 2)
    {
      switch (cursor_pos)
      {
      case START_TRACKING:
        int_flag = START_TRACKING; // Set flag to start/stop tracking
        exit_menu();
        break;
      case SAVE_WAYPOINT:
        int_flag = SAVE_WAYPOINT; // Set flag to save waypoint
        exit_menu();
        break;
      case SEND_POSITION:
        int_flag = SEND_POSITION; // Set flag to send position
        exit_menu();
        break;
      case EXIT:
        exit_menu();
        break;
      }
    }
    break;
  }
}

string to_string_rounded(double value, int decimals)
{
  double factor = pow(10, decimals);
  value = std::round(value * factor) / factor;
  ostringstream out;
  out.precision(decimals);
  out << fixed << value;
  ;
  return move(out).str();
}

void display_text(int x, int y, const string &text, uint16_t text_color, int text_size, uint16_t bg_color)
{
  disp.setCursor(x, y);
  disp.setTextColor(text_color, bg_color);
  disp.setTextSize(text_size);
  disp.print(String(text.c_str()));
}

void display_wrapped_text(int x, int y, const string &text, int line_end, uint16_t text_color, int text_size, uint16_t bg_color)
{
  disp.setTextWrap(false); // Disable text wrapping
  disp.setTextColor(text_color, bg_color);
  disp.setTextSize(text_size);

  int cursorY = y;
  String line;

  for (int i = 0; i < text.length(); i++)
  {
    // Handle explicit newlines
    if (text[i] == '\n')
    {
      disp.setCursor(x, cursorY);
      disp.print(line);
      int16_t x1, y1;
      uint16_t w, h;
      disp.getTextBounds(line, x, cursorY, &x1, &y1, &w, &h);
      cursorY += h + 2;
      line = "";
      continue;
    }

    String test = line + text[i];
    int16_t x1, y1;
    uint16_t w, h;
    disp.getTextBounds(test, x, cursorY, &x1, &y1, &w, &h);

    if (x + w > line_end && line.length() > 0)
    {
      // Current line is full
      disp.setCursor(x, cursorY);
      disp.print(line);
      cursorY += h + 2;
      line = text[i]; // Start new line with current character
    }
    else
    {
      line = test;
    }
  }

  // Print remaining text
  if (line.length())
  {
    disp.setCursor(x, cursorY);
    disp.print(line);
  }

  disp.setTextWrap(true); // Re-enable text wrapping
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
  if (gps.location.isUpdated() && boardConfig.position_reports_enabled)
  {
    if (last_report_time == nullptr || tracker.time_between(*last_report_time, last_gps_time) >= boardConfig.position_report_interval)
    {
      if (!aprs.send_position_report())
      {
        message_str = "Failed to send position report.";
      }
      last_report_time.reset(new tmElements_t(last_gps_time));
    }
  }

  switch (int_flag)
  {
  case START_TRACKING:
    if (tracker.is_tracking_active())
    {
      tracker.end_tracking();
      menu_items[0] = "Start Tracking"; // Change menu item back to "Start Tracking"
    }
    else
    {
      tracker.begin_tracking();
      menu_items[0] = "Stop Tracking"; // Change menu item to "Stop Tracking"
    }
    int_flag = -1; // Reset flag
    break;
  case SAVE_WAYPOINT:
    if (gps.location.isValid())
    {
      tracker.save_waypoint();
      tracker.save_waypoint_csv();
    }
    int_flag = -1; // Reset flag
    break;
  case SEND_POSITION:
    if (gps.location.isValid() && boardConfig.position_reports_enabled)
    {
      if (aprs.send_position_report())
      {
        message_str = "Position report sent successfully.";
      }
    }
    int_flag = -1; // Reset flag
    break;
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

  // Setup button interrupts
  attachInterrupt(digitalPinToInterrupt(PAD_UP_PIN), []()
                  { button_handler(UP); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_DOWN_PIN), []()
                  { button_handler(DOWN); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_LEFT_PIN), []()
                  { button_handler(LEFT); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_RIGHT_PIN), []()
                  { button_handler(RIGHT); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_MIDDLE_PIN), []()
                  { button_handler(MIDDLE); }, FALLING);

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
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS))
  {
#ifndef DEBUG
    message_str = "SD card not detected. Insert an SD card to enable tracking functionality.";
#endif
  }
  else
  {
    sd_card_init = true;
    tracker.set_sd_card_init(true);

    if (SD.exists("/config.txt"))
    {
      File configFile = SD.open("/config.txt", "r");
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
        tracker.load_config(boardConfig.tracking_distance, boardConfig.tracking_interval, boardConfig.track_desc);
      }
      else
      {
        message_str = "Failed to open config file!";
      }
    }
  }

  if (boardConfig.position_reports_enabled) // Initialize APRS if position reports are enabled
  {
    aprs.init(boardConfig.callsign, boardConfig.symbol, boardConfig.status);
  }

  display_text(30, 20, "Welcome,", ST77XX_YELLOW, 2);
  display_text(30, 40, (boardConfig.callsign != "NOCALL") ? boardConfig.callsign : "User", ST77XX_YELLOW, 2);
  delay(1000);
  disp.fillScreen(ST77XX_BLACK);
}

void loop()
{
  if (millis() - prev_millis > CYCLE_TIME)
  {
    prev_millis = millis();
    // do stuff here every CYCLE_TIME milliseconds
  }

  render_screen();
  run_tasks(500); // Run GPS encoding and other tasks for 500 ms
}
