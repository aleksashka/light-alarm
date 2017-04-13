#include <Adafruit_NeoPixel.h>
#define PIN 11
#define REDBUT 12
#define WHIBUT 13
#define NUMPIXELS 16
Adafruit_NeoPixel pixels = 
//  Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800); //Nano
  Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //UNO

//Night-light variables
const uint8_t baseB = 1;    //Base brightness
//uint32_t countdown = 600000;//10 minutes countdown
uint16_t baseDelay = 50;    // Base delay within loop function
uint8_t p1, p2, p3, delayMultiplier, newVal;
uint8_t red   = baseB;//Initial value
uint8_t white = 1;    //Initial value
uint8_t step  = 1;    //Brightness step value
bool redUp   = false; //Increase (true) or decrease (false) brightness for red color (reversed on each press)
bool whiteUp = false; //Increase (true) or decrease (false) brightness for white color (reversed on each press)
bool allButtonsReleased = true; // Set to true in loop if all buttons are released
bool redButtonPressed   = false;
bool whiteButtonPressed = false;
bool doubleButtonOdd = false;
bool alarmOnly = true; //on boot - light alarm, press any key to switch to night light

void setup() {
  pinMode(REDBUT, INPUT);
  pinMode(WHIBUT, INPUT);
  redButtonPressed = digitalRead(REDBUT);
//  redButtonPressed = true;
  if (redButtonPressed) { alarmOnly = false; }
  pixels.begin();
  pixels.show();
  if (!alarmOnly) {
    colorFlash(2, pixels.Color(red,0,0),1000,1000);
    setAllPixels(pixels.Color(red,0,0));
  }
}

void loop() {
  redButtonPressed   = digitalRead(REDBUT);
  whiteButtonPressed = digitalRead(WHIBUT);
  if (alarmOnly) {
    fadeInWhiteLinear(17248);                             //Fade in for  00:25:00
    fadeInOutWhiteLinear(62,10);                          //Fade in-out  00:02:00
    colorFlash(30, pixels.Color(255,255,0),1000,1000);    //Flash Yellow 00:01:00
    colorFlash(30, pixels.Color(0,255,0),1000,1000);      //Flash Green  00:01:00
    colorFlash(30, pixels.Color(0,0,255),1000,1000);      //Flash Blue   00:01:00
    colorFlash(600, pixels.Color(255,255,255),500,500);   //Flash White  00:10:00
    //delay(84600000-32);                                   //Wait    for 23:30:00 -.032
  }
  nightLight();
  delay(baseDelay * delayMultiplier / ((p1/32)+1) );
  delayMultiplier = 1;
}//loop

void nightLight() {
  if (redButtonPressed or whiteButtonPressed) {
    if (redButtonPressed and whiteButtonPressed) {
      if (doubleButtonOdd) {
        newVal = 255;
        delayMultiplier = 100;
      } else {
        newVal = 0;
        delayMultiplier = 10;
      }
      if (red == 1) {
        whiteUp = doubleButtonOdd;
        white = p1 = p2 = p3 = newVal;
      }
      else if (white == 1) {
        redUp = doubleButtonOdd;
        red = p1 = newVal;
        p2 = p3 = 0;
      }
      doubleButtonOdd = !doubleButtonOdd;
    }//red and white
    else if (redButtonPressed) {
      upDown(red, step, redUp, white, whiteUp);
      p1 = red;
      p2 = p3 = 0;
    }//red
    else if (whiteButtonPressed) {
      upDown(white, step, whiteUp, red, redUp);
      p1 = p2 = p3 = white;
    }//white
    setAllPixels(pixels.Color(p1,p2,p3));
    allButtonsReleased = false;
  }//red or white
  else {
    allButtonsReleased = true;
    doubleButtonOdd = false;
  }
}

//References (&) to variables are passed instead of just values
void upDown(uint8_t& activeCounter, uint8_t step, bool& activeCounterUp, uint8_t& inactiveCounter, bool& inactiveCounterUp) {
  inactiveCounter = 1; inactiveCounterUp = false;
  if (activeCounter == 0 or activeCounter == 255 or allButtonsReleased) {activeCounterUp = !activeCounterUp;}
  if (activeCounterUp) {
    activeCounter+=step;
    if (activeCounter > 255 - step) {activeCounter = 255;}
  } else {
    activeCounter-=step;
    if (activeCounter < step) {activeCounter = 0;}
  }
}

// Fade in all pixels with white color
void fadeInWhite(uint16_t wait) {
  for (int i = 0; i < 256; i++) {
    setAllPixels(pixels.Color(i,i,i));
    delay(wait);
  }
}

// Fade in all pixels with white color in "linear" fashion
void fadeInWhiteLinear(uint16_t wait) {
  for (int i = 0; i < 256; i++) {
    setAllPixels(pixels.Color(i,i,i));
    delay( wait / ((i/32)+1) );
  }
}

// Fade in all pixels with white color in "linear" fashion
void fadeInOutWhiteLinear(uint16_t count, uint16_t wait) {
for (int c = 0; c < count; c++){
  for (int i = 0; i < 256; i++) {
    setAllPixels(pixels.Color(i,i,i));
    delay( wait / ((i/32)+1) );
  }
  for (int i = 255; i > 0; i--) {
    setAllPixels(pixels.Color(i,i,i));
    delay( wait / ((i/32)+1) );
  }
}
}

void setAllPixels(uint32_t c) {
  for (uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
  }
  pixels.show();
}

// Turn all LEDs on, wait, turn off, wait
void colorFlash(uint16_t num, uint32_t c, 
                uint16_t waitOn, uint16_t waitOff) {
  for (uint16_t j=0; j<num; j++) {
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, c);
    }
    pixels.show();
    delay(waitOn);
    
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0));
    }
    pixels.show();
    delay(waitOff);
  }
}
