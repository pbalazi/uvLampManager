
// Main program for uv Lamp Manager
// Created on 2018-07-24 by Petr Balazi - pbalazi@gmai.com

#include <Wire.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <JC_Button.h>              // https://github.com/JChristensen/JC_Button
#include "uvLampConfig.h"           // project variables config library
//#include <DS3231.h>
#include "moje_DS3231.h"


// define buttons available
Button btnUp(UP_PIN), btnDown(DOWN_PIN), btnLeft(LEFT_PIN), btnRight(RIGHT_PIN), btnOk(OK_PIN), btnLight(LIGHT_PIN), btnUv(UV_PIN), btnLit(LIT_PIN);    // define the buttons

// setup instance of relays
relay r1;
relay r2;
relay r3;

// setup config object
persistentConfig cfg;


// setup rtc ds3231 object
DS3231 rtc;
// create variable for rtc
RTCDateTime dateTime;

// setup lcd object
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_LINES); 

// global information about menu depth
int menuDepth = 0;
unsigned long lastDateTime = 0;




#include "framesLib.h"






Frame* frames[32];

void setup()
{

    // inicialize serial console
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }


    setLanguage(0);
    Serial.println((int) cfg.curLang);
    Serial.println(sizeof(Frame));
    

    initializeFrames(frames);
    
    
    // variables
    char buffer[LCD_CHARS];

    // enable beep pin to write
    pinMode(BEEP_PIN, OUTPUT);
    
    // inicialize buttons
    btnUp.begin();
    btnDown.begin();
    btnLeft.begin();
    btnRight.begin();
    btnOk.begin();
    btnLight.begin();
    btnUv.begin();
    btnLit.begin();

    
    
    // start rtc
    rtc.begin();
    // one time setup
    //rtc.setDateTime(__DATE__, __TIME__);

    // load config from eeprom
    //EEPROM.get(EEPROM_CONFIG_ADDR, cfg);

    
    // inicialize display
    lcd.init();
    // setup backlight
    lcd.backlight();
    // clear display
    lcd.clear(); 
    // Creates the byte for the 3 custom characters
    lcd.createChar(0, downArrow);
    lcd.createChar(1, upArrow);
    lcd.createChar(2, menuCursor);
    lcd.createChar(3, upDownArrow);

    cfg.curFrame = 0;
}

void loop()
{
    static unsigned long rpt(REPEAT_FIRST);              // a variable time that is used to drive the repeats for long presses
    //char buffer[LCD_CHARS + 1];                                     // buffer for dispaly text
    int lastFrame = cfg.curFrame;

    dateTime = rtc.getDateTime();

    if (cfg.curFrame > 2 && (dateTime.unixtime - lastDateTime) > 30) {
      cfg.curFrame = 2;
    }

    Frame* frame = frames[cfg.curFrame];

    //Serial.println(cfg.curFrame);

    if (frame->beforePrint != NULL) {
      (frame->beforePrint)(frame);
    }

    if (cfg.reqToPrint) {
      printFrame(frame);
      cfg.reqToPrint = false;
    }

    btnUp.read();
    btnDown.read();
    btnLeft.read();
    btnRight.read();
    btnOk.read();
    btnLight.read();
    btnUv.read();
    btnLit.read();
    
    if (btnOk.wasPressed() && frame->btnOK != NULL) {
      Serial.println("OK pressed.");
      (frame->btnOK)(frame);
    }

    if (btnUp.wasPressed() && frame->btnUp != NULL) {
      Serial.println("Up pressed.");
      (frame->btnUp)(frame);
    }

    if (btnDown.wasPressed() && frame->btnDown != NULL) {
      Serial.println("Down pressed.");
      (frame->btnDown)(frame);
    }

    if (btnLeft.wasPressed() && frame->btnLeft != NULL) {
      Serial.println("Left pressed.");
      (frame->btnLeft)(frame);
    }

    if (btnRight.wasPressed() && frame->btnRight != NULL) {
      Serial.println("Right pressed.");
      (frame->btnRight)(frame);
    }

    if (btnLight.wasPressed() && frame->btnLight != NULL) {
      Serial.println("Light pressed.");
      (frame->btnLight)(frame);
    }

    if (btnUv.wasPressed() && frame->btnUV != NULL) {
      Serial.println("UV pressed.");
      (frame->btnUV)(frame);
    }

    if (btnLit.wasPressed() && frame->btnLit != NULL) {
      Serial.println("Lit pressed.");
      (frame->btnLit)(frame);
    }

    if (frame->afterPrint != NULL) {
      (frame->afterPrint)(frame);
    }

    if (btnOk.wasPressed() || btnUp.wasPressed() || btnDown.wasPressed() || btnLeft.wasPressed() || btnRight.wasPressed() || btnLight.wasPressed() || btnUv.wasPressed()) {
      lastDateTime = dateTime.unixtime;
      cfg.reqToPrint = true;

      if (lastFrame != cfg.curFrame) {
        tone(BEEP_PIN, BEEP_TONE, BEEP_LENGTH);
      }
    }

    delay(100);

    if (lastFrame != cfg.curFrame) {
      lcd.clear();
      cfg.reqToPrint = true;
    }
    
    /*

    // show initial screen while we are on zero level of menu
    if (menuDepth == 0) {
        char outDate[17];
        char outTime[17];
        
        // show date and time
        dateTime = rtc.getDateTime();

        sprintf(outTime, "%02d:%02d:%02d", dateTime.hour, dateTime.minute, dateTime.second);
        sprintf(outDate, "%02d.%02d.%d", dateTime.day, dateTime.month, dateTime.year);
        
        //String outDate = String(dateTime.day) + "." + String(dateTime.month) + "." + String(dateTime.year);
        //String outTime = String(ahour) + ":" + String(aminute) + ":" + String(asecond);
        if(lastDateTime < dateTime.unixtime) {
            //lcd.clear(); 
        }
    // produce first line of output
        Serial.println(outTime);
        lcd.setCursor ( 0, 0 );
        lcd.print(outTime);
    // produce second line of output
        Serial.println(outDate);
        lcd.setCursor ( 0, 1 );
        lcd.print(outDate);
        
        cfg.curFrame = 2;
        Frame* ee = frames[cfg.curFrame];
        ///Serial.println((int) ee->getType());
        //printFrame(*(frames[cfg.curFrame]));
        
        lastDateTime = dateTime.unixtime;
    }

    // read current status of buttons
    btnUp.read();
    btnDown.read();
    btnLeft.read();
    btnRight.read();
    btnOk.read();
    btnLight.read();
    btnUv.read();
    btnLit.read();

    // check if button has been just pressed
    if(btnOk.wasPressed()){
        if(menuDepth == 0) {
            //beep(BEEP_LENGTH);
            tone(BEEP_PIN, BEEP_TONE, BEEP_LENGTH);
            Serial.println("Up pressed.");
            lcd.clear();
            // produce first line of output
           // printFrame(*(frames[2]));
            
            lcd.setCursor ( LCD_CHARS - 1, 0 );
            lcd.write(byte(1)); 
            // produce second line of output
            lcd.setCursor ( LCD_CHARS - 1, 1 );
            lcd.write(byte(0));
            
            // increase dept of current menu 
            menuDepth = 1;
        }
        else {
            menuDepth = 0;
        }
    }

    delay(250);
    /*
     
    else if(btnUp.pressedFor(rpt)){
        Serial.println("Up still pressed.");
        rpt += REPEAT_INCR;
    }
    */


    
}
