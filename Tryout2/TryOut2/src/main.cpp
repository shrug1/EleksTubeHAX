//Test test IPSTube displays

// Used libraries:
// TFT_eSPI at version 2.5.43
// SPI at version 2.0.0 in esp32\2.0.16
// FS at version 2.0.0 in esp32\2.0.16
// SPIFFS at version 2.0.0 in esp32\2.0.16
//
// Arduino 2.3.2 with ESP32 Dev Module board.

#include <TFT_eSPI.h>
#include <SPI.h>

/* ---------------------------------------------------

IPSTube settings for ST7789 FP-114H03A in Libraries\TFT_eSPI\User_Setup.h

#define ST7789_DRIVER      // Full configuration option, define additional parameters below for this display
#define TFT_SDA_READ       // This option is for ESP32 ONLY, tested with ST7789 and GC9A01 display only
#define TFT_WIDTH  135
#define TFT_HEIGHT 240
#define SPI_FREQUENCY  55000000 // fastest tested to be working

#define TFT_MISO  -1
#define TFT_MOSI  32  //SDA
#define TFT_SCLK  33  //SCL

#define TFT_CS    -1  //CS: Don;t define a GPIO pin here
#define TFT_DC    25  //RS
#define TFT_RST   26  
#define TOUCH_CS  -1
 --------------------------------------------------- */

TFT_eSPI tft = TFT_eSPI(); 

int counter = 0;

#define TFT_CS_1   13 //(GPIO13)
#define TFT_CS_2   12 //(GPIO12)
#define TFT_CS_3   14 //(GPIO14)
#define TFT_CS_4   27 //(GPIO27)
#define TFT_CS_5   2  //(GPIO2)
#define TFT_CS_6   15 //(GPIO15)

int cspins[] = {TFT_CS_1,TFT_CS_2,TFT_CS_3,TFT_CS_4,TFT_CS_5,TFT_CS_6};

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

void setup() {
  Serial.begin(115200);
  delay(4000);  // Waiting for serial monitor to catch up.

  Serial.println("Set CS pins as output pins.");
  for(int i=0; i<6; i++) {
    pinMode(cspins[i],OUTPUT);
  }

  // Serial.println("Set GPIO_NUM_9 as output pin.");
  // pinMode(GPIO_NUM_9,INPUT);
  // digitalWrite(GPIO_NUM_9, HIGH);
  //-----------> Causes Watchdog faliure and CPU Reset!

  // Serial.println("Set GPIO_NUM_4 as output pin.");
  // pinMode(GPIO_NUM_4,OUTPUT);
  // digitalWrite(GPIO_NUM_4, HIGH);

  // Serial.println("Set GPIO_NUM_5 as output pin.");
  // pinMode(GPIO_NUM_5,OUTPUT);
  // digitalWrite(GPIO_NUM_5, HIGH);
    
  //Activate and init all dispays
  Serial.println("Initialising all displays all together now. Putting all CS pins LOW.");
  for(int i=0; i<6; i++) {
    digitalWrite(cspins[i], LOW);
  }
  tft.init();
  tft.setRotation(0);

  tft.fillScreen(YELLOW);


  
  //Deactivate CS for all dispays
  for(int i=0; i<6; i++) {
    digitalWrite(cspins[i], HIGH);
  }

  delay(500);

  //Serial.println("Wait 10 seconds before starting the loop.");
  //delay(10000);

  Serial.println("Initialised all displays.");
  }

void loop() {
  counter++;
  for (int i=0; i<6; i++) {

    Serial.print("Display# is :");
    Serial.print(i);
    Serial.print(" Pin is: ");
    Serial.println(cspins[i]);

    digitalWrite(cspins[i], LOW);
    delay(100);
    if (counter%2==0) {
      tft.fillScreen(YELLOW);
    } else {
      tft.fillScreen(BLACK);
    }   
    tft.setTextColor(WHITE,BLACK);
    //tft.setCursor (0, 128);
    tft.setCursor (0, 70, 2);
    tft.print("Text test on display ");
    tft.print(i);
    tft.println("!");
   
    // deselect display i
    digitalWrite(cspins[i], HIGH);
    delay(500);
  }
}