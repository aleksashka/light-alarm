#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUMPIXELS 10
Adafruit_NeoPixel pixels = 
//  Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800); //Nano?
  Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //UNO?

void setup() {
  pinMode(13, OUTPUT);
  pixels.begin();
  pixels.show();
}

void loop() {
  fadeInWhiteLinear(17248);                             //Fade in for  00:25:00
  fadeInOutWhiteLinear(62,10);                          //Fade in-out  00:02:00
  colorFlash(30, pixels.Color(255,255,0),1000,1000);    //Flash Yellow 00:01:00
  colorFlash(30, pixels.Color(0,255,0),1000,1000);      //Flash Green  00:01:00
  colorFlash(30, pixels.Color(0,0,255),1000,1000);      //Flash Blue   00:01:00
  colorFlash(600, pixels.Color(255,255,255),500,500);   //Flash White  00:10:00
  //delay(84600000-32);                                   //Wait    for 23:30:00 -.032
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
