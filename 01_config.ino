


// Replace with your network credentials
const char ssid[]     = SECRET_WIFI1_SSID;
const char password[] = SECRET_WIFI1_PASS;

const char mdns_hostname[] = SECRET_HOST;

const char* mqtt_server = SECRET_MQTT_SERVER;
const int mqtt_port = 1883;
const char* MQTT_USER = SECRET_MQTT_USER;
const char* MQTT_PASSWORD = SECRET_MQTT_PASS;

const char* MQTT_PUBLISH_POWER = "stat/ESP32/" SECRET_HOST "/POWER";
const char* MQTT_RECEIVER_POWER = "cmnd/ESP32/" SECRET_HOST "/POWER";

const char* MQTT_PUBLISH_DIMMER = "stat/ESP32/" SECRET_HOST "/Dimmer";
const char* MQTT_RECEIVER_DIMMER = "cmnd/ESP32/" SECRET_HOST "/Dimmer";

const char* MQTT_PUBLISH_TYPE = "stat/ESP32/" SECRET_HOST "/Type";
const char* MQTT_RECEIVER_TYPE = "cmnd/ESP32/" SECRET_HOST "/Type";

const char* MQTT_PUBLISH_TIME = "stat/ESP32/" SECRET_HOST "/TIME";
const char* MQTT_RECEIVER_TIME = "cmnd/ESP32/" SECRET_HOST "/TIME";

const char* MQTT_PUBLISH_ITALIC = "stat/ESP32/" SECRET_HOST "/ITALIC";
const char* MQTT_RECEIVER_ITALIC = "cmnd/ESP32/" SECRET_HOST "/ITALIC";

const char* ntpServer = SECRET_NTP_SERVER;

const String localTimezoneString = SECRET_TIMEZONE_POSIX;

Timezone LocalTimezone;

AsyncWebServer server(80);

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;


uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

bool restartNow = false;


/* Start autoBrightness config/parameters -------------------------------------------------------------- */
#define LDR_PIN 35
uint8_t upperLimitLDR = 254;                      // everything above this value will cause max brightness (according to current level) to be used (if it's higher than this)
uint8_t lowerLimitLDR = 10;                       // everything below this value will cause minBrightness to be used
uint8_t minBrightness = 5;                       // anything below this avgLDR value will be ignored
uint8_t maxBrightness = 254;                       // anything below this avgLDR value will be ignored

const bool nightMode = false;                     // nightmode true -> if minBrightness is used, colorizeOutput() will use a single color for everything, using HSV
const uint8_t nightColor[2] = { 0, 70 };          // hue 0 = red, fixed brightness of 70, https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
float factorLDR = 1.0;                            // try 0.5 - 2.0, compensation value for avgLDR. Set dbgLDR true & define DEBUG and watch the serial monitor. Looking...
const bool dbgLDR = false;                        // ...for values roughly in the range of 120-160 (medium room light), 40-80 (low light) and 0 - 20 in the dark
uint8_t intervalLDR = 75;                         // read value from LDR every 75ms (most LDRs have a minimum of about 30ms - 50ms)
uint16_t avgLDR = 0;                              // we will average this value somehow somewhere in readLDR();
uint16_t lastAvgLDR = 0;                          // last average LDR value we got
static long lastReadLDR = millis();
/* End autoBrightness config/parameters ---------------------------------------------------------------- */
