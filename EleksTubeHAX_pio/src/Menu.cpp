#include "Menu.h"

// Big ol' state machine: menu and buttons as the state, buttons as the transition triggers.

#ifndef ONE_BUTTON_ONLY_MENU
void Menu::loop(Buttons &buttons) {
  Button::state left_state = buttons.left.getState();   // decrement value  
  Button::state right_state = buttons.right.getState(); // increment value
  Button::state power_state = buttons.power.getState(); // exit menu
  Button::state mode_state = buttons.mode.getState();   // next menu

  #ifdef DEBUG_OUTPUT_MENU
        Serial.print("Menu::loop! state: ");Serial.print(state);
        Serial.print("; left_state: ");Serial.print(left_state);        
        Serial.print("; right_state: ");Serial.print(right_state);
        Serial.print("; power_state: ");Serial.println(power_state);
        Serial.print("; mode_state: ");Serial.print(mode_state);
  #endif

  // Reset the change value in every case.  We don't always change the state though.
  change = 0;
  state_changed = false;
  
  // Early out for idle state, which will be most of the time.
  if (state == idle && left_state == Button::idle && right_state == Button::idle && mode_state == Button::idle) {
    // Everything is idle.
    return;
  }

  // Go idle if the user hasn't pressed a button in a long time.
  if (state != idle && millis() - millis_last_button_press > idle_timeout_ms) {
    // Go idle.
    state = idle;
    state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Go idle if the user hasn't pressed a button in a long time.");
#endif
    return;
  }
  
  // Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.
  if (state == idle && (left_state == Button::down_edge || right_state == Button::down_edge || mode_state == Button::down_edge)) {
    state = states(1);  // Start at the beginning of the menu.

    millis_last_button_press = millis();
    state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.");
#endif
    return;
  }

  // Go to the next menu option
  if (state != idle && mode_state == Button::down_edge) {
    uint8_t new_state = (uint8_t(state) + 1) % num_states;
    if (new_state == 0) {
      new_state = 1;  // Skip over idle when incrementing through the menu.
    }
    state = states(new_state);

    millis_last_button_press = millis();
    state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.print("MENU: Go to the next menu option! New state: ");Serial.println(state);
#endif
    return;
  }

  // Exit with a power button.
  if (state != idle && (power_state == Button::down_edge)) {
    state = idle;
    state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Exit with a power button.");
#endif
    return;
  }

  // In a menu, and a left (negative change value) or right button (positive change value) has been pressed!
  if (state != idle && (left_state == Button::down_edge || right_state == Button::down_edge)) {
    // Pressing both left and right at the same time cancels out?  Sure, why not...
    if (left_state == Button::down_edge) {
      change--;
    }
    if (right_state == Button::down_edge) {
      change++;
    }

    millis_last_button_press = millis();
    state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.print("MENU: In a menu, and a left (negative change value) or right button (positive change value) has been pressed! Change: ");Serial.println(change);
#endif
    return;
  }
  // Some other button state, but it doesn't trigger any change in state.  There are LOTS of states that will
  // get here, but I think they're all "just do nothing."  If there's an explicit state we want to handle,
  // add an if() block above.
}
#endif

#ifdef ONE_BUTTON_ONLY_MENU
void Menu::loop(Buttons &buttons) {  
  Button::state buttonState = buttons.mode.getState();  
  currentButtonState = buttonState;
  switch (currentButtonState) {
    case Button::single_press:
        if (currentMenuState == idle) {
            // Enter the menu system
            currentMenuState = backlight_pattern; // Example starting point
        } else {
            // Select the current menu option
            selectCurrentOption();
        }
        break;
    case Button::double_press:
        // Navigate to the previous menu option
        currentMenuState = static_cast<states>((currentMenuState - 1 + num_states) % num_states);
        if (currentMenuState == idle) { // Skip idle when navigating
            currentMenuState = static_cast<states>((currentMenuState - 1 + num_states) % num_states);
        }
        break;
    case Button::triple_press:
        // Navigate to the next menu option
        currentMenuState = static_cast<states>((currentMenuState + 1) % num_states);
        if (currentMenuState == idle) { // Skip idle when navigating
            currentMenuState = static_cast<states>((currentMenuState + 1) % num_states);
        }
        break;
    default:
        // Handle other states or do nothing
        break;
  }
}

void Menu::selectCurrentOption() {
    // Implement the action for the selected menu option
    // This could involve changing a setting, displaying a submenu, etc.
    // Example:
    switch (currentMenuState) {
        case backlight_pattern:
            // Change the backlight pattern
            //setupMenu();            
            tfts->println("Pattern:");
            tfts->println(backlights->getPatternStr());
            break;
        case pattern_color:
            // Change the pattern color
            break;
        // Add cases for other menu options
        default:
            break;
    }
}

void Menu::setupMenu() {
  #ifdef DEBUG_OUTPUT
    Serial.println("Menu::setupMenu!");
  #endif  
  tfts->chip_select.setHoursTens();
  tfts->setTextColor(TFT_WHITE, TFT_BLACK, true);
  tfts->fillRect(0, 120, 135, 240, TFT_BLACK);
  tfts->setCursor(0, 124, 4);  // Font 4. 26 pixel high
}

void Menu::drawMenu() { 
  // Begin Draw Menu
  if (stateChanged() && tfts->isEnabled()) {
    Menu::states menu_state = getState();
    int8_t menu_change = getChange();

    if (menu_state == Menu::idle) {
      // We just changed into idle, so force redraw everything, and save the config.
      updateClockDisplay(TFTs::force);
      Serial.print("Saving config, after leaving menu...");
      stored_config.save();
      Serial.println(" Done.");
    }
    else {
      // Backlight Pattern
      if (menu_state == Menu::backlight_pattern) {
        if (menu_change != 0) {
          backlights.setNextPattern(menu_change);
          #ifdef DEBUG_OUTPUT
            Serial.println("set Next Pattern!");
          #endif
        }
        setupMenu();
        tfts.println("Pattern:");
        tfts.println(backlights.getPatternStr());
      }
      // Backlight Color
      else if (menu_state == Menu::pattern_color) {
        if (menu_change != 0) {
          backlights.adjustColorPhase(menu_change*16);
        }
        setupMenu();
        tfts.println("Color:");
        tfts.printf("%06X\n", backlights.getColor()); 
      }
      // Backlight Intensity
      else if (menu_state == Menu::backlight_intensity) {
        if (menu_change != 0) {
          backlights.adjustIntensity(menu_change);
        }
        setupMenu();
        tfts.println("Intensity:");
        tfts.println(backlights.getIntensity());
      }
      // 12 Hour or 24 Hour mode?
      else if (menu_state == Menu::twelve_hour) {
        if (menu_change != 0) {
          uclock.toggleTwelveHour();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);
        }        
        setupMenu();
        tfts.println("Hour format");
        tfts.println(uclock.getTwelveHour() ? "12 hour" : "24 hour"); 
      }
      // Blank leading zeros on the hours?
      else if (menu_state == Menu::blank_hours_zero) {
        if (menu_change != 0) {
          uclock.toggleBlankHoursZero();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::force);
        }        
        setupMenu();
        tfts.println("Blank zero?");
        tfts.println(uclock.getBlankHoursZero() ? "yes" : "no");
      }
      // UTC Offset, hours
      else if (menu_state == Menu::utc_offset_hour) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 3600);
          checkOnEveryFullHour();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::yes);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::yes);
        }
        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- Hour");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        if(offset_min < 0) {
          offset_min = -offset_min;
        }
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // UTC Offset, 15 minutes
      else if (menu_state == Menu::utc_offset_15m) {
        if (menu_change != 0) {
          uclock.adjustTimeZoneOffset(menu_change * 900);
          checkOnEveryFullHour();
          tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), TFTs::yes);
          tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::yes);
          tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), TFTs::yes);
          tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), TFTs::yes);
        }
        setupMenu();
        tfts.println("UTC Offset");
        tfts.println(" +/- 15m");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        if(offset_min < 0) {
          offset_min = -offset_min;
        }
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // select clock "font"
      else if (menu_state == Menu::selected_graphic) {
        if (menu_change != 0) {
          uclock.adjustClockGraphicsIdx(menu_change);

          if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
            tfts.current_graphic = uclock.getActiveGraphicIdx();
            updateClockDisplay(TFTs::force);   // redraw everything
          }
        }
        setupMenu();
        tfts.println("Selected");
        tfts.println(" graphic:");
        tfts.printf("    %d\n", uclock.getActiveGraphicIdx());
      }
#ifdef WIFI_USE_WPS   ////  WPS code
      // connect to WiFi using wps pushbutton mode
      else if (menu_state == Menu::start_wps) {
        if (menu_change != 0) { // button was pressed
          if (menu_change < 0) { // left button
            Serial.println("WiFi WPS start request");
            tfts.clear();
            tfts.fillScreen(TFT_BLACK);
            tfts.setTextColor(TFT_WHITE, TFT_BLACK);
            tfts.setCursor(0, 0, 4);  // Font 4. 26 pixel high
            WiFiStartWps();
          }
        }
        
        setupMenu();
        tfts.println("Connect to WiFi?");
        tfts.println("Left=WPS");
      }
#endif   
    }
  } // if (menu.stateChanged() && tfts.isEnabled())  
} //drawMenu

#endif

const String Menu::state_str[Menu::num_states] = { 
    "idle",
    "backlight_pattern",
    "pattern_color",
    "backlight_intensity",
    "twelve_hour",
    "blank_hours_zero",
    "utc_offset_hour",
    "utc_offset_15m",
    "selected_graphic",
    "start_wps"
  };
