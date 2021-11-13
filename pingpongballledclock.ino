// Code for "ESP32 Dev Module"

#include "arduino_secrets.h"
#include "customTypes.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <AsyncElegantOTA.h>
#include <ArduinoOTA.h>
#include <Arduino_JSON.h>
#include <Update.h>
#include <ezTime.h>
#include <FastLED.h>

//#define DEBUGGING //uncomment for serial debug messages, no serial messages if this whole line is a comment!
#define AUTOBRIGHTNESS
#define BAUDRATE 115200

FASTLED_USING_NAMESPACE


#define DATA_PIN    13
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    128

#define BRIGHTNESS          45
#define FRAMES_PER_SECOND  120

bool secondBlinkEnabled = true;
bool secondPointsEnabled = true;

bool running = true;
bool timeOnOff = true;
bool italic = false;
int dimming = 100;
int everymsec = 200;

void setup() {  
  // Serial port for debugging purposes
  #ifdef DEBUGGING
    Serial.begin(BAUDRATE);
    Serial.println("master start");
  #endif
  mqttSetup();
  wifiSetup();
  otaSetup();
  webSetup();
  timeSetup();
  displaySetup();
}

void loop()
{
  displayLoop();
  ArduinoOTA.handle();
  mqttLoop();
  checkForRestart();
}
