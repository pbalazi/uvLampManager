// Main program for uv Lamp Manager
// Created on 2018-07-24 by Petr Balazi - pbalazi@gmai.com

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <DS3231.h>

// definitions

static int LCD_ADDRESS = 0x3F;
static int LCD_LINES = 2;
static int LCD_CHARS = 16;
static int EEPROM_CONFIG_ADDR = 0;
static int RELAY1 = 9;
static int RELAY2 = 10;
static int RELAY3 = 11;

static String msg_en[15] = {"Lamp is ready","Options ->","-> Language","","","","","","","","","","","",""};
static String msg_de[15] = {"","","","","","","","","","","","","","",""};
static String msg_cz[15] = {"Lampa pripravena","Nastaveni ->","-> Jazyk","","","","","","","","","","","",""};
static String msg_fr[15] = {"","","","","","","","","","","","","","",""};

static int blockMenuLimit = 400;

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
  // button which detect closing of the lit
  bool litClosed = false;
  int litClosedPin = 12;
  
  // button with uv lamp symbol
  bool uv = false;
  int uvPin = 3;
  // button with light symbol
  bool light = false;
  int lightPin = 2;

  // semaphore that any button is pressed
  bool anyPressed  = false;
};

// structure covering relays
struct relay {
  int lightPin = 0;
  bool lightOn = false;
};

// structure with setup loaded from persistent ram
struct persistentConfig {
  // general delay in each cycle
  int cycleDelay = 100;
  // max number of seconds which could be uv lamp on - security limit
  int maxUvTime = 600;
  // language default
  char curLang[3] = "en";
};

// setup instance of structure
buttons btns;
// setup instance of relays
relay r1;
relay r2;
relay r3;

// setup config object
persistentConfig cfg;
// variable which helps to evaluate buttons
int btnVal = 1;
// setup lcd object
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_LINES); 
// setup rtc ds3231 object
DS3231 rtc;
// create variable for rtc
RTCDateTime dateTime;
// block menu for a second to avoid too fast switching of menu
int blockMenuCur = 0;

// -- global variables
// start unix time when uv has been light on
long uvStartTime = -1;
// start unix time when light has been light on
long lightStartTime = -1;
// string which remembers position in menu
String curMenuPos = "main";
// force to refresh lcd in next cycle
bool forceRefreshLcd = false;

int sensorValue;

// detect what buttons are pressed right now
void detectCurrentButtonsStatus() {
  // zero general semaphore
  btns.anyPressed = false;
  // process all buttons
  btnVal = digitalRead(btns.upArrowPin);
  if(btnVal == LOW) {
    btns.upArrow = true;
    btns.anyPressed = true;
  }
  else {
    btns.upArrow = false;
  }
  btnVal = digitalRead(btns.leftArrowPin);
  if(btnVal == LOW) {
    btns.leftArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.leftArrow = false; 
  }
  btnVal = digitalRead(btns.downArrowPin);
    if(btnVal == LOW) {
    btns.downArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.downArrow = false; 
  }
  btnVal = digitalRead(btns.rightArrowPin);
    if(btnVal == LOW) {
    btns.rightArrow = true;
    btns.anyPressed = true;
  }
  else {
   btns.rightArrow = false; 
  }
  btnVal = digitalRead(btns.okPin);
    if(btnVal == LOW) {
    btns.ok = true;
    btns.anyPressed = true;
  }
  else {
   btns.ok = false; 
  }
  btnVal = digitalRead(btns.uvPin);
    if(btnVal == LOW) {
    btns.uv = true;
    btns.anyPressed = true;
  }
  else {
   btns.uv = false; 
  }
  btnVal = digitalRead(btns.lightPin);
    if(btnVal == LOW) {
    btns.light = true;
    btns.anyPressed = true;
  }
  else {
   btns.light = false; 
  }
  btnVal = digitalRead(btns.litClosedPin);
    if(btnVal == LOW) {
    btns.litClosed = true;
    btns.anyPressed = false; // this is special input, not consider as standard button
  }
  else {
   btns.litClosed = false; 
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

// setup initals for relays
void setupRelays() {
  r1.lightPin = RELAY1;
  pinMode(r1.lightPin, OUTPUT);
  digitalWrite(r1.lightPin, HIGH);  
  r2.lightPin = RELAY2;
  pinMode(r2.lightPin, OUTPUT);
  digitalWrite(r2.lightPin, HIGH);
  r3.lightPin = RELAY3;
  pinMode(r3.lightPin, OUTPUT);  
  digitalWrite(r3.lightPin, HIGH);
}

// up/ down relay, specific which relay and put 'up' or 'down' word as action
// third param is boolen and if true, you have to add also fourth argument which is current status of lit button (also bool)
// return value: 0=ok, -1=unknown action, -2=action up, but lit is not closed
int setRelay(struct relay *rel, String action, bool checkLitButton = false, int litButtonStatus = false) {
  int retVal = 0;
  Serial.println("DEBUG:Relay - Status: " + String(rel->lightOn));
  // process through condition tree
  if(action == "up") {
    if(checkLitButton) {
      // power up the relay if lit button is pressed - lit cover closed
      if (litButtonStatus) {
        // do it
        digitalWrite(rel->lightPin, LOW);
        rel->lightOn = true;
        Serial.println("DEBUG:Relay " + String(rel->lightPin) + " check lit and up.");
      }
      else {
        retVal = -2;
        Serial.println("DEBUG:Relay error - lit is NOT OK.");
      }
    }
    else {
      // do it - no lit check required
      digitalWrite(rel->lightPin, LOW);
      rel->lightOn = true;
      Serial.println("DEBUG:Relay " + String(rel->lightPin) + " up - no lit check:LightOncheck:" + String(rel->lightOn));
    }
  }
  else if (action == "down") {
    // power off it
    digitalWrite(rel->lightPin, HIGH);
    rel->lightOn = false;
    Serial.println("DEBUG:Relay " + String(rel->lightPin) + " down.");
  }
  else {
    retVal = -1;
    Serial.println("DEBUG:Relay - unknown command.");
  }
  // return proper value
  return retVal;
}

// show text on display
// if entered string2, than it is expected that we should show it as two separated lines, otherwise split 
// the line into two lines if necessary
void showText(String line1, String line2, String effect = ""){
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

  // setup initial display screen
  Serial.print("Temperature: ");
  Serial.println(rtc.readTemperature());
}

// -- dispaly process functions - counting with 2 lines and 16 chars
// print message in proper language
String mLoc(int strId) {
  String retStr = "";
  Serial.println("DEBUG:mLoc");
  Serial.print("DEBUG:mLoc:curlang="); Serial.println(cfg.curLang);
  // current language is in cfg.curLang
  if(strcmp(cfg.curLang,"de") == 0) {
    Serial.print("DEBUG:mLoc:de:");Serial.println(strId);
    retStr = msg_de[strId];
  }
  else if(strcmp(cfg.curLang,"cz") == 0) {
    Serial.print("DEBUG:mLoc:cz:");Serial.println(strId);
    retStr = msg_cz[strId];
  }
  else if(strcmp(cfg.curLang,"fr") == 0) {
    Serial.print("DEBUG:mLoc:fr:");Serial.println(strId);
    retStr = msg_fr[strId];
  }
  else { // en is default
    Serial.print("DEBUG:mLoc:en:");Serial.println(strId);
    retStr = msg_en[strId];  
  }
  // return localized string
  return retStr;
}
// print status screen
void printStatus() {
  String line1 = "";
  String line2 = "";
  line1 = mLoc(0);
  line2 = "          " + String(rtc.readTemperature());
  // show on display
  showText(line1, line2);
}
// -- end display process

// -- block menu
// test if menu is blocked
bool isMenuBlocked() {
  if(blockMenuCur > 0) {
    return true;
  }
  else {
    return false;
  }
}
// reset counter to max
void setMenuBlock() {
  blockMenuCur = blockMenuLimit;
}

// process one interval to unblock
void doMenuUnBlock() {
  if(blockMenuCur > 0) {
    blockMenuCur = blockMenuCur - cfg.cycleDelay;
  }
}
// -- end block menu


// -- EEPROm functions
void storeConfig(){
  EEPROM.put(EEPROM_CONFIG_ADDR, cfg);
}
// -- ENd EEPROm functions

void setup() {
  // initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start rtc
  rtc.begin();
  // one time setup
  // rtc.setDateTime(__DATE__, __TIME__);

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
  pinMode(btns.litClosedPin,INPUT_PULLUP);

  // setup relays
  setupRelays();

  // print status page
  printStatus();
  curMenuPos = "main";
  
}

void loop() {
  // detect status of buttons set
  detectCurrentButtonsStatus();

  // -- MAIN menu --
  // if se are in setup menu and would like to go back to main
  if(curMenuPos == "main-option" && btns.leftArrow && !isMenuBlocked()) {
    printStatus();
    curMenuPos = "main";
    setMenuBlock();
  }
  
  // -- END MAIN menu --
  
  // -- OPTION menu --
  // if we would like to go into setup
  if(curMenuPos == "main" && btns.rightArrow && !isMenuBlocked()) {
    showText(mLoc(1), "");
    curMenuPos = "main-option";
    setMenuBlock();
  }
  // if se are in setup menu and would like to go back to main
  if(curMenuPos == "main-option-language" && btns.leftArrow && !isMenuBlocked()) {
    showText(mLoc(1), "");
    curMenuPos = "main-option";
    setMenuBlock();
  }
  // -- END OPTION menu --

  // -- LANGUAGE menu --
  if(curMenuPos == "main-option" && btns.rightArrow && !isMenuBlocked()) {
    showText(mLoc(2), "            ...");
    curMenuPos = "main-option-language";
    setMenuBlock();
  }

  if(curMenuPos == "main-option-language" && forceRefreshLcd) {
    Serial.print("DEBUG:Pressed OK:Menu go to Language");Serial.println(curMenuPos);
    showText(mLoc(2), "            ...");
    curMenuPos = "main-option-language";
    setMenuBlock();
    forceRefreshLcd = false;
  }
  
  if(curMenuPos == "main-option-language" && btns.upArrow && !isMenuBlocked()) {
    showText(mLoc(2), "             EN");
    curMenuPos = "main-option-language-en";
    setMenuBlock();
  }
  if(curMenuPos == "main-option-language-en" && btns.upArrow && !isMenuBlocked()) {
    showText(mLoc(2), "             DE");
    curMenuPos = "main-option-language-de";
    setMenuBlock();
  }
  if(curMenuPos == "main-option-language-de" && btns.upArrow && !isMenuBlocked()) {
    showText(mLoc(2), "             CZ");
    curMenuPos = "main-option-language-cz";
    setMenuBlock();
  }
  if(curMenuPos == "main-option-language-cz" && btns.upArrow && !isMenuBlocked()) {
    showText(mLoc(2), "             FR");
    curMenuPos = "main-option-language-fr";
    setMenuBlock();
  }
  if(curMenuPos == "main-option-language-fr" && btns.upArrow && !isMenuBlocked()) {
    showText(mLoc(2), "            ...");
    curMenuPos = "main-option-language";
    setMenuBlock();
  }
  // -- END LANGUAGE menu --
  if(btns.ok) {
    // menu handlers
      if(curMenuPos == "main-option-language-en") {
        strcpy(cfg.curLang,"en");
        storeConfig();
        curMenuPos = "main-option-language";
      }
      else if(curMenuPos == "main-option-language-de") {
        strcpy(cfg.curLang,"de");
        storeConfig();
        curMenuPos = "main-option-language";
      }
      else if(curMenuPos == "main-option-language-cz") {
        strcpy(cfg.curLang,"cz");
        storeConfig();
        curMenuPos = "main-option-language";
      }
      else if(curMenuPos == "main-option-language-fr") {
        strcpy(cfg.curLang,"fr");
        storeConfig();
        curMenuPos = "main-option-language";
      }
    // end menu handlers
    forceRefreshLcd  = true;
  }
  // -- OK actions --
  
  // -- END OK actions --
  // debug print current buttons status
  printCurrentButtonsStatus();


  // -- UV button

  // -- END UV button

  // -- LIGHT button
  if(btns.light) {
    Serial.print("DEBUG:relay:lightOn:"); Serial.println(r1.lightOn);
    if (r1.lightOn == 0) {
      setRelay(&r2, "up");
    }
    if (r1.lightOn == 1) {
      Serial.print("DEBUG:relay:lightOn:goDown"); Serial.println(r1.lightOn);
      setRelay(&r2, "down");
    }
  }
  // -- END LIGHT button
  // security limit for tim e when uv lamp may be in progress
  if(uvStartTime) {}

  // delay for configured value
  delay(cfg.cycleDelay);
  doMenuUnBlock();
}

// debug - all code which were in phase debug, but is not used now.
void noopfunc(){

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
      dateTime = rtc.getDateTime();
      
      uvStartTime = dateTime.unixtime;
      // test
      Serial.print("Datum a cas DS3231: "); 
      Serial.println(dateTime.unixtime);
      Serial.print(dateTime.year);   Serial.print("-");
      Serial.print(dateTime.month);  Serial.print("-");
      Serial.print(dateTime.day);    Serial.print(" ");
      Serial.print(dateTime.hour);   Serial.print(":");
      Serial.print(dateTime.minute); Serial.print(":");
      Serial.print(dateTime.second); Serial.println("");
  }

  // if press ok, detect status of lit
  if(btns.ok) {
    showText("Is lit closed?",String(btns.litClosed),"");
  }

  // if press light, poweron relay
  if(btns.light) {
    if (r1.lightOn == false) {
      setRelay(&r1, "up", true, btns.litClosed);
      setRelay(&r2, "up", true, btns.litClosed);
    }
  }
  else {
    // if relay is up, then get it down
    if(r1.lightOn) {
      setRelay(&r1, "down");
      setRelay(&r2, "down");
    }
  }
}

