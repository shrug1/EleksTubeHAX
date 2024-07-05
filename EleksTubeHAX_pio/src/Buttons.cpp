#include "Buttons.h"

void Button::begin() {
  millis_at_last_transition = millis();
  millis_at_last_loop = millis_at_last_transition;
#ifdef DEBUG_OUTPUT
    Serial.print("init button: ");Serial.println(bpin);
#endif
  pinMode(bpin, INPUT);  
  down_last_time = isButtonDown();
  if (down_last_time) {
    button_state = down_edge;
  } else {
    button_state = idle;
  }
}

void Button::loop() {
  millis_at_last_loop = millis();
  bool down_now = isButtonDown();
  #ifdef DEBUG_OUTPUT
  if (down_now) {
    Serial.print("[B ");Serial.print(bpin);Serial.println("]");
  }
  #endif

  state previous_state = button_state;
  
    // Check button state transitions
  if (down_last_time == false && down_now == true) { // Just pressed
    press_count++;
    last_press_time = millis_at_last_loop;
    button_state = down_edge;
  } else if (down_last_time == true && down_now == false) { // Just released
    button_state = idle;
  }

  // Check for press count within the time window
  if (millis_at_last_loop - last_press_time > PRESS_COUNT_RESET_MS) {
    if (press_count == 1) {
      // Single press action
      button_state = single_press;
      // Activate menu
    } else if (press_count == 2) {
      // Double press action
      button_state = double_press;
      // Move left
    } else if (press_count >= 3) {
      // Triple press action
      button_state = triple_press;
      // Move right
    }
    press_count = 0; // Reset press count after action is taken
  }

  down_last_time = down_now;
}

bool Button::isButtonDown() {
  // #ifdef DEBUG_OUTPUT
  //   Serial.print("Button::isButtonDown! pin: ");Serial.print(bpin);Serial.print("; DigitalRead: ");Serial.print(digitalRead(bpin));Serial.print("; Active_State: ");Serial.println(active_state);
  // #endif
  return digitalRead(bpin) == active_state;
}

const String Button::state_str[Button::num_states] = { 
    "idle", 
    "down_edge", 
    "down", 
    "down_long_edge", 
    "down_long", 
    "single_press", 
    "double_press", 
    "triple_press", 
    "up_edge", 
    "up_long_edge"
  };

//--------------------------------------------

#ifdef ONE_BUTTON_ONLY_MENU
//One Button in Buttons only
void Buttons::begin() {
  mode.begin();
}

void Buttons::loop() {
  mode.loop();
}

bool Buttons::stateChanged() {
  return mode.stateChanged();
}

#endif

#ifndef ONE_BUTTON_ONLY_MENU
//Buttons
void Buttons::begin() { 
  left.begin();
  mode.begin();
  right.begin();
  power.begin();
}

void Buttons::loop() {
  left.loop();
  mode.loop();
  right.loop();
  power.loop();
}

bool Buttons::stateChanged() {
  return 
    left.stateChanged() ||
    mode.stateChanged() ||
    right.stateChanged() ||
    power.stateChanged();
}
#endif