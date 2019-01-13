#include <Adafruit_NeoPixel.h>
#include <DS3231_Simple.h>
#define REDBUT 2
#define GREBUT 3
#define NEOPIN 4
#define NUMPIXELS 16
Adafruit_NeoPixel pixels = 
//  Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_RGB + NEO_KHZ800); //Nano
  Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800); //UNO

void showTimeOnGrid(DateTime dt, uint32_t c){
  int n0, n1, n2, n3;

  n0 = dt.Hour / 10;
  n1 = dt.Hour % 10;
  n2 = dt.Minute / 10;
  n3 = dt.Minute % 10;
  setNeoGrid(n0, n1, n2, n3, c);
}

void setNeoGrid(int n0, int n1, int n2, int n3, uint32_t c) {
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

void setAllPixels(uint32_t c) {
  for (uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
  }
  pixels.show();
}

class fader {
  int B; // B - current brightness to set (index of bVal array)
  int maxB = 31; // maxB - maximum brightness (index of bVal array)
  int prevB = -1; // prevB - previous value of brightness
  byte mode; // 1 - fade in
             // 2 - fade out
             // 3 - fade inOut
  byte seq; // Sequence number to track modes
  int count; // Number of times to run fader
  byte fromB, toB;// start and end bVal to use in fader::update
  bool enabled = false; // true if fader object is enabled
  unsigned long startTime = 0;
  unsigned long duration = 0; // total fade duration
  unsigned long scaledDuration = 0; // mode-dependant duration (half in mode 3)
  //http://forum.arduino.cc/index.php?topic=147818.msg1113263#msg1113263
  byte bVal[32] = {  0,   1,   2,   3,   4,   5,   7,   9,
                    12,  15,  18,  22,  27,  32,  38,  44,
                    51,  58,  67,  76,  86,  96, 108, 120,
                   134, 148, 163, 180, 197, 216, 235, 255};

  public:

  void enable(unsigned long duration_, byte mode_ = 1, byte seq_ = 1, int count_ = 1){
    duration = duration_;
    mode = mode_;
    seq = seq_;
    count = count_;
    switch (mode) {
      case 2: fromB = maxB; toB = 0;
              scaledDuration = duration;
              break;
      case 3: scaledDuration = duration / 2;
              break;
      default:fromB = 0;    toB = maxB;
              scaledDuration = duration;
              mode = 1;
    }
    enabled = true;
    startTime = millis();
  }

  void enable(){
    enabled = true;
    startTime = millis();
  }

  void disable(){
    enabled = false;
    setAllPixels(pixels.Color(0,0,0));
  }

  void setMode(byte mode_){ mode = mode_; }
  byte getMode()          { return mode;  }

  void setSeq(byte seq_)  { seq = seq_;   }
  byte getSeq()           { return seq;   }

  // Inspired by http://forum.arduino.cc/index.php?topic=323704.msg2237381#msg2237381
  byte update(){
    if (enabled) {
      unsigned long curTime = millis();
      if (curTime - startTime > duration) {
        if (count == 1){
          enabled = false;
          return seq;
        } else {
          startTime = millis();
          count -= 1;
        }
      }
      if (mode == 3) {
        if (curTime - startTime < scaledDuration) {
          // The first half of duration (fadeIn)
          fromB = 0;    toB = maxB;
        } else {
          // The second half of duration (fadeOut)
          fromB = maxB; toB = 0;
          curTime -= scaledDuration;
        }
      }
    // Original version, then -startTime, then scaledDuration
    //B = map(curTime            , startTime, startTime + duration, fromB, toB);
    //B = map(curTime - startTime,         0,             duration, fromB, toB);
      B = map(curTime - startTime,         0,       scaledDuration, fromB, toB);
      if (B != prevB) {
        prevB = B;
        setAllPixels(pixels.Color(bVal[B], bVal[B], bVal[B]));
      }
    }
    return 0;
  }
};

class setHM{ // Set hour and minute
  int setTimeout = 10000; //how long to wait after interaction until exit
  int blinkTimeout = 300; //how long highlight current line
  unsigned long curLineStamp = 0; //timestamp of last change of current line
  unsigned long lastStamp = 0; //timestamp of last user interaction
  byte h1, h0, m1, m0; //digits 1 and 0 of hours (h) and minutes (m)
  byte curLine; //current line to change digit
  uint32_t c; //main color to show all digits
  uint32_t b; //blink color to highlight current line
  bool modified; //true if any value has been modified

  public:
  void getTimestamp(DateTime & dt) {
    dt.Hour   = h1*10 + h0;
    dt.Minute = m1*10 + m0;
  }
  void enable(DateTime dt, uint32_t c_, uint32_t b_){
    h1 = dt.Hour / 10;
    h0 = dt.Hour % 10;
    m1 = dt.Minute / 10;
    m0 = dt.Minute % 10;
    c  = c_;
    b  = b_;
    curLine = 0;
    modified = false;
    curLineStamp = lastStamp = millis();
  }
  byte nextLine(){
    curLineStamp = lastStamp = millis();
    if (curLine == 3) {// no more lines
      if (modified) {
        return 1;//something was changed
      } else {
        return 2;//nothing was changed
      }
    } else {//more lines to check
      curLine += 1;
    }
    return 0;//nothing to do yet
  }
  byte click(){
    lastStamp = millis();
    modified = true;
    switch (curLine) {
      case 0: if (h1 >= 2) h1 = 0; else h1 += 1; break;
      case 1: if (h0 >= 9) {
                h0 = 0;
              } else if (h1 == 2 and h0 >= 3) {
                h0 = 0;
              } else {
                h0 += 1;
              }
              break;
      case 2: if (m1 >= 5) m1 = 0; else m1 += 1; break;
      case 3: if (m0 >= 9) m0 = 0; else m0 += 1; break;
    };
  }
  byte update(bool & boolSet){
    unsigned long curTime = millis();
    if (curTime - lastStamp > setTimeout){
      boolSet = 0;
      setNeoGrid(0, 0, 0, 0, c);
      return 1;//timeout happened
    }
    if (curTime - curLineStamp < blinkTimeout){
      setNeoLine(0, h1, c);
      setNeoLine(1, h0, c);
      setNeoLine(2, m1, c);
      setNeoLine(3, m0, c);
      setNeoLine(curLine, 15, b);
      pixels.show();
    } else {
      setNeoGrid(h1, h0, m1, m0, c);
    }
    return 0;
  }
};

class button{
  int debounce = 50;
  int longPressTimeout = 1000;      // time after hold to initiate long press
  int longPressRepeatTimeout = 300; // time after last long press until next long press
  int curLongPressRepeatTimeout = 0;// current time above
  int pin;
  bool curPressed;    // true if currently pressed
  bool prevPressed;   // true if previously was pressed
  bool longPressInAct;// true if long press is in action
  bool varSingleClick;// single press event
  bool varLongPress;  // long press event
  unsigned long lastPressTime;
  unsigned long lastLongPressTime;
  unsigned long upTime;
  unsigned long downTime;

  public:
  button (int pin_, int mode_){
    pin = pin_;
    pinMode(pin, mode_);
    curPressed = false;
    prevPressed = false;
    longPressInAct = false;
    varSingleClick = false;
    varLongPress = false;
    upTime = 0;
    downTime = 0;
    lastPressTime = 0;
    lastLongPressTime = 0;
  }
  bool singleClick(){
    if (varSingleClick) {
      //Serial.println("single");
      varSingleClick = 0;
      return 1;
    } else return 0;
  }
  bool longPress(){
    if (varLongPress) {
      //Serial.println("long");
      varLongPress = 0;
      return 1;
    } else return 0;
  }
  void update(){
    unsigned long curTime = millis();
    curPressed = !digitalRead(pin);
    if (curPressed and !prevPressed and (curTime - upTime) > debounce) {
      //Button pressed
      downTime = curTime;
      varSingleClick = 1;
      prevPressed = curPressed;
    } else if (!curPressed and prevPressed and (curTime - downTime) > debounce) {
      //Button released
      longPressInAct = 0;
      upTime = curTime;
      prevPressed = curPressed;
    } else if (curPressed and prevPressed and (curTime - downTime) > longPressTimeout) {
      //Button held
      if (!longPressInAct) {
        // Initial long press action
        longPressInAct = 1;
        varLongPress = 1;
        lastLongPressTime = curTime;
        //Line below to make sure that first non-initial long press repeat
        //action is delayed
        curLongPressRepeatTimeout = longPressTimeout;
      } else {
        // Non-initial long press action
        if (curTime - lastLongPressTime > curLongPressRepeatTimeout ) {
          curLongPressRepeatTimeout = longPressRepeatTimeout;
          varLongPress = 1;
          lastLongPressTime = curTime;
        }
      }
    }
  }
};

DS3231_Simple Clock;
fader f;
setHM hm;
button redB(REDBUT, INPUT_PULLUP);
button greB(GREBUT, INPUT_PULLUP);
unsigned long showTimeTimeout = 10000; // Timeout to show time after click
unsigned long showTimeStamp = 0; // millis() of the moment showTimeTemp activated
bool showTimeTemp = false; // True if time should be shown temporarily only
bool showTimeConst = false; // True if time should be shown constantly
bool alarmActive = false; // True if alarm is active
bool setAlarm = false; // True if alarm set mode is active
bool setClock = false; // True if clock set mode is active

void setup() {
  Clock.begin();
  pixels.begin();
  setAllPixels(pixels.Color(0,0,0)); //Turn grid off
  //Serial.begin(9600);
  //Serial.begin(115200);
  //Serial.println("Go!");
  if (!digitalRead(GREBUT)) {
    // Demo mode on green hold at power on
    setAllPixels(pixels.Color(1,1,1));
    delay(300);
    setAllPixels(pixels.Color(0,0,0));
    delay(700);
    alarmActive = true;
    f.enable(2000,1,10,1); // 3s,fadeIn,seq=10,1time
  } else {
    showTimeTemp = true;
    showTimeStamp = millis();
  }
}

void loop() {
  redB.update();
  greB.update();
  if (redB.singleClick()) {
    // Red Click event =============================================
    if (alarmActive){
      //Serial.println("Alarm->Standby");
      alarmActive = false;
      f.disable();
    } else if (setAlarm) {
      //Serial.println("Setting the alarm");
      byte result = hm.nextLine();
      if (result == 1) { // Alarm was set, so enable it
        setAlarm = false;
        DateTime dt = Clock.read();
        hm.getTimestamp(dt);
        Clock.disableAlarms();
        Clock.setAlarm(dt, DS3231_Simple::ALARM_MATCH_MINUTE_HOUR);
        showTimeTemp = true;
        showTimeConst = false;
        showTimeStamp = millis();
      } else if (result == 2) { // Alarm was not set, so go to clock set mode
        setAlarm = false;
        setClock = true;
        DateTime dt = Clock.read();
        hm.enable(dt, pixels.Color(10,0,0),pixels.Color(0,0,10));
      }
    } else if (setClock) {
      //Serial.println("Setting the clock");
      byte result = hm.nextLine();
      if (result == 1) { // Clock was set, so apply
        DateTime dt = Clock.read();
        hm.getTimestamp(dt);
        dt.Second = 0;
        Clock.write(dt);
        setClock = false;
        showTimeTemp = true;
        showTimeConst = false;
        showTimeStamp = millis();
      } else if (result == 2) { // Clock was not set, so go to standby mode
        setClock = false;
      }
    } else if (showTimeConst) {
      //Serial.println("Show time const->Standby");
      showTimeConst = false;
      showTimeTemp = false;
      setAllPixels(pixels.Color(0,0,0));
    } else if (showTimeTemp) {
      //Serial.println("Show time temp->const");
      showTimeTemp = false;
      showTimeConst = true;
    } else {
      //Serial.println("->Show time temp");
      showTimeStamp = millis();
      showTimeConst = false;
      showTimeTemp = true;
    }
  } else if (redB.longPress()) {
    // Red Hold  event =============================================
    if (setAlarm) {
      //Serial.println("setAlarm->Set clock");
      setAlarm = false;
      setClock = true;
      DateTime dt = Clock.read();
      hm.enable(dt, pixels.Color(10,0,0),pixels.Color(0,0,10));
    } else if (setClock) {
      //Serial.println("Set clock->Standby");
      setClock = false;
    } else {
      //Serial.println("->Set alarm");
      setAlarm = true;
      DateTime dt = Clock.read();
      dt.Hour = 5;
      dt.Minute = 0;
      hm.enable(dt, pixels.Color(10,0,0),pixels.Color(0,0,10));
    }
  } else if (greB.singleClick()) {
    // Green Click event ===========================================
    if (alarmActive) {
      //Serial.println("Alarm->Standby");
      alarmActive = false;
      f.disable();
    } else if (setAlarm or setClock) {
      //Serial.println("Increment alarm");
      //Serial.println("Increment clock");
      hm.click();
    } else {
      //Serial.println("->Show time temp");
      showTimeStamp = millis();
      showTimeConst = false;
      showTimeTemp = true;
    }
  } else if (greB.longPress()) {
    // Green Hold event ============================================
    if (setAlarm or setClock) {
      hm.click();
    }
  }
  // No buttons were clicked =====================================
  else if (setAlarm or setClock) {
    // Set mode is active -----------------------------------------
    if (setAlarm) {
      hm.update(setAlarm);
    } else if (setClock) {
      hm.update(setClock);
    }
  } else if (alarmActive) {
    // Alarm1 or alarm2 events ---------------------------------
    byte seq = f.update();
    switch (seq) {
      case  0: break;
      case  1: alarmActive = false;
               break;
      case  2: f.enable(500,3,seq+1,60); //500ms,fadeInOut,,60times
               break;
      case  3: f.enable(100,3,seq+1,300);
               break;
      case 10: f.enable(1000,2,seq+1,1);
               break;
      case 11: f.enable(500,3,seq+1,5);
               break;
      case 12: f.enable(100,3,seq+1,25);
               break;
      default: alarmActive = false;
               setAllPixels(pixels.Color(0,0,0));
               break;
    }
  } else {
    if (millis() % 30000 == 0) { // Check for alarm every 30s
      uint8_t alarmsFired = Clock.checkAlarms();
      if (alarmsFired & 2) {
        alarmActive = true;
        //f.enable(500,3,3,5);
        f.enable(1200000,1,2,1); //20min,fadeIn,seq2,1time
      }
    }
    if (showTimeConst) {
      // Show time constantly --------------------------------------
      if (millis() % 1000 == 0){ // Update grid once a second
        DateTime dt = Clock.read();
        showTimeOnGrid(dt, pixels.Color(1,0,0));
      }
    } else if (showTimeTemp) {
      // Show time temporarily -------------------------------------
      if (millis() - showTimeStamp < showTimeTimeout) {
        DateTime dt = Clock.read();
        showTimeOnGrid(dt, pixels.Color(1,0,0));
      } else {
        showTimeTemp = false;
        setAllPixels(pixels.Color(0,0,0));
      }
    }
  }
}//loop
