#ifndef CHIP_SELECT_H
#define CHIP_SELECT_H

#include "GLOBAL_DEFINES.h"

/*
 * `digit`s are as defined in Hardware.h, 0 == seconds ones, 5 == hours tens.
 */

class ChipSelect {
public:
  ChipSelect() {};

  void begin();
  void update();

  int currentLCD = 0;

  // These speak the indexes defined in Hardware.h.
  // So 0 is disabled, 1 is enabled (even though CS is active low, this gets mapped.)
  // So bit 0 (LSB), is index 0, is SECONDS_ONES
  // Translation to what the 74HC595 uses is done in update()
  void setDigitMap(uint8_t map, bool update_=true)   { if (update_) update(); }//digits_map = map;  }
  uint8_t getDigitMap()                        { return digits_map; }

  // Helper functions
  // Sets just the one digit by digit number
  void setDigit(uint8_t digit, bool update_=true);
  void enableDigitCSPinsH401(uint8_t digit);
  void disableDigitCSPinsH401(uint8_t digit);
  
  void setAll(bool update_=true);
  void clear(bool update_=true);
  void setSecondsOnes()                           { setDigit(SECONDS_ONES); }
  void setSecondsTens()                           { setDigit(SECONDS_TENS); }
  void setMinutesOnes()                           { setDigit(MINUTES_ONES); }
  void setMinutesTens()                           { setDigit(MINUTES_TENS); }
  void setHoursOnes()                             { setDigit(HOURS_ONES); }
  void setHoursTens()                             { setDigit(HOURS_TENS); }
  // bool isSecondsOnes()                            { return (digits_map&SECONDS_ONES_MAP > 0); }
  // bool isSecondsTens()                            { return (digits_map&SECONDS_TENS_MAP > 0); }
  // bool isMinutesOnes()                            { return (digits_map&MINUTES_ONES_MAP > 0); }
  // bool isMinutesTens()                            { return (digits_map&MINUTES_TENS_MAP > 0); }
  // bool isHoursOnes()                              { return (digits_map&HOURS_ONES_MAP > 0); }
  // bool isHoursTens()                              { return (digits_map&HOURS_TENS_MAP > 0); }

  void enableAllCSPinsH401();
  void disableAllCSPinsH401();
private:
  uint8_t digits_map;
  const uint8_t all_on = 0x3F;
  const uint8_t all_off = 0x00;
};


#endif // CHIP_SELECT_H
