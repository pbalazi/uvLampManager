// Main program for uv Lamp Manager
// Created on 2018-07-24 by Petr Balazi - pbalazi@gmai.com

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// definitions

static int LCD_ADDRESS = 0x3F;
static int LCD_LINES = 2;
static int LCD_CHARS = 16;
static int EEPROM_CONFIG_ADDR = 0;

// Structure which stores current status of exact buttons and semaphore that any button is down or not
struct buttons {
  // up button
  bool upArrow = false;
  int upArrowPin = 6;
  // left button
  bool leftArrow = false;
  int leftArrowPin = 8;
  // down button
  bool downArrow = false;
  int downArrowPin = 5;
  // right button
  bool rightArrow = false;
  int rightArrowPin = 7;
  // button with center OK
  bool ok = false;
  int okPin = 4;
  
  // button with uv lamp symbol
  bool uv = false;
  int uvPin = 3;
  // button with light symbol
  bool light = false;
  int lightPin = 2;

  // semaphore that any button is pressed
  bool anyPressed  = false;
};

// structure with setup loaded from persistent ram
struct persistentConfig {
  int cycleDelay = 300;
};

// setup instance of structure
buttons btns;
// setup config object
persistentConfig cfg;
// variable which helps to evaluate buttons
int btnVal = 1;
// setup lcd object
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_LINES); 

int sensorValue;

// detect what buttons are pressed right now
void detectCurrentButtonsStatus() {
  // zero general semaphore
  btns.anyPressed = false;
  // process all buttons
  btnVal = digitalRead(btns.upArrowPin);
  if(btnVal == 0) {
    btns.upArrow = true;
    btns.anyPressed = true;
  }
  else {
    btns.upArrow = false;
  }
  btnVal = digitalRead(btns.leftArrowPin);
  if(btnVal == 0) {
    btns.leftArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.leftArrow = false; 
  }
  btnVal = digitalRead(btns.downArrowPin);
    if(btnVal == 0) {
    btns.downArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.downArrow = false; 
  }
  btnVal = digitalRead(btns.rightArrowPin);
    if(btnVal == 0) {
    btns.rightArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.rightArrow = false; 
  }
  btnVal = digitalRead(btns.okPin);
    if(btnVal == 0) {
    btns.ok = true;
    btns.anyPressed = true;
  }
  else {
   btns.ok = false; 
  }
  btnVal = digitalRead(btns.uvPin);
    if(btnVal == 0) {
    btns.uv = true;
    btns.anyPressed = true;
  }
  else {
   btns.uv = false; 
  }
  btnVal = digitalRead(btns.lightPin);
    if(btnVal == 0) {
    btns.light = true;
    btns.anyPressed = true;
  }
  else {
   btns.light = false; 
  }
}

// print control status of buttons to the output
void printCurrentButtonsStatus(){
  String outStr = "";

  if(btns.upArrow) {outStr = String(outStr + "UP - "); }
  if(btns.leftArrow) {outStr = String(outStr + "LEFT - "); }
  if(btns.downArrow) {outStr = String(outStr + "DOWN - "); }
  if(btns.rightArrow) {outStr = String(outStr + "RIGHT - "); }
  if(btns.ok) {outStr = String(outStr + "OK - "); }
  if(btns.uv) {outStr = String(outStr + "UV - "); }
  if(btns.light) {outStr = String(outStr + "LIGHT - "); }

  if(outStr != "") {
    Serial.println(outStr);
  }
  
}

// show text on display
// if entered string2, than it is expected that we should show it as two separated lines, otherwise split 
// the line into two lines if necessary
void showText(String line1, String line2, String effect){
  lcd.clear();
  // if second line is explicitly
  if(line2 != ""){
    lcd.setCursor ( 0, 0 );
    lcd.print(line1);
    lcd.setCursor ( 0, 1 );
    lcd.print(line2);
  }
  else {
    lcd.setCursor ( 0, 0 );
    lcd.print(line1);
  }
}

void setup() {
  // initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // load config from eeprom
  EEPROM.get(EEPROM_CONFIG_ADDR, cfg);

  // inicialize display
  lcd.init();
  // setup backlight
  lcd.backlight();
  // clear display
  lcd.clear();

  // setup input pins for input mode
  pinMode(btns.upArrowPin,INPUT_PULLUP);
  pinMode(btns.leftArrowPin,INPUT_PULLUP);
  pinMode(btns.downArrowPin,INPUT_PULLUP);
  pinMode(btns.rightArrowPin,INPUT_PULLUP);
  pinMode(btns.okPin,INPUT_PULLUP);
  pinMode(btns.uvPin,INPUT_PULLUP);
  pinMode(btns.lightPin,INPUT_PULLUP);
  digitalWrite(btns.lightPin, HIGH);

}

void loop() {
  // detect status of buttons set
  detectCurrentButtonsStatus();

  // if up button increse the time
  if(btns.upArrow) {
    showText("UP interval",String(cfg.cycleDelay),"");
    cfg.cycleDelay = cfg.cycleDelay + 10;
  }

  // if down button increse the time
  if(btns.downArrow) {
    showText("DOWN interval",String(cfg.cycleDelay),"");
    cfg.cycleDelay = cfg.cycleDelay - 10;
  }
  
  // if uv, than store config
  if(btns.uv) {
      EEPROM.put(EEPROM_CONFIG_ADDR, cfg);
  }
  
  printCurrentButtonsStatus();

  // delay for configured value
  delay(cfg.cycleDelay);
}
