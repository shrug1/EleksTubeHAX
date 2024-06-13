#include "ChipSelect.h"

// Define the pins for each LCD's enable wire
const int lcdEnablePins[] = {GPIO_NUM_15,GPIO_NUM_2,GPIO_NUM_27,GPIO_NUM_12,GPIO_NUM_14,GPIO_NUM_13};
const int numLCDs = sizeof(lcdEnablePins) / sizeof(lcdEnablePins[0]);

void ChipSelect::begin() {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::begin!");  
#endif

  // Initialize each LCD enable pin as OUTPUT and set it to HIGH (disabled)
  for (int i = 0; i < numLCDs; ++i) {
    pinMode(lcdEnablePins[i], OUTPUT);
    digitalWrite(lcdEnablePins[i], HIGH);
  }
}

void ChipSelect::update() {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::update!");
  Serial.print("CurrentLCD: ");
  Serial.println(currentLCD);
#endif
  // Example logic to enable/disable each LCD
  for (int i = 0; i < numLCDs; ++i) {
    // Determine if the current LCD should be enabled or disabled
    // This is where you'd add your logic, for now, we'll just cycle through them
    bool enable = (i == currentLCD); // Example condition, replace with your logic

        // Set the pin HIGH (disable) or LOW (enable) based on the condition
    //digitalWrite(lcdEnablePins[i], enable ? HIGH : LOW);
        //digitalWrite(lcdEnablePins[i], HIGH);
  }

  // Optionally, update currentLCD to cycle through LCDs or implement your logic
  //currentLCD = 0
  //(currentLCD + 1) % numLCDs;
}
