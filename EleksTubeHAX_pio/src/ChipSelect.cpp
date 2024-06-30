#include "ChipSelect.h"


  //#define TFT_CS        GPIO_NUM_15 //seconds ones
  //#define TFT_CS        GPIO_NUM_2 //seconds tens
  //#define TFT_CS        GPIO_NUM_27 //minutes ones
  //#define TFT_CS        GPIO_NUM_14 //minutes tens
  //#define TFT_CS        GPIO_NUM_12 //hours ones
  //#define TFT_CS         GPIO_NUM_13 //hours tens

// Define the pins for each LCD's enable wire
const int lcdEnablePins[NUM_DIGITS] = {GPIO_NUM_13,GPIO_NUM_12,GPIO_NUM_14,GPIO_NUM_27,GPIO_NUM_2,GPIO_NUM_15};
const int numLCDs = NUM_DIGITS;

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

          // void ChipSelect::update() {
          // #ifdef DEBUG_OUTPUT
          //   Serial.println("ChipSelect::update!");
          //   Serial.print("CurrentLCD: ");
          //   Serial.println(currentLCD);
          // #endif
          //   // Example logic to enable/disable each LCD
          //   for (int i = 0; i < numLCDs; ++i) {
          //     // Determine if the current LCD should be enabled or disabled
          //     // This is where you'd add your logic, for now, we'll just cycle through them
          //     bool enable = (i == currentLCD); // Example condition, replace with your logic

          //     // Set the pin LOW (enable) or HIGH (disable) based on the condition
          //     //Always disable all other LCDs (not the current one)
          //     digitalWrite(lcdEnablePins[i], enable ? LOW : HIGH);
          //     delay(100);
          //     //deactivate again
          //     digitalWrite(lcdEnablePins[i], HIGH);
          //   }  
          // }

void ChipSelect::enableAllCSPinsH401() {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::enableAllCSPinsH401!");
#endif
  // enable each LCD
  for (int i = 0; i < numLCDs; ++i) {
    digitalWrite(lcdEnablePins[i], LOW);
  }
}

void ChipSelect::disableAllCSPinsH401() {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::disableAllCSPinsH401!");
#endif
  // disable each LCD
  for (int i = 0; i < numLCDs; ++i) {
    digitalWrite(lcdEnablePins[i], HIGH);
  }
}

void ChipSelect::enableDigitCSPinsH401(uint8_t digit) {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::enableDigitCSPinsH401!");
#endif
  // enable the LCD for the given digit
  digitalWrite(lcdEnablePins[digit], LOW);
}

void ChipSelect::disableDigitCSPinsH401(uint8_t digit) {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::disableDigitCSPinsH401!");
#endif
  // disable the LCD for the given digit
  digitalWrite(lcdEnablePins[digit], HIGH);
}

