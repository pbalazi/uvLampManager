// Main program for uv Lamp Manager
// Created on 2018-07-24 by Petr Balazi - pbalazi@gmai.com

#include <Wire.h>
#include <EEPROM.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <JC_Button.h>              // https://github.com/JChristensen/JC_Button
#include "uvLampConfig.h"           // project variables config library

// define buttons available
Button btnUp(UP_PIN), btnDown(DOWN_PIN), btnLeft(LEFT_PIN), btnRight(RIGHT_PIN), btnOk(OK_PIN), btnLight(LIGHT_PIN), btnUv(UV_PIN), btnLit(LIT_PIN);    // define the buttons

// setup instance of relays
relay r1;
relay r2;
relay r3;

// setup config object
persistentConfig cfg;

// setup lcd object
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_LINES); 
// setup rtc ds3231 object
DS3231 rtc;
// create variable for rtc
RTCDateTime dateTime;
// global information about menu depth
int menuDepth = 0;
unsigned long lastDateTime = 0;

void beep(unsigned char delayms){
  analogWrite(BEEP_PIN, BEEP_TONE);      // Almost any value can be used except 0 and 255
                           // experiment to get the best tone
  delay(delayms);          // wait for a delayms ms
  analogWrite(BEEP_PIN, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}  

void setup()
{
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

    // inicialize serial console
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

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

    // inital sequence on display
    // produce first line of output
        strcpy_P(buffer, (char*)pgm_read_word(&(t0011[cfg.curLang])));
        Serial.println(buffer);
        lcd.setCursor ( 0, 0 );
        lcd.print(buffer);
    // produce second line of output
        strcpy_P(buffer, (char*)pgm_read_word(&(t0012[cfg.curLang])));
        Serial.println(buffer);
        lcd.setCursor ( 0, 1 );
        lcd.print(buffer);
    delay(INITIAL_DELAY);
    lcd.clear(); 
    // produce first line of output
        strcpy_P(buffer, (char*)pgm_read_word(&(t0021[cfg.curLang])));
        Serial.println(buffer);
        lcd.setCursor ( 0, 0 );
        lcd.print(buffer);
    // produce second line of output
        strcpy_P(buffer, (char*)pgm_read_word(&(t0022[cfg.curLang])));
        Serial.println(buffer);
        lcd.setCursor ( 0, 1 );
        lcd.print(buffer);
    delay(INITIAL_DELAY); 
    lcd.clear(); 
}

void loop()
{
    static unsigned long rpt(REPEAT_FIRST);              // a variable time that is used to drive the repeats for long presses
    char buffer[LCD_CHARS];                                     // buffer for dispaly text

    // show initial screen while we are on zero level of menu
    if(menuDepth == 0){
        // show date and time
        dateTime = rtc.getDateTime(); 
        String outDate = String(dateTime.day) + "." + String(dateTime.month) + "." + String(dateTime.year);
        String outTime = String(dateTime.hour) + ":" + String(dateTime.minute) + ":" + String(dateTime.second);
        if(lastDateTime < dateTime.unixtime) {
            lcd.clear(); 
        }
    // produce first line of output
        Serial.println(outTime);
        lcd.setCursor ( 0, 0 );
        lcd.print(outTime);
    // produce second line of output
        Serial.println(outDate);
        lcd.setCursor ( 0, 1 );
        lcd.print(outDate);
        
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
            strcpy_P(buffer, (char*)pgm_read_word(&(t0031[cfg.curLang])));
            Serial.println(buffer);
            lcd.setCursor ( 0, 0 );
            lcd.print(buffer);
            lcd.setCursor ( LCD_CHARS - 1, 0 );
            lcd.write(byte(1)); 
            // produce second line of output
            strcpy_P(buffer, (char*)pgm_read_word(&(t0032[cfg.curLang])));
            Serial.println(buffer);
            lcd.setCursor ( 0, 1 );
            lcd.print(buffer);
            lcd.setCursor ( LCD_CHARS - 1, 1 );
            lcd.write(byte(0));
    
            // increase dept of current menu 
            menuDepth = 1;
        }
        else {
            menuDepth = 0;
        }
    }
    /*
     
    else if(btnUp.pressedFor(rpt)){
        Serial.println("Up still pressed.");
        rpt += REPEAT_INCR;
    }
    */


    
}
