#include <Adafruit_NeoPixel.h>
#include <DS3231_Simple.h>
#define REDBUT 2
#define WHIBUT 3
#define NEOPIN 4
#define NUMPIXELS 16
Adafruit_NeoPixel pixels = 
//  Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_RGB + NEO_KHZ800); //Nano
  Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800); //UNO

//Night-light variables
const uint8_t baseB = 0;    //Base brightness
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

DS3231_Simple Clock;

void setup() {
  Clock.begin();
  pinMode(REDBUT, INPUT_PULLUP);
  pinMode(WHIBUT, INPUT_PULLUP);
  redButtonPressed = !digitalRead(REDBUT);
  whiteButtonPressed = !digitalRead(WHIBUT);
//  redButtonPressed = true;
  if (redButtonPressed or whiteButtonPressed) { alarmOnly = false; }
  pixels.begin();
  setAllPixels(pixels.Color(0,0,0));
  pixels.show();
  showTimeOnGrid(pixels.Color(1,0,0), Clock.read());
  delay(5000);
  if (!alarmOnly) {
    colorFlash(1, pixels.Color(1,0,0),1000,1000);
    setAllPixels(pixels.Color(red,0,0));
  }
}

void loop() {
  redButtonPressed   = !digitalRead(REDBUT);
  whiteButtonPressed = !digitalRead(WHIBUT);
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

void showTimeOnGrid(uint32_t c, DateTime dt){
  int n0, n1, n2, n3;
  n0 = dt.Hour / 10;
  n1 = dt.Hour % 10;
  n2 = dt.Minute / 10;
  n3 = dt.Minute % 10;
  setNeoGrid(c, n0, n1, n2, n3);
}

void setNeoGrid(uint32_t c, int n0, int n1, int n2, int n3) {
  setNeoLine(0, n0, c);
  setNeoLine(1, n1, c);
  setNeoLine(2, n2, c);
  setNeoLine(3, n3, c);
  pixels.show();
}

void setNeoLine(int line, int num, uint32_t c) {
  bool BOOL_NUM[4];
  bool leftToRight = line % 2;
  int BOOL_POS;
  if (leftToRight){
    BOOL_POS = 0;
  } else {
    BOOL_POS = 3;
  }
  decToBin(BOOL_NUM, num, 4);
  for (int pix = 4 * line; pix < 4 * (line +1); pix++){
    if (BOOL_NUM[BOOL_POS]){
      pixels.setPixelColor(pix, c);
    } else {
      pixels.setPixelColor(pix, pixels.Color(0,0,0));
    }
    if (leftToRight){
      BOOL_POS++;
    } else {
      BOOL_POS--;
    }
  }
}

void decToBin(bool* BOOL_OUT, int INT_IN, int MAX_BITS){
  int MAX_VAL = 1;
  for (int i = 0; i < MAX_BITS; i++){
    MAX_VAL *= 2;
    BOOL_OUT[i] = 0;
  }
  if (INT_IN > MAX_VAL){
    for (int i = 0; INT_IN > 0; i++){
      BOOL_OUT[i] = 0;
    }
  }
  for (int i = 0; INT_IN > 0; i++){
    BOOL_OUT[i] = INT_IN % 2;
    INT_IN /= 2;
  }
}

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
  if (activeCounter == 0 or activeCounter == 255 or allButtonsReleased) {
    activeCounterUp = !activeCounterUp;
  }
  if (activeCounterUp) {
    activeCounter+=step;
    if (activeCounter > 255 - step) {activeCounter = 255;}
  } else {
    activeCounter-=step;
    if (activeCounter < step) {
      activeCounter = 0;
      delayMultiplier = 10;
    }
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
