/*
 * Project: Alternative firmware for EleksTube IPS clock
 * Hardware: ESP32
 * File description: User preferences for the complete project
 * Hardware connections and other deep settings are located in "GLOBAL_DEFINES.h"
 */


#ifndef USER_DEFINES_H_
#define USER_DEFINES_H_


//uncomment for Debug logs via serial interface
#define DEBUG_OUTPUT 1
//#define DEBUG_OUTPUT_TFT 1
//#define DEBUG_OUTPUT_VERBOSE 1
//#define DEBUG_OUTPUT_MENU 1
//#define DEBUG_OUTPUT_BUTTONS 1
//#define DEBUG_OUTPUT_MQTT 1


// ************* Type of the clock hardware  *************
#define HARDWARE_Elekstube_CLOCK  // uncomment for the original Elekstube clock
//#define HARDWARE_Elekstube_CLOCK_Gen2  // uncomment for the original Elekstube clock Gen2.1 (ESP32 Pico D4 Chip)
//#define HARDWARE_SI_HAI_CLOCK  // uncomment for the SI HAI copy of the clock
//#define HARDWARE_NovelLife_SE_CLOCK  // uncomment for the NovelLife SE version (Gesture only) - tested and working!; Non-SE version (Buttons only) NOT tested!; Pro version (Buttons and Gesture) NOT tested!
//#define HARDWARE_PunkCyber_CLOCK  // uncomment for the PunkCyber / RGB Glow tube / PCBway clock
//#define HARDWARE_IPSTUBE_H401_CLOCK  // uncomment for the IPSTUBE Model H401 Clock


// ************* Clock font file type selection (.clk or .bmp)  *************
//#define USE_CLK_FILES   // select between .CLK and .BMP images


// ************* Display Dimming / Night time operation *************
#define NIGHT_TIME                  24  // begin of night time - dim brightness of displays after X
#define DAY_TIME                    0   // end of night time - full brightness after X
#define BACKLIGHT_DIMMED_INTENSITY  1   // 0..7
#define TFT_DIMMED_INTENSITY        20  // 0..255

// ************* WiFi config *************
#define WIFI_CONNECT_TIMEOUT_SEC    20
#define WIFI_RETRY_CONNECTION_SEC   15
#define WIFI_USE_WPS                                            //uncomment to use WPS instead of hard coded wifi credentials 
#define WIFI_SSID      "__enter_your_wifi_ssid_here__"          // not needed if WPS is used
#define WIFI_PASSWD    "__enter_your_wifi_password_here__"      // not needed if WPS is used.  Caution - Hard coded password is stored as clear text in BIN file

//  *************  Custom Timezone Offset *************
// ONLY define this on, if no geolocation is wanted/avaiable and GEOLOCATION_ENABLED is NOT defined
// you can use "+" and "-" in front of the value and decimal values. E.g.: "2" or "+2" or "-7" or "2.5" or "-4.75"
// No check is done, so values over 24 will turn over, but work, strings will fail (leading to 0 as offset)
// No auto-switch for Daylight-Saving-Time (Summertime) to Normal time with this! This is only avaiable with internet-based GeoLoc in the moment!
#define CUSTOM_TIMEZONE_OFFSET "2"

//  *************  Geolocation  *************
// Get your API Key on https://www.abstractapi.com/ (login) --> https://app.abstractapi.com/api/ip-geolocation/tester (key) *************
//#define GEOLOCATION_ENABLED    // enable after creating an account and copying Geolocation API below:
#define GEOLOCATION_API_KEY "__enter_your_api_key_here__" 


// ************* MQTT config *************
// Just for naming 
#if defined(HARDWARE_Elekstube_CLOCK)
    #define MQTT_SUFFIX "_Elekstube"
#elif defined(HARDWARE_Elekstube_CLOCK_Gen2)
    #define MQTT_SUFFIX "_ElekstubeGen2"
#elif defined(HARDWARE_SI_HAI_CLOCK)
    #define MQTT_SUFFIX "_SI_HAI"
#elif defined(HARDWARE_NovelLife_SE_CLOCK)
    #define MQTT_SUFFIX "_NovelLifeSE"
#elif defined(HARDWARE_PunkCyber_CLOCK)
    #define MQTT_SUFFIX "_PunkCyber"
#elif defined(HARDWARE_IPSTUBE_CLOCK)
    #define MQTT_SUFFIX "_IPSTUBE"
#else
    #define MQTT_SUFFIX "_Unknown"
#endif

#define MQTT_BASE_NAME  "EleksHAXTubeClock"

#define MQTT_HOME_ASSISTANT                                     // enable if you want Home Assistant support
#define MQTT_HOME_ASSISTANT_DISCOVERY
#define MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MANUFACTURER "EleksMaker"
#define MQTT_HOME_ASSISTANT_DISCOVERY_DEVICE_MODEL MQTT_BASE_NAME MQTT_SUFFIX
#define MQTT_HOME_ASSISTANT_DISCOVERY_SW_VERSION FIRMWARE_VERSION
#define MQTT_HOME_ASSISTANT_DISCOVERY_HW_VERSION "X.X.XX"

// You can either use an internet-based MQTT broker (smartnest.cz) or a local one (e.g. Mosquitto).
// If you choose an internet based one, you will need to create an account, setting up the device there and filling in the data below then.
// If you choose a local one, you will need to set up the broker on your local network and fill in the data below.
#define MQTT_ENABLED                                            // Enable MQTT if you want to control the clock via published messages
#define MQTT_BROKER     "192.168.0.100"                         // Broker host (or IP address) - can also be your local MQTT server!
#define MQTT_PORT       1883                                    // Broker port
#define MQTT_USERNAME   "__enter_your_username_here__"          // Username (i.e. from Smartnest or your local MQTT server)
#define MQTT_PASSWORD   "__enter_your_api_key_here__"           // Password (i.e. from Smartnest or the API key under "My Account" or from your local MQTT server)
#define MQTT_SAVE_PREFERENCES_AFTER_SEC 60
#define MQTT_CLIENT     MQTT_BASE_NAME MQTT_SUFFIX              // Device ID (i.e. from Smartnest) - This will be used as Root for each Topic - Local can be anything, like "EleksTubeHAXClock"


// ************* Optional temperature sensor *************
//#define ONE_WIRE_BUS_PIN   4  // DS18B20 connected to GPIO4; comment this line if sensor is not connected


#endif  // USER_DEFINES_H_
