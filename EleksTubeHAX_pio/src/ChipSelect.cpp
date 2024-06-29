#include "ChipSelect.h"

int CSPins[] = {TFT_CS_1,TFT_CS_2,TFT_CS_3,TFT_CS_4,TFT_CS_5,TFT_CS_6};
const int numLCDs = sizeof(CSPins) / sizeof(CSPins[0]);

//const int lcdEnablePins[] = {GPIO_NUM_15,GPIO_NUM_2,GPIO_NUM_27,GPIO_NUM_12,GPIO_NUM_14,GPIO_NUM_13};
//const int numLCDs = sizeof(lcdEnablePins) / sizeof(lcdEnablePins[0]);

void ChipSelect::begin() {
#ifdef DEBUG_OUTPUT_TFT
  Serial.println("ChipSelect::begin!");  
#endif

  // Initialize each LCD CS pins as OUTPUT and set it to HIGH (disabled)
  Serial.println("Initialize each LCD CS pins as OUTPUT and set it to HIGH (disabled)");
  Serial.print("Number of LCDs: "); Serial.println(numLCDs);
  for (int i = 0; i < numLCDs; ++i) {
    pinMode(CSPins[i], OUTPUT);
    digitalWrite(CSPins[i], HIGH);
  }
  //Test to deactivate Backlights
  // pinMode(BACKLIGHTS_PIN, OUTPUT);
  // digitalWrite(BACKLIGHTS_PIN, LOW);
  // pinMode(4, OUTPUT);
  // digitalWrite(4, LOW);
}

void ChipSelect::update() {
#ifdef DEBUG_OUTPUT_TFT
  Serial.println("ChipSelect::update!");
  Serial.print("CurrentLCD: ");Serial.println(currentLCD);
#endif
  // Example logic to enable/disable each LCD
  for (int i = 0; i < numLCDs; ++i) {
#ifdef DEBUG_OUTPUT_TFT
    Serial.print("CS in loop: "); Serial.println(i);
#endif
    // Determine if the current LCD should be enabled or disabled
    // This is where you'd add your logic, for now, we'll just cycle through them
    bool enable = (i == currentLCD); // Example condition, replace with your logic
#ifdef DEBUG_OUTPUT_TFT
    Serial.print("enable: ");Serial.println(enable);    
    Serial.println("Set the pin :");Serial.println(enable ? "LOW" : "HIGH");
#endif
    // Set the pin HIGH (disable) or LOW (enable) based on the condition
    digitalWrite(CSPins[i], enable ? LOW : HIGH);
  }
}
