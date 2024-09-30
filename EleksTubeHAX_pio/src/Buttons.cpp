#include "Buttons.h"

void Button::begin() {
  millis_at_last_transition = millis();
  millis_at_last_loop = millis_at_last_transition;

#ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("Init button: "); Serial.println(bpin);
#endif
  pinMode(bpin, INPUT);
  //check if the button is pressed
  down_last_time = isButtonDown();
  //set the member variable down_last_time to down_edge, because the button is pressed while initializing
  if (down_last_time) {
    button_state = down_edge;
  } else {
    //if not pressed, set the member variable down_last_time to idle, because the button is not pressed while initializing
    button_state = idle;
  }
}

void Button::loop() {
  //get the current tick count (starting tick) in milliseconds and store it in a member variable (for the next loop, to do non-blocking things in an inteval)
  millis_at_last_loop = millis();
  //Check every loop if the button is pressed or not and store it in the variable down_now (true or false)
  bool down_now = isButtonDown();
 #ifdef DEBUG_OUTPUT_BUTTONS
  if (down_now) { Serial.print("[BUTTON: "); Serial.print(bpin); Serial.println("]"); }
 #endif

  //set the previous state from the member variabel button_state
  state previous_state = button_state;
  
  //Set the button state, based on the current state and the previous state

  //check, if the button was NOT pressed while in the last loop and is also NOT pressed now
  if (down_last_time == false && down_now == false) {
    // set the member button state to "idle"
    button_state = idle;
    //check if we have a single click pending and the time between the last loop and the last press is greater than the double click wait time
    if (single_click_pending && (millis_at_last_loop - millis_at_last_press > double_click_ms)) {
    #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: single_click_pending was true, but now it is SINGLE_CLICK!");
    #endif
      //if yes, we have a single click
      button_state = single_click;      
      single_click_pending = false;
    }
  }
  //check, if the button was NOT pressed while in the last loop and IS pressed now
  else if (down_last_time == false && down_now == true) {
    // Just pressed
    #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Just Pressed!");
    #endif
    //set the member button state to "down_edge"
    button_state = down_edge;
    //store the starting tick count in milliseconds in the member variable millis_at_last_transition
    millis_at_last_transition = millis_at_last_loop;
    millis_at_last_press = millis_at_last_loop;
  }
  //check, if the button WAS pressed while in the last loop and IS also pressed now
  else if (down_last_time == true && down_now == true) {
    // Been pressed. For how long?
    #ifdef DEBUG_OUTPUT_BUTTONS 
      Serial.println("BUTTON: Been pressed. For how long?");
    #endif
    //check, if the time between the last transition and the starting tick count is greater or equal to the long press time
    if (millis_at_last_loop - millis_at_last_transition >= long_press_ms) {
      // Long pressed. Did we just transition?
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Long pressed. Did we just transition?");
      #endif
      //check, if the previous state was "down_long_edge" or "down_long"
      //if yes, set the member variabele button_state to "down_long"
      if (previous_state == down_long_edge || previous_state == down_long) {
        // No, we already detected the edge.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: No, we already detected the edge in last loop.");
        #endif
        button_state = down_long;
      }
      //if no, set the member button state to "down_long_edge"
      else {
        // Previous state was something else, so this is the transition.
        // down -> down_long_edge does NOT update millis_at_last_transition.
        // We'd rather know how long it's been down than been down_long.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: else something! set button_stage to down_long_edge.");
        #endif
        button_state = down_long_edge;
      }
    }  //if (millis_at_last_loop - millis_at_last_transition >= long_press_ms) {
    else {
      // Not yet long pressed
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Not yet long pressed! Set/Keep button_stage to down.");
      #endif
      button_state = down;
    }
  }  //if (down_last_time == true && down_now == true)
  //check, if the button WAS pressed while in the last loop and is NOT pressed now
  //So the button was released just now
  else if (down_last_time == true && down_now == false) {
    // Just released.  From how long?
    #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Just released. From how long?");
    #endif
    //check, if the previous state was "down_long_edge" or "down_long"
    //so if the button was long pressed
    if (previous_state == down_long_edge || previous_state == down_long) {
      #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just released from a long press.");
      #endif
      // Just released from a long press.      
      button_state = long_click; // Set long_click state
    } 
    else {
      //check, if the time between the last release and the starting tick count is less or equal to the double click time
      //so the button was pressed and released in a short time twice
      //this is a double click
      if (millis_at_last_loop - millis_at_last_release <= double_click_ms) {
        // Double click.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: Double Click.");
        #endif
        button_state = double_click;
        single_click_pending = false;
      } else if (millis_at_last_loop - millis_at_last_release <= double_click_ms * 2) {
        // Triple click.
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: Triple Click.");
        #endif
        button_state = triple_click;
        single_click_pending = false;
      } else {
        // Just released from a short press. Pending single click!
        #ifdef DEBUG_OUTPUT_BUTTONS
          Serial.println("BUTTON: Just released from a short press. Pending single click!");
        #endif
        single_click_pending = true;
      }
    }
    millis_at_last_release = millis_at_last_loop;
    millis_at_last_transition = millis_at_last_loop;
  }

  //check, if the previous state is NOT equal to the current state
  //this means, that the button state has changed in this loop
  //so set the member variable state_changed to true
  state_changed = previous_state != button_state;
  #ifdef DEBUG_OUTPUT_BUTTONS
    if (state_changed) {
      Serial.print("BUTTON: State changed from ");
      Serial.print(state_str[previous_state]);
      Serial.print(" to ");
      Serial.println(state_str[button_state]);
    }
  #endif
  //store the current down state in the member variable down_last_time (true or false)
  down_last_time = down_now;
}

const String Button::state_str[Button::num_states] = 
  { "idle", 
    "down_edge", 
    "down", 
    "down_long_edge", 
    "down_long", 
    "up_edge", 
    "up_long_edge",    
    "single_click",
    "double_click"
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