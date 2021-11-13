#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

CRGB leds[NUM_LEDS];
CHSV colourOfNumbers( 0, 0, 180);

#define MAX_ARRAY_SIZE 13

const int Digits[2][10][MAX_ARRAY_SIZE] =
{
  {
    {  4,  7,  8, 10, 12, 17, 19, 21, 22, 25,  0,  0,  0 }, // 0
    {  7,  8, 10, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0 }, // 1
    {  7, 10, 11, 12, 17, 18, 19, 22,  0,  0,  0,  0,  0 }, // 2
    {  7,  8, 10, 11, 12, 17, 18, 19,  0,  0,  0,  0,  0 }, // 3
    {  7,  8, 10, 11, 18, 19, 21,  0,  0,  0,  0,  0,  0 }, // 4
    {  8, 10, 11, 12, 17, 18, 19, 21,  0,  0,  0,  0,  0 }, // 5
    {  8, 10, 11, 12, 14, 17, 18, 22,  0,  0,  0,  0,  0 }, // 6
    {  7, 11, 12, 15, 17, 19,  0,  0,  0,  0,  0,  0,  0 }, // 7
    {  7,  8, 10, 11, 12, 17, 18, 19, 21, 22,  0,  0,  0 }, // 8
    {  7, 11, 12, 15, 17, 18, 19, 21,  0,  0,  0,  0,  0 }, // 9
  },
  {
    {  5,  7, 11, 12, 15, 17, 19, 21, 24, 25, 29, 31,  0 }, // 0
    {  5,  7, 11, 15, 17,  0,  0,  0,  0,  0,  0,  0,  0 }, // 1
    {  5,  7, 11, 12, 17, 18, 19, 24, 25, 29, 31,  0,  0 }, // 2
    {  5,  7, 11, 12, 15, 17, 18, 19, 24, 31,  0,  0,  0 }, // 3
    {  7, 11, 15, 17, 18, 19, 21, 25,  0,  0,  0,  0,  0 }, // 4
    {  5, 11, 12, 15, 17, 18, 19, 21, 24, 25, 31,  0,  0 }, // 5
    { 11, 12, 15, 17, 18, 19, 21, 24, 25, 29, 31,  5,  0 }, // 6
    {  5,  7, 11, 12, 15, 17, 19,  0,  0,  0,  0,  0,  0 }, // 7
    {  5,  7, 11, 12, 15, 17, 18, 19, 21, 24, 25, 29, 31 }, // 8
    {  5,  7, 11, 12, 15, 17, 18, 19, 21, 24, 25, 31,  0 }, // 9
  }
};

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, drawBlackBackground };


void displaySetup() {
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(BRIGHTNESS);
}

void displayLoop() {
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
  if (running) {
    #ifdef AUTOBRIGHTNESS
    if ( millis() - lastReadLDR >= intervalLDR ) {     // if LDR is enabled and sample interval has been reached...
      readLDR();                                       // ...call readLDR();
      if ( abs(avgLDR - lastAvgLDR) >= 5 ) {           // if avgLDR has changed for more than +/- 5 update lastAvgLDR
        lastAvgLDR = avgLDR;
        #ifdef DEBUGGING
          Serial.print("Setting brightness to: ");
          Serial.println(avgLDR);
        #endif
        FastLED.setBrightness(avgLDR);
      }
      lastReadLDR = millis();
    }
  #endif

    
    gPatterns[gCurrentPatternNumber]();
    if (timeOnOff) {
      if (italic) {
        drawTimeItalic();
      } else {
        drawTime();
      }
    }
    FastLED.show();  
    FastLED.delay(1000/FRAMES_PER_SECOND);
     // slowly cycle the "base color" through the rainbow
  } else {
    FastLED.clear();  // clear all pixel data
    FastLED.show();
  }
}


void displayNumber( int place , int number , CHSV colour, int italic){
  // place 0 = left , 1 = right .
  int f = (int)number / 10;
  int s = (int)number % 10;
  int fading = map((dimming), 0, 100, 0, 255);
  for (int i = 0; i < MAX_ARRAY_SIZE; i++) {
    if (Digits[italic][f][i] != 0 ) {
      int num = Digits[italic][f][i] + 28 + place;
      if (italic && place != 0 && num > 126) {
        num--;
      }
      leds[num] = colour;
      leds[num].fadeLightBy(fading);
    }
  }
  
  for (int i = 0; i < MAX_ARRAY_SIZE; i++) {
    if (Digits[italic][s][i] != 0 ) {
      int num = Digits[italic][s][i] + place;
      leds[num] = colour;
      leds[num].fadeLightBy(fading);
    }
  }
  
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void drawTimeBackground() {

  int fading = map((100-dimming), 0, 100, 0, 255);
  #ifdef DEBUGGING
    /*Serial.print("fading: ");
    Serial.print(fading);
    Serial.print(" dimming: ");
    Serial.println(dimming);*/
  #endif
  fill_rainbow( leds, NUM_LEDS, gHue, 2);
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeLightBy(fading);
  }
}

void drawBlackBackground() {
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void onlyTime() {
  drawBlackBackground();
  drawTime();
}

void onlyItalic() {
  drawBlackBackground();
  drawTimeItalic();
}

void showtime()
{
  drawTimeBackground();
  drawTime();
}

void showtimeItalic()
{
  drawTimeBackground();
  drawTimeItalic();
}

void drawTime () {
  timeInt newTime = getNewTime();
  // Serial.println(newTime.minute);
  displayNumber(70, newTime.hour, colourOfNumbers, 0);
  displayNumber(0, newTime.minute, colourOfNumbers, 0);

  if (secondBlinkEnabled && secondPointsEnabled) {
    if(newTime.showSec) {
      showSecondPoints(colourOfNumbers, 0);
    }
  } else if(secondPointsEnabled) {
    showSecondPoints(colourOfNumbers, 0);
  }
}

void drawTimeItalic () {
  timeInt newTime = getNewTime();
  // Serial.println(newTime.minute);
  displayNumber(70, newTime.hour, colourOfNumbers, 1);
  displayNumber(0, newTime.minute, colourOfNumbers, 1);

  if (secondBlinkEnabled && secondPointsEnabled) {
    if(newTime.showSec) {
      showSecondPoints(colourOfNumbers, 1);
    }
  } else if(secondPointsEnabled) {
    showSecondPoints(colourOfNumbers, 1);
  }
}

void showSecondPoints(CHSV colour, int italic){
  
  int fading = map((dimming), 0, 100, 0, 255);
  if (italic == 1) {
    leds[63] = colour;
    leds[71] = colour;
    
    leds[63].fadeLightBy(fading);
    leds[71].fadeLightBy(fading);
  } else {
    leds[63] = colour;
    leds[64] = colour;
    leds[63].fadeLightBy(fading);
    leds[64].fadeLightBy(fading);
  }
}
void rainbow() 
{
  
  // FastLED's built-in rainbow generator
  
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  
  int fading = (100-dimming) * (256/100);
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

#ifdef AUTOBRIGHTNESS
void readLDR() {                                                                                            // read LDR value 5 times and write average to avgLDR
  static uint8_t runCounter = 1;
  static uint16_t tmp = 0;
  int LDRval = analogRead(LDR_PIN);

  uint8_t readOut = map(LDRval, 0, 4095, minBrightness, maxBrightness);
  tmp += readOut;
  if (runCounter == 5) {
    avgLDR = ( tmp / 5 )  * factorLDR;
    tmp = 0;
    runCounter = 0;
    #ifdef DEBUGGING
      Serial.print(F("readLDR(): avgLDR value: "));
      Serial.print(avgLDR);
    #endif
    if ( avgLDR >= upperLimitLDR ) {
      avgLDR = maxBrightness; 
    } else if ( avgLDR <= lowerLimitLDR ) {
      avgLDR = minBrightness;
    }
    #ifdef DEBUGGING
      Serial.print(F(" - adjusted to: "));
      Serial.println(avgLDR);
    #endif
  }
  runCounter++;
}
#endif
