#include "Gestures.h"

// void Gestures::GestureStart()
// {
//     // for gesture sensor APDS9660 - Set interrupt pin on ESP32 as input
//   pinMode(GESTURE_SENSOR_INPUT_PIN, INPUT);

//   // Initialize interrupt service routine for interupt from APDS-9960 sensor
//   attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), GestureInterruptRoutine, FALLING);

//   // Initialize gesture sensor APDS-9960 (configure I2C and initial values)
//   if ( apds.init() ) {
//     Serial.println(F("APDS-9960 initialization complete"));

//     //Set Gain to 1x, bacause the cheap chinese fake APDS sensor can't handle more (also remember to extend ID check in Sparkfun libary to 0x3B!)
//     apds.setGestureGain(GGAIN_1X);
          
//     // Start running the APDS-9960 gesture sensor engine
//     if ( apds.enableGestureSensor(true) ) {
//       Serial.println(F("Gesture sensor is now running"));
//     } else {
//       Serial.println(F("Something went wrong during gesture sensor enablimg in the APDS-9960 library!"));
//     }
//   } else {
//     Serial.println(F("Something went wrong during APDS-9960 init!"));
//   }
// }

// void Gestures::HandleGestureInterupt()
// {
//   //Handle Interrupt from gesture sensor and simulate a short button press (state down_edge) of the corresponding button, if a gesture is detected 
//   if( isr_flag == 1 ) {
//     detachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN));
//     HandleGesture();
//     isr_flag = 0;
//     attachInterrupt(digitalPinToInterrupt(GESTURE_SENSOR_INPUT_PIN), GestureInterruptRoutine, FALLING);
//   }
//   return;
// }

// //mark, that the Interrupt of the gesture sensor was signaled
// void Gestures::GestureInterruptRoutine() {
// #ifdef HARDWARE_NovelLife_SE_CLOCK // NovelLife_SE Clone XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//   isr_flag = 1;
//  #endif
//   return;
// }

// //check which gesture was detected
// void Gestures::HandleGesture() {
//     //Serial.println("->main::HandleGesture()");
//     if ( apds.isGestureAvailable() ) {
//     switch ( apds.readGesture() ) {
//       case DIR_UP:
//         buttons.left.setDownEdgeState();
//         Serial.println("LEFT");
//         break;
//       case DIR_DOWN:
//         buttons.right.setDownEdgeState();
//         Serial.println("RIGHT");
//         break;
//       case DIR_LEFT:
//         buttons.power.setDownEdgeState();
//         Serial.println("DOWN");
//         break;
//       case DIR_RIGHT:
//         buttons.mode.setDownEdgeState();
//         Serial.println("UP");
//         break;
//       case DIR_NEAR:
//         buttons.mode.setDownEdgeState();
//         Serial.println("NEAR");
//         break;
//       case DIR_FAR:
//         buttons.power.setDownEdgeState();
//         Serial.println("FAR");
//         break;
//       default:        
//         Serial.println("NONE");
//     }
//   }
//   return;
// }
