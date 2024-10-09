#include "Menu.h"

// Big ol' state machine: menu and buttons as the state, buttons as the transition triggers.

#ifndef ONE_BUTTON_ONLY_MENU
void Menu::loop(Buttons &buttons) {
  Button::state left_state = buttons.left.getState();   // decrement value  
  Button::state right_state = buttons.right.getState(); // increment value
  Button::state power_state = buttons.power.getState(); // exit menu
  Button::state mode_state = buttons.mode.getState();   // next menu

  // Reset the change value in every case.  We don't always change the state though.
  change = 0;
  menu_state_changed = false;
  
  // Early out for idle state, which will be most of the time.
  if (menu_state == idle && left_state == Button::idle && right_state == Button::idle && mode_state == Button::idle) {
    // Everything is idle.
    return;
  }

  // Go idle if the user hasn't pressed a button in a long time.
  if (menu_state != idle && millis() - millis_last_button_press > idle_timeout_ms) {
    // Go idle.
    menu_state = idle;
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Go idle if the user hasn't pressed a button in a long time.");
#endif
    return;
  }
  
  // Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.
  if (menu_state == idle && (left_state == Button::up_edge || right_state == Button::up_edge || mode_state == Button::up_edge)) {
    menu_state = states(1);  // Start at the beginning of the menu.

    millis_last_button_press = millis();
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.");
#endif
    return;
  }

  // Go to the next menu option
  if (menu_state != idle && mode_state == Button::up_edge) {
    uint8_t new_state = (uint8_t(menu_state) + 1) % num_states;
    if (new_state == 0) {
      new_state = 1;  // Skip over idle when incrementing through the menu.
    }
    menu_state = states(new_state);

    millis_last_button_press = millis();
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.print("MENU: Go to the next menu option! New menu_state: ");Serial.println(menu_state);
#endif
    return;
  }

  // Exit with a power button.
  if (menu_state != idle && (power_state == Button::up_edge)) {
    menu_state = idle;
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.println("MENU: Exit with a power button.");
#endif
    return;
  }

  // In a menu, and a left (negative change value) or right button (positive change value) has been pressed!
  if (menu_state != idle && (left_state == Button::up_edge || right_state == Button::up_edge)) {
    // Pressing both left and right at the same time cancels out?  Sure, why not...
    if (left_state == Button::up_edge) {
      change--;
    }
    if (right_state == Button::up_edge) {
      change++;
    }

    millis_last_button_press = millis();
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
        Serial.print("MENU: In a menu, and a left (negative change value) or right button (positive change value) has been pressed! Change: ");Serial.println(change);
#endif
    return;
  }
  // Some other button state, but it doesn't trigger any change in menu_state.  There are LOTS of states that will
  // get here, but I think they're all "just do nothing."  If there's an explicit state we want to handle,
  // add an if() block above.
}
#endif

#ifdef ONE_BUTTON_ONLY_MENU
void Menu::loop(Buttons &buttons) {
  Button::state onlybutton_state = buttons.mode.getState();   // next menu

  // Reset the change value in every case.  We don't always change the state though.
  change = 0;
  menu_state_changed = false;
  
  // Early out for idle state, which will be most of the time.
  if (menu_state == idle && onlybutton_state == Button::idle) {
    // Everything is idle. Do nothing.
    return;
  }

  // Go idle if the user hasn't pressed a button in a long time.
  if (menu_state != idle && millis() - millis_last_button_press > idle_timeout_ms) {
    // Go idle.
    menu_state = idle;
    menu_state_changed = true;
#ifdef DEBUG_OUTPUT_MENU
      Serial.println("MENU: Go idle if the user hasn't pressed a button in a long time.");
      Serial.println("-----------------------------------------------------------------------------------");
#endif
    return;
  } 
  
  // Menu is idle. A button is pressed, go into the menu, but don't act on the button press. It just wakes up the menu.
  //check 
  // (millis_last_button_press + double_click_ms) < millis() && onlybutton_state == Button::up_edge
  if (menu_state == idle && (onlybutton_state == Button::single_click)) {
    menu_state = states(1);  // Start at the beginning of the menu.

    millis_last_button_press = millis();
    menu_state_changed = true;
    //buttons.mode.resetState();
#ifdef DEBUG_OUTPUT_MENU
      Serial.println("MENU: Menu was idle. A single_click is detected. So changed the menu_state to 1, that will wake up the menu in the drawMenu loop.");
      Serial.print("MENU: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(Button::state_str[onlybutton_state]);
      Serial.println("-----------------------------------------------------------------------------------");
#endif
    return;
  }

//   //handle long click here, but don't show the menu, just change the clock face index +1
//   //has actually nothing to do with the menu, but is handled here, because the menu is the only thing, which is always checked
//   if (menu_state == idle && (onlybutton_state == Button::long_click)) {
//     uclock.adjustClockGraphicsIdx(1);
//     buttons.mode.resetState();
// #ifdef DEBUG_OUTPUT_MENU
//       Serial.println("MENU/CONTROL: Menu was idle. A long button press was detected and handled.");
//       Serial.print("MENU/CONTROL: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(onlybutton_state);
// #endif
//     return;
//   }

//   //handle double click here, but don't show the menu, just change the clock face index -1
//   //has actually nothing to do with the menu, but is handled here, because the menu is the only thing, which is always checked
//   if (menu_state == idle && (onlybutton_state == Button::double_click)) {
//     uclock.adjustClockGraphicsIdx(-1);
//     buttons.mode.resetState();
// #ifdef DEBUG_OUTPUT_MENU
//       Serial.println("MENU/CONTROL: Menu was idle. A double_click was detected and handled.");
//       Serial.print("MENU/CONTROL: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(onlybutton_state);
// #endif
//     return;
//   }

  // In a menu, and button long pressed! -> simulate right button press
  // Must be done BEFORE the next menu option
  if (menu_state != idle && (onlybutton_state == Button::long_click)) {
    change--;

    millis_last_button_press = millis();
    menu_state_changed = true;
    //buttons.mode.resetState();
    #ifdef DEBUG_OUTPUT_MENU
      Serial.println("MENU: In the menu, and button pressed long time! -> simulate left button press!");
      Serial.print("MENU: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(Button::state_str[onlybutton_state]);
      Serial.println("-----------------------------------------------------------------------------------");
    #endif
    return;
  }

  //onlybutton_state == Button::triple_click || 

  // In the menu, and the button is double pressed! -> simulate right button press
  // Must be done BEFORE the next menu option
  if (menu_state != idle && (onlybutton_state == Button::double_click)) {
    change++;

    millis_last_button_press = millis();
    menu_state_changed = true;
    //buttons.mode.resetState();
    #ifdef DEBUG_OUTPUT_MENU
      Serial.println("MENU: In the menu, and button released after double click pressed! -> simulate right button press!");
      Serial.print("MENU: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(Button::state_str[onlybutton_state]);
      Serial.println("-----------------------------------------------------------------------------------");
    #endif
    return;
  }

  // Go to the next menu option
  if (menu_state != idle && onlybutton_state == Button::single_click) {
    uint8_t new_state = (uint8_t(menu_state) + 1) % num_states;
    if (new_state == 0) {
      new_state = 1;  // Skip over idle when incrementing through the menu.
    }
    menu_state = states(new_state);

    millis_last_button_press = millis();
    menu_state_changed = true;
    //buttons.mode.resetState();
    #ifdef DEBUG_OUTPUT_MENU
      Serial.print("MENU: Go to the next menu option! New menu_state: ");Serial.println(menu_state);      
      Serial.print("MENU: menu_state: ");Serial.print(menu_state);Serial.print("; onlybutton_state: ");Serial.println(Button::state_str[onlybutton_state]);
      Serial.println("-----------------------------------------------------------------------------------");
    #endif
    return;
  }

  // Some other button state, but it doesn't trigger any change in menu_state.  There are LOTS of states that will
  // get here, but I think they're all "just do nothing."  If there's an explicit state we want to handle,
  // add an if() block above.
}
#endif





#ifndef WIFI_USE_WPS
const String Menu::state_str[Menu::num_states] = { 
    "idle",
    "selected_graphic",
    "pattern_color",
    "backlight_intensity",
    "twelve_hour",
    "blank_hours_zero",
    "utc_offset_hour",
    "utc_offset_15m",
    "dimming_begin",
    "dimming_end",
    "backlight_pattern"
  };
  #else
  const String Menu::state_str[Menu::num_states] = { 
    "idle",
    "selected_graphic",
    "pattern_color",
    "backlight_intensity",
    "twelve_hour",
    "blank_hours_zero",
    "utc_offset_hour",
    "utc_offset_15m",
    "dimming_begin",
    "dimming_end",
    "backlight_pattern",
    "start_wps"
  };
  #endif
