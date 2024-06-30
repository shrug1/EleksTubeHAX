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
  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    pinMode(CSSR_LATCH_PIN, OUTPUT);
    pinMode(CSSR_DATA_PIN, OUTPUT);
    pinMode(CSSR_CLOCK_PIN, OUTPUT);

    digitalWrite(CSSR_DATA_PIN, LOW);
    digitalWrite(CSSR_CLOCK_PIN, LOW);
    digitalWrite(CSSR_LATCH_PIN, LOW);
    update();
  #else
    // Initialize each LCD enable pin as OUTPUT and set it to HIGH (disabled)
    for (int i = 0; i < numLCDs; ++i) {
      pinMode(lcdEnablePins[i], OUTPUT);
      digitalWrite(lcdEnablePins[i], HIGH);
    }
  #endif
}

void ChipSelect::clear(bool update_) {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::clear!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  setDigitMap(all_off, update_);
#else
  disableAllCSPinsH401();
#endif
}

void ChipSelect::setAll(bool update_) {
#ifdef DEBUG_OUTPUT 
  Serial.println("ChipSelect::setAll!");
#endif
#ifndef HARDWARE_IPSTUBE_H401_CLOCK
  setDigitMap(all_on,  update_);
#else
  enableAllCSPinsH401();
#endif
}

void ChipSelect::setDigit(uint8_t digit, bool update_) {
#ifdef DEBUG_OUTPUT
  Serial.println("ChipSelect::setDigit!");
#endif
  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    // Set the bit for the given digit in the digits_map
    setDigitMap(1 << digit, update_);
  #else
    // Set the bit for the given digit in the digits_map
    currentLCD = digit; 
    if (update_) update();
  #endif
}

void ChipSelect::update() {
  #ifdef DEBUG_OUTPUT
    Serial.println("ChipSelect::update!");
  #endif

  #ifndef HARDWARE_IPSTUBE_H401_CLOCK
    // Documented in README.md.  Q7 and Q6 are unused. Q5 is Seconds Ones, Q0 is Hours Tens.
    // Q7 is the first bit written, Q0 is the last.  So we push two dummy bits, then start with
    // Seconds Ones and end with Hours Tens.
    // CS is Active Low, but digits_map is 1 for enable, 0 for disable.  So we bit-wise NOT first.

    uint8_t to_shift = (~digits_map) << 2;

    digitalWrite(CSSR_LATCH_PIN, LOW);
    shiftOut(CSSR_DATA_PIN, CSSR_CLOCK_PIN, LSBFIRST, to_shift);
    digitalWrite(CSSR_LATCH_PIN, HIGH);
  #else
    // Example logic to enable/disable each LCD
    // for (int i = 0; i < numLCDs; ++i) {
    //   // Determine if the current LCD should be enabled or disabled
    //   // This is where you'd add your logic, for now, we'll just cycle through them
    //   bool enable = (i == currentLCD); // Example condition, replace with your logic

      // Set the pin LOW (enable) or HIGH (disable) based on the condition
      //Always disable all other LCDs (not the current one)
      digitalWrite(lcdEnablePins[currentLCD], LOW);
      delay(100);
      //deactivate again
      digitalWrite(lcdEnablePins[currentLCD], HIGH);
    //}
  #endif
}

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

