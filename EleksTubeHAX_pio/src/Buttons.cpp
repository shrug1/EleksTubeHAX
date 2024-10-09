#include "Buttons.h"

void Button::begin() {
  millis_at_last_transition = millis();
  Serial.print("BUTTON begin: millis_at_last_transition: Set to");Serial.println(millis_at_last_transition);
  millis_at_last_loop = millis_at_last_transition;

#ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("Init button: "); Serial.println(bpin);
#endif
  pinMode(bpin, INPUT);
  //check if the button is pressed
  down_last_time = isButtonDown();
  //set the member variable down_last_time to down_edge, because the button is pressed while initializing
  if (down_last_time) {
#ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("BUTTON begin: millis_at_last_loop: ");Serial.println(millis_at_last_loop);
    Serial.print("BUTTON begin: millis_at_last_transition: ");Serial.println(millis_at_last_transition);
    Serial.print("BUTTON begin: millis_at_last_loop - millis_at_last_transition: ");Serial.println((millis_at_last_loop - millis_at_last_transition));
    Serial.println("BUTTON begin: button_state was down last loop. Change button_state to down_edge.");
#endif
    button_state = down_edge;
  } else {
#ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("BUTTON begin: millis_at_last_loop: ");Serial.println(millis_at_last_loop);
    Serial.print("BUTTON begin: millis_at_last_transition: ");Serial.println(millis_at_last_transition);
    Serial.print("BUTTON begin: millis_at_last_loop - millis_at_last_transition: ");Serial.println((millis_at_last_loop - millis_at_last_transition));
    Serial.println("BUTTON begin: button_state was not down last loop. Change button_state to idle.");
#endif
    //if not pressed, set the member variable down_last_time to idle, because the button is not pressed while initializing
    button_state = idle;
  }
}

void Button::loop() {
  // Get the current tick count (starting tick) in milliseconds and store it in a member variable (for the next loop, to do non-blocking things in an interval)
  millis_at_last_loop = millis();
  // Check every loop if the button is pressed or not (digital read from pin for button) and store it in the variable down_now (true or false)
  bool down_now = isButtonDown();
  
  #ifdef DEBUG_OUTPUT_BUTTONS
  if (down_now) {
    Serial.println("-----------------------------------------");
    Serial.print("BUTTON: Actual Button pressed down is: ");
    Serial.println(bpin);
  }
  #endif

  // Debounce logic
  if (down_now != down_last_time) {
    // Reset the debounce timer
    last_debounce_time = millis_at_last_loop;
  }

  if ((millis_at_last_loop - last_debounce_time) > debounce_delay) {
    // Only update the button state if the debounce delay has passed
    if (down_now != down_last_time) {
      down_last_time = down_now;

      if (down_now) {
        // Button was just pressed
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just Pressed!");
        #endif
        button_state = down_edge;
        millis_at_last_transition = millis_at_last_loop;
        millis_at_last_press = millis_at_last_loop;
      } else {
        // Button was just released
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just Released!");
        #endif
        millis_at_last_release = millis_at_last_loop;
        millis_at_last_transition = millis_at_last_loop;
      }
    }
  }

  // Set the previous state from the member variable button_state
  state previous_state = button_state;
  
  // Set the button state, based on the current state and the previous state

  // Check if the button was NOT pressed while in the last loop and is also NOT pressed now
  if (!down_last_time && !down_now) {
    // Set the member button state to "idle"
    button_state = idle;
    // Check if we have a single click pending and the time between the last loop and the last press is greater than the double click wait time
    if (single_click_pending && (millis_at_last_loop - millis_at_last_press > double_click_ms)) {
      #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: single_click_pending was true, but now it is SINGLE_CLICK!");
      #endif
      // If yes, we have a single click
      button_state = single_click;      
      single_click_pending = false;
    }
  }
  // Check if the button was NOT pressed while in the last loop and IS pressed now
  else if (!down_last_time && down_now) {
    // Just pressed
    #ifdef DEBUG_OUTPUT_BUTTONS
    Serial.println("BUTTON: Just Pressed!");
    #endif
    // Set the member button state to "down_edge"
    button_state = down_edge;
    // Store the starting tick count in milliseconds in the member variable millis_at_last_transition
    millis_at_last_transition = millis_at_last_loop;
    millis_at_last_press = millis_at_last_loop;
    #ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("BUTTON: millis_at_last_transition is now: "); Serial.println(millis_at_last_transition);
    Serial.print("BUTTON: millis_at_last_press is now: "); Serial.println(millis_at_last_press);
    #endif
  }
  // Check if the button WAS pressed while in the last loop and IS also pressed now
  else if (down_last_time && down_now) {
    // Been pressed. For how long?
    #ifdef DEBUG_OUTPUT_BUTTONS 
    Serial.println("BUTTON: Been pressed. For how long?");
    Serial.print("BUTTON: millis_at_last_loop: "); Serial.println(millis_at_last_loop);
    Serial.print("BUTTON: millis_at_last_transition: "); Serial.println(millis_at_last_transition);
    Serial.print("BUTTON: millis_at_last_loop - millis_at_last_transition: "); Serial.println((millis_at_last_loop - millis_at_last_transition));
    Serial.print("BUTTON: long_press_ms: "); Serial.println(long_press_ms);
    #endif
    // Check if the time between the last transition and the starting tick count is greater or equal to the long press time
    if (millis_at_last_loop - millis_at_last_transition >= long_press_ms) {
      // Long pressed. Did we just transition?
      #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Long pressed. Did we just transition?");
      #endif
      // Check if the previous state was "down_long_edge" or "down_long"
      // If yes, set the member variable button_state to "down_long"
      if (previous_state == down_long_edge || previous_state == down_long) {
        // No, we already detected the edge.
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: No, we already detected the edge in last loop.");
        #endif
        button_state = down_long;
      }
      // If no, set the member button state to "down_long_edge"
      else {
        // Previous state was something else, so this is the transition.
        // down -> down_long_edge does NOT update millis_at_last_transition.
        // We'd rather know how long it's been down than been down_long.
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: button_state was not down_long_edge or down_long in the last loop! Set button_state to down_long_edge now.");
        #endif
        button_state = down_long_edge;
        millis_at_last_transition = millis_at_last_loop; // Update transition time
      }
    } else {
      // Not yet long pressed
      #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.print("BUTTON: millis_at_last_transition is now: "); Serial.println(millis_at_last_transition);
      Serial.print("BUTTON: millis_at_last_press is now: "); Serial.println(millis_at_last_press);        
      Serial.println("BUTTON: Not yet long pressed! Set/Keep button_stage to down.");
      #endif
      button_state = down;
    }
  }
  // Check if the button WAS pressed while in the last loop and is NOT pressed now
  // So the button was released just now
  else if (down_last_time && !down_now) {
    // Just released. From how long?
    #ifdef DEBUG_OUTPUT_BUTTONS
    Serial.println("BUTTON: Just released. From how long?");
    #endif
    // Check if the previous state was "down_long_edge" or "down_long"
    // So if the button was long pressed
    if (previous_state == down_long_edge || previous_state == down_long) {
      #ifdef DEBUG_OUTPUT_BUTTONS
      Serial.println("BUTTON: Just released from a long press.");
      #endif
      // Just released from a long press.
      button_state = long_click; // Set long_click state
    } else {
      // Check if the time between the last release and the starting tick count is less or equal to the double click time
      // So the button was pressed and released in a short time twice
      // This is a double click
      if (millis_at_last_loop - millis_at_last_release <= double_click_ms) {
        // Double click.
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Double Click.");
        Serial.print("BUTTON: millis_at_last_loop: "); Serial.println(millis_at_last_loop);
        Serial.print("BUTTON: millis_at_last_release: "); Serial.println(millis_at_last_release);
        Serial.print("BUTTON: millis_at_last_loop - millis_at_last_release: "); Serial.println((millis_at_last_loop - millis_at_last_release));
        Serial.print("BUTTON: double_click_ms: "); Serial.println(double_click_ms);
        #endif
        button_state = double_click;
        single_click_pending = false;
      } else {
        // Just released from a short press. Pending single click!
        #ifdef DEBUG_OUTPUT_BUTTONS
        Serial.println("BUTTON: Just released from a short press. Pending single click!");
        Serial.print("BUTTON: millis_at_last_loop: "); Serial.println(millis_at_last_loop);
        Serial.print("BUTTON: millis_at_last_release: "); Serial.println(millis_at_last_release);
        Serial.print("BUTTON: millis_at_last_loop - millis_at_last_release: "); Serial.println((millis_at_last_loop - millis_at_last_release));
        Serial.print("BUTTON: double_click_ms: "); Serial.println(double_click_ms);
        #endif
        single_click_pending = true;
      }
    }
    millis_at_last_release = millis_at_last_loop;
    millis_at_last_transition = millis_at_last_loop;
  }

  // Check if the previous state is NOT equal to the current state
  // This means that the button state has changed in this loop
  // So set the member variable state_changed to true
  state_changed = previous_state != button_state;
  // Store the current down state in the member variable down_last_time (true or false)
  down_last_time = down_now;

  #ifdef DEBUG_OUTPUT_BUTTONS
  if (state_changed) {
    Serial.print("BUTTON: End of loop! State changed from "); Serial.print(state_str[previous_state]); Serial.print(" to "); Serial.println(state_str[button_state]);
    Serial.println("BUTTON: ----------------------------------------------");
  }
  #endif
}

void Button::resetState() {
  #ifdef DEBUG_OUTPUT_BUTTONS
    Serial.print("BUTTON: Reset state of button: "); Serial.println(bpin);
  #endif
  button_state = idle;
  single_click_pending = false;
  state_changed = false;
  down_last_time = false;
  millis_at_last_press = 0;
  millis_at_last_release = 0;
  millis_at_last_transition = 0;
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
    "double_click",   
    "long_click"
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