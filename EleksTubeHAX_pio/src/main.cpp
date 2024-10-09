/*
 * Author: Aljaz Ogrin
 * Project: Alternative firmware for EleksTube IPS clock
 * Original location: https://github.com/aly-fly/EleksTubeHAX
 * Hardware: ESP32
 * Based on: https://github.com/SmittyHalibut/EleksTubeHAX
 */

#include <stdint.h>
#include "GLOBAL_DEFINES.h"
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
TFTs          tfts;
Clock         uclock;
Menu          menu;
StoredConfig  stored_config;

bool          FullHour          = false;
uint8_t       hour_old          = 255;
bool          DstNeedsUpdate    = false;
uint8_t       yesterday         = 0;

// Helper function, defined below.
void updateClockDisplay(TFTs::show_t show=TFTs::yes);
void setupMenu(void);
void checkOnEveryFullHour(bool loopUpdate=false);
//for now, switch it off, because I dont want to use the geolocation service
#ifdef GEOLOCATION_ENABLED
void updateDstEveryNight(void);
#endif
void drawMenuAndHandleButtons();
void handlePowerSwitchPressed();
void handleMQTTCommands();
#ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void gestureStart();
void handleGestureInterupt(void); //only for NovelLife SE
void gestureInterruptRoutine(void); //only for NovelLife SE
void handleGesture(void); //only for NovelLife SE
#endif //NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void setup() {
  Serial.begin(115200);
  delay(1000);  // Waiting for serial monitor to catch up.
  Serial.println("");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("In setup().");

  stored_config.begin();
  stored_config.load();

  #ifdef DEBUG_OUTPUT
    Serial.print("Current Dimming Start time after load from config: ");Serial.println(stored_config.config.uclock.dimming_start);
  #endif

  #ifdef DEBUG_OUTPUT
    Serial.print("Current Dimming End time after load from config: ");Serial.println(stored_config.config.uclock.dimming_end);
  #endif

  //check if the dimming times are set in the config, if not, set them to the default values
  if (stored_config.config.uclock.dimming_start == -1) {
    Serial.print("Dimming start time not set in config, setting to default value:");Serial.println(NIGHT_TIME);
    stored_config.config.uclock.dimming_start = NIGHT_TIME;
    stored_config.save();
  }
  if (stored_config.config.uclock.dimming_end == -1) {
    Serial.print("Dimming end time not set in config, setting to default value.");Serial.println(DAY_TIME);
    stored_config.config.uclock.dimming_end = DAY_TIME;
    stored_config.save();
  }

  backlights.begin(&stored_config.config.backlights);
  buttons.begin();
  menu.begin();

  // Setup the displays (TFTs) initaly and show bootup message(s)
  #ifdef DEBUG_OUTPUT_TFT
    Serial.println("Setup TFTs");
  #endif
  tfts.begin();  // and count number of clock faces available
  tfts.fillScreen(TFT_BLACK);
  tfts.setTextColor(TFT_WHITE, TFT_BLACK);
  tfts.setCursor(0, 0, 2);  // Font 2. 16 pixel high
  tfts.println("Setup...");
  #ifdef DEBUG_OUTPUT_TFT
    Serial.println("Finished setup TFTs");
  #endif

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
  #ifdef DEBUG_OUTPUT_VERBOSE
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

  #ifdef DEBUG_OUTPUT_TFT
  Serial.print("Current active graphic index in uclock after init with config load: ");Serial.println(uclock.getActiveGraphicIdx());
  Serial.print("Number of clock faces in tfts after init: ");Serial.println(tfts.NumberOfClockFaces);
  #endif

  //Fallback, if number of clock faces is not counted correctly, set at least to 1 (can happen, if the SPIFFS is not mounted correctly)
  if (tfts.NumberOfClockFaces <= 0) {
    tfts.NumberOfClockFaces = 1;
    Serial.println("Number of clock faces is not counted correctly, set to 1.");
  }

  // Check if the selected clock face is within the available range of clock faces (some clock faces which was existing before, could be deleted now)
  if (uclock.getActiveGraphicIdx() > tfts.NumberOfClockFaces) {
    uclock.setActiveGraphicIdx(tfts.NumberOfClockFaces);
    Serial.println("Last selected index of clock face is larger than currently available number of image sets. Set to last available.");
  }

  // Set actual clock face in the instance of the TFTs class to the selected one from the clock
  tfts.current_graphic = uclock.getActiveGraphicIdx();
  #ifdef DEBUG_OUTPUT_TFT
    Serial.print("Current active graphic index in tfts after correction: ");Serial.println(tfts.current_graphic);
  #endif


  tfts.println("Done with initializing setup!");Serial.println("Done with initializing!");

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

  buttons.loop(); // Sets the states of the buttons, by the detected button presses, releases and gives the time of the press

  handleMQTTCommands(); // Handle MQTT commands, afer the buttons loop, to simulate button presses from MQTT, if needed

  #ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  handleGestureInterupt();
  #endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  handlePowerSwitchPressed();

  menu.loop(buttons);     // Must be called after buttons.loop() - Sets the states of the menu, by the detected button presses
  backlights.loop();
  uclock.loop();          // Read the time values from RTC, if needed

  checkOnEveryFullHour(true);    // Check, if dimming is needed, if actual time is in the timeslot for the night time.
  updateClockDisplay();   // Update the digits of the clock face. Get actual time from RTC and set the LCDs.
//for now, switch it off, because I dont want to use the geolocation service
#ifdef GEOLOCATION_ENABLED
  updateDstEveryNight();  // Check for Daylight-Saving-Time (Summertime) adjustment once a day
#endif

  drawMenuAndHandleButtons();    // Draw the menu on the clock face, if menu is requested and handle the button presses in the menu

// End of normal loop
//------------------------------------------------------------------------------------------------------------------------------------------------------------
// Loop time management + other things to do in "free time"

  uint32_t time_in_loop = millis() - millis_at_top;
  if (time_in_loop < 20) {
    // we have free time, spend it for loading next image into buffer
    tfts.LoadNextImage();

    // we still have extra time - do "usefull" things in the loop
    time_in_loop = millis() - millis_at_top;
    if (time_in_loop < 20) {
      MqttLoopInFreeTime();
#ifdef ONE_WIRE_BUS_PIN
      PeriodicReadTemperature();
      if (bTemperatureUpdated) {
#ifdef DEBUG_OUTPUT
          Serial.println("Temperature updated!");
#endif
        tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), TFTs::force);  // show latest clock digit and temperature readout together
        bTemperatureUpdated = false;
      }
#endif
      
      //Check if we need to update the timezone offset , because daylight saving time is active or vice versa      
      //this work ONLY if the geolocation is enabled, because the timezone offset is set by the geolocation

//for now, switch it off, because I dont want to use the geolocation service
#ifdef GEOLOCATION_ENABLED
      // run once a day (= 744 times per month which is below the limit of 5k for free account)
      if (DstNeedsUpdate) { // Daylight savings time changes at 3 in the morning
        //This will always return false, if the geolocation is not enabled!
        if (GetGeoLocationTimeZoneOffset()) {
#ifdef DEBUG_OUTPUT
            Serial.print("Set TimeZone offset once per hour: ");Serial.println(GeoLocTZoffset);
#endif
          uclock.setTimeZoneOffset(GeoLocTZoffset * 3600);
          DstNeedsUpdate = false;  // done for this night; retry if not sucessfull
        }
        else {
          Serial.println("Geolocation is not enabled or failed!.");
          Serial.println("We are missing the information to update the timezone offset for daylight saving time.");
          Serial.println("SWITCH TIMEZONE MANUALLY!");
        }
      }
#endif
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
#ifdef DEBUG_OUTPUT      
    Serial.println(F("APDS-9960 initialization complete"));
#endif

    //Set Gain to 1x, bacause the cheap chinese fake APDS sensor can't handle more (also remember to extend ID check in Sparkfun libary to 0x3B!)
    apds.setGestureGain(GGAIN_1X);

    // Start running the APDS-9960 gesture sensor engine
    if ( apds.enableGestureSensor(true) ) {
#ifdef DEBUG_OUTPUT      
      Serial.println(F("Gesture sensor is now running"));
#endif
    } else {
#ifdef DEBUG_OUTPUT
      Serial.println(F("Something went wrong during gesture sensor enabling in the APDS-9960 library!"));
#endif
    }
  } else {
#ifdef DEBUG_OUTPUT
    Serial.println(F("Something went wrong during APDS-9960 init!"));
#endif
  }
}

//Handle Interrupt from gesture sensor and simulate a short button press (state down_edge) of the corresponding button, if a gesture is detected
void handleGestureInterupt()
{
  if( isr_flag == 1 ) {
    detachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN));
    handleGesture();
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
void handleGesture() {
#ifdef DEBUG_OUTPUT_VERBOSE
    Serial.println("->main::handleGesture()");
#endif
    if ( apds.isGestureAvailable() ) {
      Menu::states menu_state = Menu::idle;
      switch ( apds.readGesture() ) {
        case DIR_UP:
#ifdef DEBUG_OUTPUT
          Serial.println("Gesture detected! LEFT");
#endif
          menu_state = menu.getState();
          if (menu_state == Menu::idle) { //not in the menu, so set the clock face instead
#ifdef DEBUG_OUTPUT
            Serial.println("Adjust Clock Graphics down 1");
#endif
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
#ifdef DEBUG_OUTPUT
          Serial.println("Gesture detected! RIGHT");
#endif
          menu_state = menu.getState();
#ifdef DEBUG_OUTPUT
          Serial.println(menu_state);
#endif
          if (menu_state == Menu::idle) { //not in the menu, so set the clock face instead
#ifdef DEBUG_OUTPUT
            Serial.println("Adjust Clock Graphics up 1");
#endif
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
#ifdef DEBUG_OUTPUT
          Serial.println("Gesture detected! DOWN");
#endif
          break;
        case DIR_RIGHT:        
          buttons.mode.setDownEdgeState();
#ifdef DEBUG_OUTPUT
          Serial.println("Gesture detected! UP");
#endif
          break;
        case DIR_NEAR:
          buttons.mode.setDownEdgeState();
#ifdef DEBUG_OUTPUT          
          Serial.println("Gesture detected! NEAR");
#endif
          break;
        case DIR_FAR:
          buttons.power.setDownEdgeState();
#ifdef DEBUG_OUTPUT
          Serial.println("Gesture detected! FAR");
#endif
          break;
        default:
#ifdef DEBUG_OUTPUT
          Serial.println("Movement detected but NO gesture detected!");
#endif
      } //switch apds.readGesture()
    } //if apds.isGestureAvailable()
  return;
}
#endif // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

void handleMQTTCommands() {
#ifdef MQTT_ENABLED
  MqttStatusPower = tfts.isEnabled();
  MqttStatusState = (uclock.getActiveGraphicIdx()+1) * 5;
  MqttLoopFrequently();

  //work through the received commands
  //Power Change
  if (MqttCommandPowerReceived) {
    MqttCommandPowerReceived = false;
    if (MqttCommandPower) {
#endif
#if defined(HARDWARE_SI_HAI_CLOCK) && defined(MQTT_ENABLED)
      if (!tfts.isEnabled()) {
        tfts.reinit();  // reinit (original EleksTube HW: after a few hours in OFF state the displays do not wake up properly)
        updateClockDisplay(TFTs::force);
      }
#endif
#ifdef MQTT_ENABLED
      tfts.enableAllDisplays();
      backlights.PowerOn();
    } else {
      tfts.disableAllDisplays();
      backlights.PowerOff();
    }
  }

  //State Change
  if (MqttCommandStateReceived) {
    MqttCommandStateReceived = false;
    randomSeed(millis());            //WHY????
    uint8_t idx;
    //All commands under 100 are graphic change requests now
    if (MqttCommandState < 100) {
      //to enhance the possible selecteable clock faces with the MQTT commands, we index the commands (base 5)
      //I personally have NO IDEA, why this is done this way. We can select ANY topic, with ANY values we like!
      //So I don't get this, lets say, "interesting" way to do this.
      idx = (MqttCommandState / 5) - 1; // e.g. state = 25; 25/5 = 5; 5-1 = 4
      //10 == clock face 1; 15 == clock face 2; 20 == clock face 3; 25 == clock face 4; 30 == clock face 5; 35 == clock face 6; 40 == clock face 7...

      int MaxIdx = tfts.NumberOfClockFaces;
      if (idx > MaxIdx) { idx = 1; }
#ifdef DEBUG_OUTPUT_MQTT
      Serial.print("Clock face change request from MQTT; command: ");Serial.print(MqttCommandState);Serial.print("; so selected index: ");Serial.println(idx);
#endif
      uclock.setClockGraphicsIdx(idx);
      tfts.current_graphic = uclock.getActiveGraphicIdx();
      updateClockDisplay(TFTs::force);   // redraw everything
    } else {
      //button press commands
      if (MqttCommandState >= 100 && MqttCommandState <= 120){
        if (MqttCommandState == 100) {
          #ifdef DEBUG_OUTPUT_MQTT
            Serial.println("MQTT button pressed command received: MODE");
          #endif
          buttons.mode.setUpEdgeState();
        } else
        #ifndef ONE_BUTTON_ONLY_MENU
        if (MqttCommandState == 110) {
#ifdef DEBUG_OUTPUT_MQTT
            Serial.println("MQTT button pressed command received: LEFT");
#endif
          buttons.left.setUpEdgeState();
        } else if (MqttCommandState == 115) {
#ifdef DEBUG_OUTPUT_MQTT
            Serial.println("MQTT button pressed command received: POWER");
#endif
          buttons.power.setUpEdgeState();
        } else if (MqttCommandState == 120) {
#ifdef DEBUG_OUTPUT_MQTT
            Serial.println("MQTT button pressed command received: RIGHT");
#endif
          buttons.right.setUpEdgeState();
        } else {
#ifdef DEBUG_OUTPUT_MQTT
          Serial.print("Unknown MQTT button pressed command received: ");Serial.println(MqttCommandState);
#endif
        }
        #else
        {
#ifdef DEBUG_OUTPUT_MQTT
          Serial.print("Unknown MQTT button pressed command received: ");Serial.println(MqttCommandState);
#endif
        }
        #endif
      } else { //else from button press commands (state 100-120)
#ifdef DEBUG_OUTPUT_MQTT      
        Serial.print("Unknown MQTT command received: ");Serial.println(MqttCommandState);
#endif
      } //end if button press commands
    } //commands under 100
  }
  #endif //MQTT_ENABLED
} //HandleMQTTCommands

void setupMenu() {
  #ifdef DEBUG_OUTPUT_VERBOSE
    Serial.println("main::setupMenu!");
  #endif
  tfts.chip_select.setHoursTens();
  tfts.setTextColor(TFT_WHITE, TFT_BLACK, true);
  tfts.fillRect(0, 120, 135, 240, TFT_BLACK);
  tfts.setCursor(0, 124, 4);  // Font 4. 26 pixel high
}

bool isNightTime(uint8_t current_hour) {
  if (stored_config.config.uclock.dimming_end < stored_config.config.uclock.dimming_start) {
    // "Night" spans across midnight
    return (current_hour < stored_config.config.uclock.dimming_end) || (current_hour >= stored_config.config.uclock.dimming_start);
  }
  else {
    // "Night" starts after midnight, entirely contained within the day
    return (current_hour >= stored_config.config.uclock.dimming_start) && (current_hour < stored_config.config.uclock.dimming_end);
  }
}

void checkOnEveryFullHour(bool loopUpdate) {
  // dim the clock at night
  uint8_t current_hour = uclock.getHour24();
  FullHour = current_hour != hour_old;
  if (FullHour) {
#ifdef DEBUG_OUTPUT
    Serial.println("Check if dimming is needed on every full hour!");
    Serial.print("Current hour = ");
    Serial.println(current_hour);
#endif
    if (isNightTime(current_hour)) {
#ifdef DEBUG_OUTPUT
      Serial.println("Setting night mode (dimmed)");
#endif
      tfts.dimming = TFT_DIMMED_INTENSITY;
      tfts.InvalidateImageInBuffer(); // invalidate; reload images with new dimming value
      backlights.dimming = true;
      if (menu.getState() == Menu::idle || !loopUpdate) { // otherwise erases the menu
        updateClockDisplay(TFTs::force); // update all
      }
    } else {
#ifdef DEBUG_OUTPUT
      Serial.println("Setting daytime mode (normal brightness)");
#endif
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

//for now, switch it off, because I dont want to use the geolocation service
#ifdef GEOLOCATION_ENABLED
//check Daylight-Saving-Time (Summertime)
void updateDstEveryNight() {
  uint8_t currentDay = uclock.getDay();
  // This `DstNeedsUpdate` is True between 3:00:05 and 3:00:59. Has almost one minute of time slot to fetch updates, incl. eventual retries.
  // It is either switching back from 3 am to 2 am (to summer time) or from 2 am to 3 am (to normal/winter time).
  //so check is done EVERY DAY shortly after the time switch, to know, we need to get the correct time
  //Needed, because the time switch is not always on the same day, so we need to check every day, if the time switch is done
  DstNeedsUpdate = (currentDay != yesterday) && (uclock.getHour24() == 3) && (uclock.getMinute() == 0) && (uclock.getSecond() > 5);
  if (DstNeedsUpdate) {
    Serial.println("Daylight-Saving-Time (Summertime) needs update...");
    Serial.println("Will be done in the next 'free time' of the loop()...");
    // Update day after geoloc was sucesfully updated. Otherwise this will immediatelly disable the failed update retry.
    //NOTHING IN HERE!
    //WHAT IS IT GOOD FOR?
    //just set the flag, that the update is needed, but do the update in the loop, if the geolocation was updated successfully
    //set the day to the current day, so we know, we already checked for the update today
    yesterday = currentDay;
  }
}
#endif

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
  //refreshing starting on hours -> Stupid idea
  // tfts.setDigit(HOURS_TENS, uclock.getHoursTens(), show);
  // tfts.setDigit(HOURS_ONES, uclock.getHoursOnes(), show);
  // tfts.setDigit(MINUTES_TENS, uclock.getMinutesTens(), show);
  // tfts.setDigit(MINUTES_ONES, uclock.getMinutesOnes(), show);
  // tfts.setDigit(SECONDS_TENS, uclock.getSecondsTens(), show);
  // tfts.setDigit(SECONDS_ONES, uclock.getSecondsOnes(), show);
}




//It is not only about "drawing" the menu here
//It is also about handling the menu states and the changes to the config/clock settings out of the actual menu states
//So it is a bit more than just drawing the menu
//If the menu was already active and the user choosed to change a value in the actual shown/selected menu, the function to do the change is called from here
//A change is detected by getting the menu.getChange() value from the menu class and then checking if the value is negativ, positive or zero
//Zero means, no change was requested, so nothing to do
//Positive means, the user wants to increase the value, so the belonging function to increase the value is called
//Negative means, the user wants to decrease the value, so the belonging function to decrease the value is called
void drawMenuAndHandleButtons() {
  // Begin Draw Menu
  Menu::states menu_state = menu.getState();
  bool bMenuStateChanged = menu.menuStateChanged();

  if (bMenuStateChanged && tfts.isEnabled()) {
#ifdef DEBUG_OUTPUT_MENU
  Serial.print("MENU: bMenuStateChanged is true! and menu_state is: ");Serial.println(menu_state);
#endif
    int8_t menu_change = menu.getChange();
#ifdef DEBUG_OUTPUT_MENU
    Serial.print("MENU: Value stored in variable menu_change is: ");Serial.println(menu_change);
#endif
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
        tfts.println("+/- Hour");
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
        tfts.println("+/- 15m");
        time_t offset = uclock.getTimeZoneOffset();
        int8_t offset_hour = offset/3600;
        int8_t offset_min = (offset%3600)/60;
        if(offset_min < 0) {
          offset_min = -offset_min;
        }
        tfts.printf("%d:%02d\n", offset_hour, offset_min);
      }
      // dimming start hour
      else if (menu_state == Menu::dimming_begin) {
        if (menu_change != 0) {
          if (menu_change < 0) {
            stored_config.config.uclock.dimming_start--;
            if (stored_config.config.uclock.dimming_start < 0) {
              stored_config.config.uclock.dimming_start = 23;
            }
          } else {
            stored_config.config.uclock.dimming_start++;
            if (stored_config.config.uclock.dimming_start > 23) {
              stored_config.config.uclock.dimming_start = 0;
            }
          }
        }
        setupMenu();
        tfts.println("Dimming");
        tfts.println("start time");
        tfts.println("+/- 1h");
        tfts.printf("%d\n", stored_config.config.uclock.dimming_start);
      }
      // dimming end hour
      else if (menu_state == Menu::dimming_end) {
        if (menu_change != 0) {
          if (menu_change < 0) {
            stored_config.config.uclock.dimming_end--;
            if (stored_config.config.uclock.dimming_end < 0) {
              stored_config.config.uclock.dimming_end = 23;
            }
          } else {
            stored_config.config.uclock.dimming_end++;
            if (stored_config.config.uclock.dimming_end > 23) {
              stored_config.config.uclock.dimming_end = 0;
            }
          }
        }
        setupMenu();
        tfts.println("Dimming");
        tfts.println("end time");
        tfts.println("+/- 1h");
        tfts.printf("%d\n", stored_config.config.uclock.dimming_end);
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
        tfts.println("graphic:");
        tfts.printf("   %d\n", uclock.getActiveGraphicIdx());
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
  //else {
    //We will pass every loop here, if the menu is not active -> we can check, if the button states are changed...
// #ifdef DEBUG_OUTPUT_MENU
//     Serial.println("No change in menu state detected!");
// #endif
//     if(bMenuStateChanged == Menu::idle) {
//       //Only do something, if the menu is not active
// #ifdef DEBUG_OUTPUT_MENU
//       Serial.println("Menu is not active!");
// #endif

//vermutlich BULLSHIT
//       #ifdef ONE_BUTTON_ONLY_MENU
//     // Not in menu, so check for double click or long press
//     if (buttons.mode.isDoubleClick()) {
//       //Switch the clock face index by incrementing it
// //#ifdef DEBUG_OUTPUT
//         Serial.println("Adjust clock graphics index up 1");
// //#endif
//       uclock.adjustClockGraphicsIdx(1);
//       if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
//         tfts.current_graphic = uclock.getActiveGraphicIdx();
//         updateClockDisplay(TFTs::force);   // redraw everything
//       }
//       //buttons.mode.resetState();
//     }
//     else if (buttons.mode.isDownLongy()) {
//       //Switch the clock face index by decrementing it
// //#ifdef DEBUG_OUTPUT
//         Serial.println("Adjust clock graphics index down 1");
// //#endif
//       uclock.adjustClockGraphicsIdx(-1);
//       if(tfts.current_graphic != uclock.getActiveGraphicIdx()) {
//         tfts.current_graphic = uclock.getActiveGraphicIdx();
//         updateClockDisplay(TFTs::force);   // redraw everything
//       }
//       //buttons.mode.resetState();
//     }
// #endif



 //   }
 // }
} //drawMenu

// "Power" button pressed, do something
void handlePowerSwitchPressed() {
#ifndef ONE_BUTTON_ONLY_MENU
  // Power button pressed: If in menu, exit menu. Else turn off displays and backlight.
  if (buttons.power.isDownEdge() && (menu.getState() == Menu::idle)) {
    #ifdef DEBUG_OUTPUT_BUTTONS
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
