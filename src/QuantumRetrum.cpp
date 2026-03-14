#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

#define LIGHT_SENSOR A0
#define BTN_SET 8
#define BTN_PLUS 6
#define BTN_MINUS 4

bool colonOn = true;
unsigned long lastBlink = 0;

// debounce variables
unsigned long lastSet = 0;
unsigned long lastPlus = 0;
unsigned long lastMinus = 0;
const unsigned long debounceDelay = 200;

bool setMode = false;
uint8_t setStep = 0;

int setHour;
int setMinute;
int setYear;
int setMonth;
int setDay;

// for digits blinking
bool blinkState = true;
unsigned long lastBlinkDigits = 0;

void setBrightness() {

    //Serial.write(0x18); // Change brightness
    //Serial.write(0x10); // Display Off
    //Serial.write(0x18); // Brightness = 1
    //Serial.write(0x14); // Brightness = 2
    //Serial.write(0x1C); // Brightness = 3
    //Serial.write(0x12); // Brightness = 4
    //Serial.write(0x1A); // Display On

    int light = analogRead(LIGHT_SENSOR);
    if ( light < 10 ) {
      Serial.write(0x18);
      Serial.write(0x18);
    }
    if ( light > 50 ) {
      Serial.write(0x18);
      Serial.write(0x12);
    }
    delay(10);
}

void setup() {

  Serial.begin(19200, SERIAL_8O1);
  delay(100);

  Serial.write(0x18);
  Serial.write(0x18);

  //8.8:8.8
  Serial.write(0x08);
  Serial.write(0x7f | 0x80);
  Serial.write(0x7f | 0x80);
  Serial.write(0x7f | 0x80);
  Serial.write(0x7f | 0x80);
  delay(1000);

  pinMode(LIGHT_SENSOR, INPUT);
  pinMode(BTN_SET, INPUT_PULLUP);
  pinMode(BTN_PLUS, INPUT_PULLUP);
  pinMode(BTN_MINUS, INPUT_PULLUP);

  Wire.begin();

  if ( !rtc.begin() ) { while(1); }
  if ( rtc.lostPower() ) { rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); }
}

void tm1652_display_time(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, uint8_t dots) {
  
    uint8_t smgduan[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, // 0 1 2 3 4 5 6 7 8 9
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76, 0x8F, 0x0E, // A b C d E F G H I J
    0x75, 0x38, 0xB7, 0x54, 0x5C, 0x73, 0x67, 0x31, 0xC9, 0x78, // K L M n o P q r S t
    0x3E, 0x1C, 0xFE, 0xE4, 0x6E, 0xDA, 0x40, 0x48, 0x80, 0x00, // U v W X Y Z - = . Null
    0x02, 0x63                                                  // ' *
  };

  Serial.write(0x08);
  Serial.write(smgduan[dig1]);
  Serial.write(smgduan[dig2] | (dots ? 0x80 : 0x00));
  Serial.write(smgduan[dig3]);
  Serial.write(smgduan[dig4]);
}

void loop() {

  setBrightness();
  DateTime now = rtc.now();

  // colon blinking
  if (millis() - lastBlink > 1000) {
    colonOn = !colonOn;
    lastBlink = millis();
  }

  // digits blinking
  if (millis() - lastBlinkDigits > 500) {
    blinkState = !blinkState;
    lastBlinkDigits = millis();
  }

  // button "SET"
  if (digitalRead(BTN_SET) == LOW && millis() - lastSet > debounceDelay) {

    lastSet = millis();

    if ( !setMode ) {

      setMode = true;
      setStep = 0;

      setHour = now.hour();
      setMinute = now.minute();
      setYear = now.year();
      setMonth = now.month();
      setDay = now.day();

    } else {

      setStep++;

      if (setStep > 4) {

        rtc.adjust(DateTime(
          setYear,
          setMonth,
          setDay,
          setHour,
          setMinute,
          0
        ));

        setMode = false;
      }
    }
  }

  // buttons "+" and "-" for time setting
  if (setMode) {

    if (digitalRead(BTN_PLUS) == LOW && millis() - lastPlus > debounceDelay) {

      lastPlus = millis();

      if (setStep == 0) {
        setHour++;
        if (setHour > 23) { setHour = 0; }
      }

      if(setStep == 1) {
        setMinute++;
        if(setMinute > 59) { setMinute = 0; }
      }

      if(setStep == 2) {
        setYear++;
        if(setYear > 3000) { setYear = 2000; }
      }

      if(setStep == 3) {
        setMonth++;
        if(setMonth > 12) { setMonth = 1; }
      }

      if(setStep == 4) {
        setDay++;
        if(setDay > 31) { setDay = 1; }
      }

    }

    if (digitalRead(BTN_MINUS) == LOW && millis() - lastMinus > debounceDelay) {

      lastMinus = millis();

      if ( setStep == 0 ) {
        if ( setHour == 0 ) { setHour = 23; }
        else { setHour--; }
      }

      if ( setStep == 1 ) {
        if ( setMinute == 0 ) { setMinute = 59; }
        else { setMinute--; }
      }

      if ( setStep == 2 ) {
        setYear--;
        if ( setYear < 2000 ) { setYear = 3000; }
      }

      if ( setStep == 3 ) {
        setMonth--;
        if ( setMonth < 1 ) { setMonth = 12; }
      }

      if ( setStep == 4 ) {
        setDay--;
        if ( setDay < 1 ) { setDay = 31; }
      }

    }

    if ( setStep == 0 || setStep == 1 ){
      uint8_t d0 = setHour / 10;
      uint8_t d1 = setHour % 10;
      uint8_t d2 = setMinute / 10;
      uint8_t d3 = setMinute % 10;

      // Blinking for hours
      if ( setStep == 0 && !blinkState ) {
        d0 = 39;
        d1 = 39;
      }
      // Blinking for minutes
      if ( setStep == 1 && !blinkState ) {
        d2 = 39;
        d3 = 39;
      }

      tm1652_display_time(d0,d1,d2,d3,1);
      delay(50);
      return;
    }
    

    if ( setStep == 2 ) {
      uint8_t d0 = (setYear / 1000) % 10;
      uint8_t d1 = (setYear / 100) % 10;
      uint8_t d2 = (setYear / 10) % 10;
      uint8_t d3 = setYear % 10;

      if ( !blinkState ) {
        d0 = 39;
        d1 = 39;
        d2 = 39;
        d3 = 39;
      }

      tm1652_display_time(d0,d1,d2,d3,0);
      delay(50);
      return;
    }

    if ( setStep == 3 ) {
      uint8_t d0 = 39;
      uint8_t d1 = setMonth / 10;
      uint8_t d2 = setMonth % 10;
      uint8_t d3 = 39;

      if ( !blinkState ) {
        d1 = 39;
        d2 = 39;
      }

      tm1652_display_time(d0,d1,d2,d3,0);
      delay(50);
      return;
    }

    if ( setStep == 4 ) {
      uint8_t d0 = 39;
      uint8_t d1 = setDay / 10;
      uint8_t d2 = setDay % 10;
      uint8_t d3 = 39;

      if ( !blinkState ) {
        d1 = 39;
        d2 = 39;
      }

      tm1652_display_time(d0,d1,d2,d3,0);
      delay(50);
      return;
    }
    
  }

  // button "+" for date display
  if (digitalRead(BTN_PLUS) == LOW && millis() - lastPlus > debounceDelay) {

    lastPlus = millis();

    uint8_t d0 = now.year() / 1000;
    uint8_t d1 = now.year() % 1000 / 100;
    uint8_t d2 = now.year() % 100 / 10;
    uint8_t d3 = now.year() % 10;

    tm1652_display_time(d0,d1,d2,d3,0);
    delay(1000);

    d0 = 39;
    d1 = now.month() / 10;
    d2 = now.month() % 10;
    d3 = 39;

    tm1652_display_time(d0,d1,d2,d3,0);
    delay(1000);

    d0 = 39;
    d1 = now.day() / 10;
    d2 = now.day() % 10;
    d3 = 39;

    tm1652_display_time(d0,d1,d2,d3,0);
    delay(1000);
  }

  // button "-" for temperature display
  if (digitalRead(BTN_MINUS) == LOW && millis() - lastMinus > debounceDelay) {

    lastMinus = millis();
    int temp = round(rtc.getTemperature());
    uint8_t d0 = temp / 10;
    uint8_t d1 = temp % 10;
    uint8_t d2 = 41;
    uint8_t d3 = 12;

    tm1652_display_time(d0,d1,d2,d3,0);
    delay(1000);
  }

  uint8_t d0 = now.hour() / 10;
  uint8_t d1 = now.hour() % 10;
  uint8_t d2 = now.minute() / 10;
  uint8_t d3 = now.minute() % 10;

  tm1652_display_time(d0,d1,d2,d3,colonOn);
  delay(200);
}