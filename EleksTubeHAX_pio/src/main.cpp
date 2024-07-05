/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Original location: https://github.com/aly-fly/EleksTubeHAX
 * Hardware: ESP32
 * Based on: https://github.com/SmittyHalibut/EleksTubeHAX
 */

#include <stdint.h>
#include "GLOBAL_DEFINES.h"
#include "ChipSelect.h"
#include "Buttons.h"
#include "Backlights.h"
#include "TFTs.h"
#include "Clock.h"
#include "Menu.h"
#include "StoredConfig.h"
#include "WiFi_WPS.h"
#include "Mqtt_client_ips.h"
#include "TempSensor_inc.h"
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//#include "Gestures.h"
//TODO put into class
#include <Wire.h>
#include <SparkFun_APDS9960.h>
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// Constants

// Global Variables
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//TODO put into class
SparkFun_APDS9960 apds      = SparkFun_APDS9960();
//interupt signal for gesture sensor
int volatile      isr_flag  = 0;
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

Backlights    backlights;
Buttons       buttons;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Clock         uclock;
Menu          menu;
StoredConfig  stored_config;
TFTs          tfts;

bool          FullHour        = false;
uint8_t       hour_old        = 255;
bool          DstNeedsUpdate  = false;
uint8_t       yesterday       = 0;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu(void);
void checkOnEveryFullHour(bool loopUpdate=false);
void updateDstEveryNight(void);
void drawMenu();
void handlePowerSwitchPressed();
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void gestureStart();
void handleGestureInterupt(void); //only for NovelLife SE
void gestureInterruptRoutine(void); //only for NovelLife SE
void HandleGesture(void); //only for NovelLife SE
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void setup() {
  Serial.begin(115200);
  delay(4000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("In setup().");

  stored_config.begin();
  stored_config.load();

  backlights.begin(&stored_config.config.backlights);
  buttons.begin();
  menu.begin();

  // Setup the displays (TFTs) initaly and show bootup message(s)
  tfts.begin();  // and count number of clock faces available
  tfts.fillScreen(TFT_SKYBLUE);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 2);  // Font 2. 18 pixel high (needs to be definded for loading)
  tfts.println("setup...");

#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  //Init the Gesture sensor
  tfts.println("Gesture sensor start");Serial.println("Gesture sensor start");
  gestureStart(); //TODO put into class
#endif

  // Setup WiFi connection. Must be done before setting up Clock.
  // This is done outside Clock so the network can be used for other things.
  tfts.println("WiFi start");Serial.println("WiFi start");
  WifiBegin();
  
  // wait for a bit before querying NTP
  for (uint8_t ndx=0; ndx < 5; ndx++) {
    tfts.print(">");
    delay(100);
  }
  tfts.println("");

  // Setup the clock.  It needs WiFi to be established already.
  tfts.println("Clock start");Serial.println("Clock start");
  uclock.begin(&stored_config.config.uclock);
  #ifdef DEBUG_OUTPUT
    Serial.print("Blank hours zero: ");
    Serial.println(uclock.getBlankHoursZero());
    Serial.print("Twelve hour: ");
    Serial.println(uclock.getTwelveHour());
    Serial.print("Timezone offset: ");
    Serial.println(uclock.getTimeZoneOffset());
  #endif

  // Setup MQTT
  tfts.println("MQTT start");Serial.println("MQTT start");
  MqttStart();

#ifdef GEOLOCATION_ENABLED
  tfts.println("Use internet based geo locaction query to get the actual timezone to be used!");
  if (GetGeoLocationTimeZoneOffset()) {
    tfts.print("TZ: ");
    tfts.println(GeoLocTZoffset);
    uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
    Serial.print("Saving config, triggered by timezone change...");
    stored_config.save();
    Serial.println(" Done.");
  } else {
    Serial.println("Geolocation failed.");
    tfts.println("Geo FAILED");
  }
#else
  //Serial.print("Stored zimezone offset is: ");Serial.println((uclock.getTimeZoneOffset()/3600));

  //Hack to set the timezone offset to a custom value
  Serial.println("HACK!!! Always use custom timezone offset defined in the CODE!");
  GeoLocTZoffset = strtod(CUSTOM_TIMEZONE_OFFSET, NULL);
  Serial.print("Used custom timezone offset: ");Serial.println(GeoLocTZoffset);
  //set as seconds (times 3600 to get seconds from hours)
  uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);

  Serial.print("Saving config, triggered by timezone change...");
  stored_config.save();
  Serial.println(" Done.");
#endif

  if (uclock.getActiveGraphicIdx() > tfts.NumberOfClockFaces) {
    uclock.setActiveGraphicIdx(tfts.NumberOfClockFaces);
    Serial.println("Last selected index of clock face is larger than currently available number of image sets.");
  }
  
  tfts.current_graphic = uclock.getActiveGraphicIdx();
  #ifdef DEBUG_OUTPUT
    Serial.print("Current active graphic index: ");
    Serial.println(tfts.current_graphic);
  #endif

  menu.setTFTsInstance(&tfts);
  menu.setBacklightsInstance(&backlights);
  menu.setConfigInstance(&stored_config);

  tfts.println("Done with initializing setup!");
  Serial.println("Done with initializing setup!");

  // Leave boot up messages on screen for a few seconds.
  // 0.2 s times 10 = 2s, each loop prints a > character
  for (uint8_t ndx=0; ndx < 10; ndx++) {
    tfts.print(">");
    delay(200);
  }

  // Start up the clock displays.
  tfts.fillScreen(TFT_BLACK);
  uclock.loop();
  updateClockDisplay(TFTs::force);

  Serial.println("Setup finished!");
}

void loop() {
  uint32_t millis_at_top = millis();

  // Do all the maintenance work
  WifiReconnect(); // if not connected attempt to reconnect

  MqttStatusPower = tfts.isEnabled();
  MqttStatusState = (uclock.getActiveGraphicIdx()+1) * 5; // ??? Why times five?
  MqttLoopFrequently();
  if (MqttCommandPowerReceived) {
    MqttCommandPowerReceived = false;
    if (MqttCommandPower) {
#ifndef HARDWARE_SI_HAI_CLOCK
      if (!tfts.isEnabled()) {
        tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
        updateClockDisplay(TFTs::force);
      }
#endif
      tfts.enableAllDisplays();
      backlights.PowerOn();
    } else {
      tfts.disableAllDisplays();
      backlights.PowerOff();
    }
  }

  if (MqttCommandStateReceived) {
    MqttCommandStateReceived = false;
    randomSeed(millis());
    uint8_t idx;
    if (MqttCommandState >= 90) { 
      idx = random(1, tfts.NumberOfClockFaces+1); 
    } else { 
      idx = (MqttCommandState / 5) -1; // 10..40 -> graphic 1..6
    } 
    Serial.print("Graphic change request from MQTT; command: ");Serial.print(MqttCommandState);Serial.print("; index: ");Serial.println(idx);
    uclock.setClockGraphicsIdx(idx);
    tfts.current_graphic = uclock.getActiveGraphicIdx();
    updateClockDisplay(TFTs::force);   // redraw everything        
  }

  buttons.loop(); // Sets the states of the buttons, by the detected button presses, releases and gives the time of the press 

  #ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  handleGestureInterupt();
  #endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  handlePowerSwitchPressed();
 
  menu.loop(buttons);     // Must be called after buttons.loop() - Sets the states of the menu, by the detected button presses
  backlights.loop();
  uclock.loop();          // Read the time values from RTC, if needed

  checkOnEveryFullHour(true);    // Check, if dimming is needed, if actual time is in the timeslot for the Nighttime.
  updateClockDisplay();   // Update the clock face. Get actual time from RTC and set the LCDs.
  updateDstEveryNight();  // Check for Daylight-Saving-Time (Summertime) once a day

  drawMenu();             // Draw the menu on the clock face if needed

// End of normal loop
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Loop time management

  uint32_t time_in_loop = millis() - millis_at_top;
  if (time_in_loop < 20) {
    // we have free time, spend it for loading next image into buffer
    tfts.LoadNextImage();
    // we still have extra time - do "usefull" things in the loop
    time_in_loop = millis() - millis_at_top;
    if (time_in_loop < 20) {
      MqttLoopInFreeTime();
      PeriodicReadTemperature();
      if (bTemperatureUpdated) {
        #ifdef DEBUG_OUTPUT
          Serial.println("Temperature updated!");
        #endif
        tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);  // show latest clock digit and temperature readout together
        bTemperatureUpdated = false;
      }
      // run once a day (= 744 times per month which is below the limit of 5k for free account)
      if (DstNeedsUpdate) { // Daylight savings time changes at 3 in the morning
        if (GetGeoLocationTimeZoneOffset()) {
          #ifdef DEBUG_OUTPUT
            Serial.print("Set TimeZone offset once per hour: ");Serial.println(GeoLocTZoffset);
          #endif
          uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
          DstNeedsUpdate = false;  // done for this night; retry if not sucessfull
        }
      }
      // Sleep for up to 20ms, less if we've spent time doing stuff above.
      time_in_loop = millis() - millis_at_top;
      if (time_in_loop < 20) {
        delay(20 - time_in_loop);
      }
    }
  }
#ifdef DEBUG_OUTPUT
  if (time_in_loop <= 1) Serial.print(".");
  else {
    Serial.print("time spent in loop (ms): ");Serial.println(time_in_loop);
  }
#endif
}

#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void gestureStart()
{
    // for gesture sensor APDS9660 - Set interrupt pin on ESP32 as input
  pinMode(GESTURE_SENSOR_INPUT_PIN, INPUT);

  // Initialize interrupt service routine for interupt from APDS-9960 sensor
  attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), gestureInterruptRoutine, FALLING);

  // Initialize gesture sensor APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));

    //Set Gain to 1x, bacause the cheap chinese fake APDS sensor can't handle more (also remember to extend ID check in Sparkfun libary to 0x3B!)
    apds.setGestureGain(GGAIN_1X);
          
    // Start running the APDS-9960 gesture sensor engine
    if ( apds.enableGestureSensor(true) ) {
      Serial.println(F("Gesture sensor is now running"));
    } else {
      Serial.println(F("Something went wrong during gesture sensor enablimg in the APDS-9960 library!"));
    }
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
}

//Handle Interrupt from gesture sensor and simulate a short button press (state down_edge) of the corresponding button, if a gesture is detected 
void handleGestureInterupt()
{
  if( isr_flag == 1 ) {
    detachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN));
    HandleGesture();
    isr_flag = 0;
    attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), gestureInterruptRoutine, FALLING);
  }
  return;
}

//mark, that the Interrupt of the gesture sensor was signaled
void gestureInterruptRoutine() {
  isr_flag = 1;
  return;
}

//check which gesture was detected
void HandleGesture() { 
    //Serial.println("->main::HandleGesture()");
    if ( apds.isGestureAvailable() ) {
      Menu::states menu_state = Menu::idle;
      switch ( apds.readGesture() ) {
        case DIR_UP:
          Serial.println("Gesture detected! LEFT");
          menu_state = menu.getState();
          if (menu_state == Menu::idle) { //not in the menu, so set the clock face instead
            Serial.println("Adjust Clock Graphics down 1");
            uclock.adjustClockGraphicsIdx(-1);
            if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
              tfts.current_graphic = uclock.getActiveGraphicIdx();
            updateClockDisplay(TFTs::force);   // redraw everything
            }
          }
          else {
            buttons.left.setDownEdgeState(); // in the menu, so "press" the left button
          }
          break;
        case DIR_DOWN:
          Serial.println("Gesture detected! RIGHT");
          menu_state = menu.getState();
          Serial.println(menu_state);
          if (menu_state == Menu::idle) { //not in the menu, so set the clock face instead
            Serial.println("Adjust Clock Graphics up 1");
            uclock.adjustClockGraphicsIdx(1);
            if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
              tfts.current_graphic = uclock.getActiveGraphicIdx();
            updateClockDisplay(TFTs::force);   // redraw everything
            }
          }
          else {
            buttons.right.setDownEdgeState(); // in the menu, so "press" the right button
          }
          break;
        case DIR_LEFT:
          buttons.power.setDownEdgeState();
          Serial.println("Gesture detected! DOWN");
          break;
        case DIR_RIGHT:
          buttons.mode.setDownEdgeState();
          Serial.println("Gesture detected! UP");
          break;
        case DIR_NEAR:
          buttons.mode.setDownEdgeState();
          Serial.println("Gesture detected! NEAR");
          break;
        case DIR_FAR:
          buttons.power.setDownEdgeState();
          Serial.println("Gesture detected! FAR");
          break;
        default:        
          Serial.println("Movement detected but NO gesture detected!");
      } //switch apds.readGesture()
    } //if apds.isGestureAvailable()
  return;
}
#endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void setupMenu() {
  #ifdef DEBUG_OUTPUT
    Serial.println("main::setupMenu!");
  #endif  
  tfts.chip_select.setHoursTens();
  tfts.setTextColor(TFT_WHITE, TFT_BLACK, true);
  tfts.fillRect(0, 120, 135, 240, TFT_BLACK);
  tfts.setCursor(0, 124, 4);  // Font 4. 26 pixel high
}

bool isNightTime(uint8_t current_hour) {
    if (DAY_TIME < NIGHT_TIME) {
      // "Night" spans across midnight
      return (current_hour < DAY_TIME) || (current_hour >= NIGHT_TIME);
    }
    else {
      // "Night" starts after midnight, entirely contained within the day
      return (current_hour >= NIGHT_TIME) && (current_hour < DAY_TIME);  
    }
}

void checkOnEveryFullHour(bool loopUpdate) {
  // dim the clock at night
  uint8_t current_hour = uclock.getHour24();
  FullHour = current_hour != hour_old;
  if (FullHour) {
  Serial.print("current hour = ");
  Serial.println(current_hour);
    if (isNightTime(current_hour)) {
      Serial.println("Setting night mode (dimmed)");
      tfts.dimming = TFT_DIMMED_INTENSITY;
      tfts.InvalidateImageInBuffer(); // invalidate; reload images with new dimming value
      backlights.dimming = true;
      if (menu.getState() == Menu::idle || !loopUpdate) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    } else {
      Serial.println("Setting daytime mode (normal brightness)");
      tfts.dimming = 255; // 0..255
      tfts.InvalidateImageInBuffer(); // invalidate; reload images with new dimming value
      backlights.dimming = false;
      if (menu.getState() == Menu::idle || !loopUpdate) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    }
    hour_old = current_hour;
  }
}

//check Daylight-Saving-Time (Summertime)
void updateDstEveryNight() {
  uint8_t currentDay = uclock.getDay();
  // This `DstNeedsUpdate` is True between 3:00:05 and 3:00:59. Has almost one minute of time slot to fetch updates, incl. eventual retries.
  DstNeedsUpdate = (currentDay != yesterday) && (uclock.getHour24() == 3) && (uclock.getMinute() == 0) && (uclock.getSecond() > 5);
  if (DstNeedsUpdate) {
    Serial.print("DST needs update...");
    // Update day after geoloc was sucesfully updated. Otherwise this will immediatelly disable the failed update retry.
    yesterday = currentDay;
  }
}

void updateClockDisplay(TFTs::show_t show) {
  #ifdef DEBUG_OUTPUT_VERBOSE
    Serial.println("main::updateClockDisplay!");
  #endif
  // refresh starting on seconds
  tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
  tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
}

void drawMenu() { 
  // Begin Draw Menu
  if (menu.stateChanged() && tfts.isEnabled()) {
    Menu::states menu_state = menu.getState();
    int8_t menu_change = menu.getChange();

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

// "Power" button pressed, do something
void handlePowerSwitchPressed() {  
#ifndef ONE_BUTTON_ONLY_MENU
  // Power button pressed: If in menu, exit menu. Else turn off displays and backlight.
  if (buttons.power.isDownEdge() && (menu.getState() == Menu::idle)) {
    #ifdef DEBUG_OUTPUT
      Serial.println("Power button pressed.");
    #endif
    tfts.chip_select.setAll();
    tfts.fillScreen(TFT_BLACK);
    tfts.toggleAllDisplays();
    if (tfts.isEnabled()) {
    #ifndef HARDWARE_SI_HAI_CLOCK
      tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
    #endif
      tfts.chip_select.setAll();
      tfts.fillScreen(TFT_BLACK);
      updateClockDisplay(TFTs::force);
    }
    backlights.togglePower();
  }
  #endif
}
