/*
 * 
 * Add and define following variables in a file called arduino_secrets.h
 * 
#define SECRET_WIFI1_PASS
#define SECRET_WIFI1_SSID
#define SECRET_WIFI2_PASS
#define SECRET_WIFI2_SSID

#define SECRET_MQTT_SERVER
#define SECRET_MQTT_USER
#define SECRET_MQTT_PASS

#define SECRET_NTP_SERVER

#define SECRET_TIMEZONE_POSIX
 * 
 * 
 * 
 */



#include "arduino_secrets.h"
#include "index_html.h"
#include "login_html.h"

#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <ezTime.h>

const char* host = "ambientlight";

// Replace with your network credentials
const char* ssid0     = SECRET_WIFI1_SSID;
const char* password0 = SECRET_WIFI1_PASS;
// Replace with your network credentials
const char* ssid1     = SECRET_WIFI2_SSID;
const char* password1 = SECRET_WIFI2_PASS;

const char* mqtt_server = SECRET_MQTT_SERVER;
const int mqtt_port = 1883;
const char* MQTT_USER = SECRET_MQTT_USER;
const char* MQTT_PASSWORD = SECRET_MQTT_PASS;


#define MQTT_PUBLISH_POWER "stat/ESP32/ambienttime/POWER"
#define MQTT_RECEIVER_POWER "cmnd/ESP32/ambienttime/POWER"

#define MQTT_PUBLISH_DIMMER "stat/ESP32/ambienttime/Dimmer"
#define MQTT_RECEIVER_DIMMER "cmnd/ESP32/ambienttime/Dimmer"

#define MQTT_PUBLISH_TYPE "stat/ESP32/ambienttime/Type"
#define MQTT_RECEIVER_TYPE "cmnd/ESP32/ambienttime/Type"

const char* ntpServer = SECRET_NTP_SERVER;
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const String localTimezoneString = SECRET_TIMEZONE_POSIX;

Timezone LocalTimezone;

WebServer server(80);

WiFiClient wifiClient;

PubSubClient client(wifiClient);

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    13
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    128
CRGB leds[NUM_LEDS];
CHSV colourOfNumbers( 0, 0, 180);

#define BRIGHTNESS          45
#define FRAMES_PER_SECOND  120


String formattedTime;
String timeStamp;

bool running = false;
int dimming = 10;
int everymsec = 200;
bool secondBlinkEnabled = true;
bool secondPointsEnabled = true;

int wifi=0;
int wificount=0;
int counter=0;
const int Digits[10][10] =
{
  {4,7,8,10,12,17,19,21,22,25}, // 0
  {7,8,10,11,12}, // 1
  {7,10,11,12,17,18,19,22}, // 2
  {7,8,10,11,12,17,18,19}, // 3
  {7,8,10,11,18,19,21}, // 4
  {8,10,11,12,17,18,19,21}, // 5
  {8,10,11,12,14,17,18,22}, // 6
  {7,11,12,15,17,19}, // 7
  {7,8,10,11,12,17,18,19,21,22}, // 8
  {7,11,12,15,17,18,19,21}, // 9
};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

typedef struct {
  int hour, minute;
  bool showSec;
} timeInt;

timeInt getNewTime()
{
  int seconds = LocalTimezone.second();
  bool showsec = false;
  if (seconds%2==0) {
    showsec = true;
  }
  return (timeInt){LocalTimezone.hour(), LocalTimezone.minute(), showsec};
}

void printLocalTime()
{
  Serial.println("Local time: " + LocalTimezone.dateTime());
}

void setup_ota() {
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void setup_wifi() {

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid0);
  WiFi.begin(ssid0, password0);
  while (WiFi.status() != WL_CONNECTED) {
    if (wificount > 0) {
      Serial.print("Connecting to ");
      if (wificount%2==0) {
        WiFi.begin(ssid1, password1);
        Serial.println(ssid1);
      } else {
        WiFi.begin(ssid0, password0);
        Serial.println(ssid0);
      }
    }
    delay(5000);
    wificount++;
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      sendStatus();
      sendDimmer();
      sendType();
      // ... and resubscribe
      client.subscribe(MQTT_RECEIVER_POWER);
      client.subscribe(MQTT_RECEIVER_DIMMER);
      client.subscribe(MQTT_RECEIVER_TYPE);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
  Serial.println("-------new message from broker-----");
  Serial.print("channel:");
  Serial.println(topic);
  Serial.print("data:");
  Serial.write(payload, length);
  Serial.println();

  String myPayload = String((char *)payload);
  String cutPayload = myPayload.substring(0,length);

  Serial.println(cutPayload);
  
  if(strcmp(topic, MQTT_RECEIVER_POWER) == 0) {
    setStatus(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_DIMMER) == 0) {
    setDimmer(cutPayload);
  } else if(strcmp(topic, MQTT_RECEIVER_TYPE) == 0) {
    setType(cutPayload);
  }
}

void setStatus(String cutPayload) {
  if (cutPayload == "ON") {
    running = true;
  } else if(cutPayload == "OFF") {
    running = false;
  }
  sendStatus();
}

void sendStatus() {
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_PUBLISH_POWER, running?"ON":"OFF");
}

void setDimmer(String cutPayload) {
  try {
    dimming = cutPayload.toInt();
  } catch(...) {
    Serial.println("parsing to int has not worked");
  }
  sendDimmer();
}

void sendDimmer() {
    if (!client.connected()) {
    reconnect();
  }
  String d = String(dimming);
  client.publish(MQTT_PUBLISH_DIMMER, (char*) d.c_str());
}

void setType(String cutPayload) {
  try {
    gCurrentPatternNumber = cutPayload.toInt();
  } catch(...) {
    Serial.println("parsing to int has not worked");
  }
  sendDimmer();
}

void sendType() {
  if (!client.connected()) {
    reconnect();
  }
  Serial.println("test");
  char *s = (char*) String((char) gCurrentPatternNumber).c_str(); 
  client.publish(MQTT_PUBLISH_TYPE, s);
  Serial.println("test2");
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_PUBLISH_POWER, serialData);
}

void setup() {
  delay(3000); // 3 second delay for recovery

  // Initialize Serial Monitor
  Serial.begin(115200);


  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  setup_ota();


  //init time and get the time
  waitForSync();
  // setDebug(INFO);
  
  setServer(ntpServer);
  setInterval(60);
  
  Serial.println("UTC: " + UTC.dateTime());
  
  LocalTimezone.setPosix(localTimezoneString);
  Serial.println("Local time: " + LocalTimezone.dateTime());
  
  printLocalTime();


  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  // FastLED.setBrightness(BRIGHTNESS);
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { showtime, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

void displaynumber( int place , int number , CHSV colour){// place 0 = left , 1 = right .
  for (int i = 0 ; i < 10 ; i++) {
    if (Digits[number/10][i] != 0) {
      leds[(Digits[number/10][i]+28+place)] = colour;
    }
    if (Digits[number%10][i] != 0) {
      leds[(Digits[number%10][i]+place)] = colour;
    }
  }
}

void loop()
{
  client.loop();
  server.handleClient();
  events();
  /*
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
  */
  if (running) {
    gPatterns[gCurrentPatternNumber]();
    
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND);
     // slowly cycle the "base color" through the rainbow
  } else {
    FastLED.clear();  // clear all pixel data
    FastLED.show();
  }
  EVERY_N_SECONDS( 10 ) {
    sendStatus();
    sendDimmer();
  } // reconnect to mqtt if disconnected

  EVERY_N_SECONDS( 86400 ) {
    esp_restart();
  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void showtime()
{
  EVERY_N_MILLISECONDS( 200 ) { gHue++; }
  int fading = (100-dimming) * (256/100);
  //Serial.println("fading: ");
  //Serial.print(fading);
  fill_rainbow( leds, NUM_LEDS, gHue, 0.5);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeLightBy(fading);
  }
  timeInt newTime = getNewTime();
  // Serial.println(newTime.minute);
  displaynumber(70, newTime.hour, colourOfNumbers);
  displaynumber(0, newTime.minute, colourOfNumbers);
  if (secondBlinkEnabled && secondPointsEnabled) {
    if(newTime.showSec) {
      showSecondPoints(colourOfNumbers);
    }
  } else if(secondPointsEnabled) {
    showSecondPoints(colourOfNumbers);
  }
}

void showSecondPoints(CHSV colour){
  leds[63] = colour;
  leds[64] = colour;
}
void rainbow() 
{
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
  // FastLED's built-in rainbow generator
  int fading = (100-dimming) * (256/100);
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeLightBy(fading);
  }
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
