#include "main.h"

void init_display()
{
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_CS, OUTPUT);

  disp.initR(INITR_MINI160x80_PLUGIN);
  disp.setRotation(1); // Landscape
  disp.fillScreen(ST77XX_BLACK);

  delay(50); // Wait for screen to clear

  // Enable backlight
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
}

void render_screen()
{
  screen_id = message_str.empty() ? screen_id : Screens::WARNING_SCREEN; // If there's a message, show the message screen

  switch (screen_id)
  {
  case Screens::MAIN_SCREEN:
  { // draw main screen
    // convert GPS time to local time
    last_gps_time = tracker.get_current_time(); // timezone is not handled, so this will be UTC time
    time_t local_time = timezone_obj.toLocal(makeTime(last_gps_time));

    time_str =
        string(hour(local_time) < 10 ? "0" : "") + to_string(hour(local_time)) + ":" +
        string(minute(local_time) < 10 ? "0" : "") + to_string(minute(local_time)) + ":" +
        string(second(local_time) < 10 ? "0" : "") + to_string(second(local_time));

    display_text(0, 0, time_str, ST77XX_BLUE, 2);
    display_text(0, 19, to_string_rounded(gps.speed.kmph(), 1) + " km/h", ST77XX_RED, 2);

    display_text(110, 19, "Sat: " + to_string(gps.satellites.value()) + (gps.satellites.value() < 10 ? " " : ""), ST77XX_GREEN);
    display_text(110, 29, "HP:" + to_string_rounded(gps.hdop.hdop(), 1), ST77XX_ORANGE);

    display_text(0, 39, to_string_rounded(gps.altitude.meters(), 1) + " m", ST77XX_CYAN, 2);

    display_text(0, 60, "Lat:" + to_string_rounded(gps.location.lat(), 5) + " Lon:" + to_string_rounded(gps.location.lng(), 5), ST77XX_GREEN);

    if (tracker.is_tracking_active())
    {
      disp.fillCircle(125, 6, 4, ST77XX_RED);
      display_text(133, 4, to_string(tracker.get_recorded_points()), ST77XX_WHITE);
    }

    draw_page_indicators(Screens::MAIN_SCREEN);
    break;
  }

  case Screens::NAVIGATION_SCREEN: // draw navigation screen
    draw_page_indicators(Screens::NAVIGATION_SCREEN);
    break;

  case Screens::EXTRA_SCREEN: // extra base screen
    display_text(0, 0, to_string_rounded(gps.speed.kmph(), 1) + " km/h", ST77XX_RED, 3);
    display_text(0, 29, to_string_rounded(gps.altitude.meters(), 1) + " m", ST7735_CYAN, 2);

    draw_page_indicators(Screens::EXTRA_SCREEN);
    break;

  case Screens::SETTINGS_SCREEN: // draw settings screen
    for (int i = 0; i < MENU_ITEMS_COUNT; i++)
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

  case Screens::WAYPOINT_SCREEN:                                          // draw waypoint selection screen
    stored_waypoints.push_back(Waypoint({"Back to menu", 0.0, 0.0, -1})); // add return as "dummy" waypoint
    for (int i = 0; i < stored_waypoints.size(); i++)
    {
      if (i == cursor_pos)
      {
        display_text(3, (11 * i) + 3, "> " + stored_waypoints.at(i).name, ST77XX_BLUE);
      }
      else
      {
        display_text(3, (11 * i) + 3, "  " + stored_waypoints.at(i).name, ST77XX_BLUE);
      }
    }
    stored_waypoints.pop_back(); // remove extra menu option
    break;

  case Screens::WARNING_SCREEN: // draw warning screen
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
      screen_id = Screens::MAIN_SCREEN; // return to main screen if no message to show
      disp.fillScreen(ST77XX_BLACK);
    }
    break;

  default:
    break;
  }
}

void exit_menu()
{
  screen_id = Screens::MAIN_SCREEN;
  cursor_pos = 0;
  disp.fillScreen(ST77XX_BLACK);
}

void IRAM_ATTR button_handler(PadDirection btn_id)
{
  num_items = screen_id == Screens::SETTINGS_SCREEN ? MENU_ITEMS_COUNT : stored_waypoints.size();

  switch (btn_id)
  {
  case PadDirection::UP:
    if ((screen_id == Screens::SETTINGS_SCREEN) || (screen_id == Screens::WAYPOINT_SCREEN))
    {
      cursor_pos = (cursor_pos - 1 + num_items) % num_items; // Wrap around the menu items
    }
    break;

  case PadDirection::DOWN:
    if ((screen_id == Screens::SETTINGS_SCREEN) || (screen_id == Screens::WAYPOINT_SCREEN))
    {
      cursor_pos = (cursor_pos + 1) % num_items; // Wrap around the menu items
    }
    break;

  case PadDirection::MIDDLE:
    if (!message_str.empty())
    {
      message_str = ""; // Dismiss message
    }
    else if (screen_id == Screens::MAIN_SCREEN)
    {
      screen_id = Screens::SETTINGS_SCREEN;
      disp.fillScreen(ST77XX_BLACK);
    }
    else if (screen_id == WAYPOINT_SCREEN)
    {
      screen_id = cursor_pos < stored_waypoints.size() ? Screens::MAIN_SCREEN : Screens::SETTINGS_SCREEN;
      disp.fillScreen(ST77XX_BLACK);
      cursor_pos = 0;
    }
    else if (screen_id == Screens::SETTINGS_SCREEN)
    {
      switch (cursor_pos)
      {
      case EXIT: // other options use the same code, so they can use default block
        exit_menu();
        break;

      default:
        int_flag = cursor_pos;
        exit_menu();
      }
    }
    break;

  case PadDirection::LEFT:
    screen_id = (screen_id - 1 + SCREEN_COUNT) % SCREEN_COUNT;
    disp.fillScreen(ST77XX_BLACK);
    break;

  case PadDirection::RIGHT:
    screen_id = (screen_id + 1) % SCREEN_COUNT;
    disp.fillScreen(ST77XX_BLACK);
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

void draw_page_indicators(int current_page, int total_pages)
{
  for (int i = 0; i < total_pages; i++)
  {
    if (i == current_page)
    {
      disp.fillCircle(((DISP_WIDTH / 2) - 10) + (i * 10), 74, 3, ST7735_CYAN);
    }
    else
    {
      disp.drawCircle(((DISP_WIDTH / 2) - 10) + (i * 10), 74, 3, ST7735_BLUE);
    }
  }
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

  switch (int_flag)
  {
  case START_TRACKING:
    if (tracker.is_tracking_active())
    {
      tracker.end_tracking();
      menu_items[START_TRACKING] = "Start Tracking"; // Change menu item back to "Start Tracking"
    }
    else
    {
      tracker.begin_tracking();
      menu_items[START_TRACKING] = "Stop Tracking"; // Change menu item to "Stop Tracking"
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

  case SHOW_WAYPOINTS:
    stored_waypoints = tracker.get_waypoints();
    if (stored_waypoints.size() > 0)
    {
      screen_id = Screens::WAYPOINT_SCREEN; // show waypoint selection screen
    }
    else
    {
      message_str = "No waypoints loaded!";
    }
    int_flag = -1; // Reset flag
    break;
  }
}

void setup()
{
  // Setup ADC for battery monitoring
  // pinMode(BATT_ADC, INPUT);

  // Setup GPS
  pinMode(GPS_ENABLE_PIN, OUTPUT);
  digitalWrite(GPS_ENABLE_PIN, HIGH);
  Serial1.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.begin(115200);

  // Setup buttons, interrupts will be attached at the end of setup
  pinMode(PAD_UP_PIN, INPUT_PULLUP);
  pinMode(PAD_DOWN_PIN, INPUT_PULLUP);
  pinMode(PAD_LEFT_PIN, INPUT_PULLUP);
  pinMode(PAD_RIGHT_PIN, INPUT_PULLUP);
  pinMode(PAD_MIDDLE_PIN, INPUT_PULLUP);

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
        }

        configFile.close();
        tracker.load_config(boardConfig.tracking_distance, boardConfig.tracking_interval, boardConfig.track_desc);
      }
      else
      {
        message_str = "Failed to open config file!";
      }
    }

    if (SD.exists("/.tracking_active")) // tracking was interrupted by board reset, restore it here
    {
      File tracking_file = SD.open("/.tracking_active", "r");
      if (tracking_file)
      {
        tracker.restore_tracking(tracking_file.readStringUntil(EOF).c_str());
        message_str = "Tracking was restored from file: " + tracker.get_filename();
        menu_items[START_TRACKING] = "Stop Tracking"; // update menu to reflect this change
      }
    }
  }

  // enter FTP mode
  if (digitalRead(PAD_MIDDLE_PIN) == LOW) // FTP mode for accessing SD card
  {
    ftp_mode = true;
    bool res = init_ftp();

    display_text(30, 20, "FTP mode", ST77XX_RED, 2);
    if (!res)
    {
      display_text(30, 40, "Failed to start AP", ST77XX_RED);
      return; // Skip the rest of the setup if FTP mode fails
    }

    display_text(30, 40, "IP: " + string(WiFi.softAPIP().toString().c_str()), ST77XX_BLUE);
    return; // Skip the rest of the setup if FTP mode is active
  }

  // Setup button interrupts
  attachInterrupt(digitalPinToInterrupt(PAD_UP_PIN), []()
                  { button_handler(PadDirection::UP); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_DOWN_PIN), []()
                  { button_handler(PadDirection::DOWN); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_LEFT_PIN), []()
                  { button_handler(PadDirection::LEFT); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_RIGHT_PIN), []()
                  { button_handler(PadDirection::RIGHT); }, FALLING);
  attachInterrupt(digitalPinToInterrupt(PAD_MIDDLE_PIN), []()
                  { button_handler(PadDirection::MIDDLE); }, FALLING);

  display_text(30, 20, "Welcome,", ST77XX_YELLOW, 2);
  display_text(30, 40, (boardConfig.callsign != "NOCALL") ? boardConfig.callsign : "User", ST77XX_YELLOW, 2);
  delay(1000);
  disp.fillScreen(ST77XX_BLACK);
}

void loop()
{
  if (!ftp_mode)
  {
    if (millis() - prev_millis > CYCLE_TIME)
    {
      prev_millis = millis();
      // do stuff here every CYCLE_TIME milliseconds
    }

    render_screen();
    run_tasks(500); // Run GPS encoding and other tasks for 500 ms
  }
  else
  {
    ftp.handleFTP(); // Handle FTP requests
    display_text(30, 50, "Connected: " + to_string(WiFi.softAPgetStationNum()), ST77XX_BLUE);
  }
}

// ----- FTP methods -----
bool init_ftp()
{
  if (!WiFi.softAP(WIFI_SSID, FTP_PASSWORD))
  {
    return false;
  }
  WiFi.setSleep(false);

  ftp.begin(FTP_USER, FTP_PASSWORD);
  return true;
}