
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


// setup rtc ds3231 object
DS3231 rtc;
// create variable for rtc
RTCDateTime dateTime;

// setup lcd object
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_LINES); 

// global information about menu depth
int menuDepth = 0;
unsigned long lastDateTime = 0;


typedef struct Frame_ {
  byte type = 0;
  char* topLine = NULL;
  char* bottomLine = NULL;
  byte flags = 0b00000000;
  void (*beforePrint)(Frame_*) = NULL;
  void (*afterPrint)(Frame_*) = NULL;
  void (*btnOK)(Frame_*) = NULL;
  void (*btnUp)(Frame_*) = NULL;
  void (*btnDown)(Frame_*) = NULL;
  void (*btnLeft)(Frame_*) = NULL;
  void (*btnRight)(Frame_*) = NULL;
  void (*btnUV)(Frame_*) = NULL;
  void (*btnLight)(Frame_*) = NULL;
  void (*btnLit)(Frame_*) = NULL;
  int topLineN = 0;
  int bottomLineN = 0;
} Frame;



/*
class DisplayFrame {
  const char* topLine;
  const char* bottomLine;
  char type = 0;

public:
  DisplayFrame() {}
  DisplayFrame(const char* topLine, const char* bottomLine);
  ~DisplayFrame() {}

  char getType() {
    return this->type;
  }
  
  void btnOK();
  void btnTop();
  void btnBottom();
  void btnLeft();
  void btnRight();
  void btnUV();
  void btnLight();

  const char* getTopLine();
  const char* getBottomLine();
};

DisplayFrame::DisplayFrame(const char* topLine, const char* bottomLine) {
  this->topLine = topLine;
  this->bottomLine = bottomLine;
}


const char* DisplayFrame::getTopLine() {
  return this->topLine;
}

const char* DisplayFrame::getBottomLine() {
  return this->bottomLine;
}


class TimeFrame : public DisplayFrame {
  char topLine[17];
  char bottomLine[17];
  String outTime;
  String outDate;
  char type = 1;

public:
  TimeFrame() {}
  ~TimeFrame() {}

  char getType() {
    return this->type;
  }
  
  const char* getTopLine();
  const char* getBottomLine();
};


const char* TimeFrame::getTopLine() {
  dateTime = rtc.getDateTime();

  char ahour[3];
  char aminute[3];
  char asecond[3];

  sprintf(this->topLine, "%02d:%02d:%02d", dateTime.hour, dateTime.minute, dateTime.second);
  Serial.println(this->topLine);
        
  //this->outTime = String(ahour) + ":" + String(aminute) + ":" + String(asecond);

  return this->topLine;
}

const char* TimeFrame::getBottomLine() {
  dateTime = rtc.getDateTime();
  
  sprintf(this->bottomLine, "%02d.%02d.%0d", dateTime.day, dateTime.month, dateTime.year);
  //this->outDate = String(dateTime.day) + "." + String(dateTime.month) + "." + String(dateTime.year);
  
  return this->bottomLine;
}*/


void setLanguage(byte lang) {
  if (lang == (byte) 0) {
    cfg.curLang = (char**) czech;
  }
  else if (lang == (byte) 1) {
    cfg.curLang = (char**) english;
  }
}


void printFrame(Frame* frame) {
  char buffer[LCD_CHARS + 1];
  
  // produce first line of output
  if (frame->type == FT_PROGMEM_PROGMEM) {
    strcpy_P(buffer, frame->topLine);
  }
  else if (frame->type == FT_SRAM_SRAM) {
    strcpy(buffer, frame->topLine);
  }
  else if (frame->type == FT_PROGMEM_SRAM) {
    strcpy_P(buffer, frame->topLine);
  }
  else if (frame->type == FT_PROGMEM_PROGMEM_2) {
    strcpy_P(buffer, (char*) pgm_read_word(&(cfg.curLang[frame->topLineN])));
  }
  else if (frame->type == FT_PROGMEM_SRAM_2) {
    strcpy_P(buffer, (char*) pgm_read_word(&(cfg.curLang[frame->topLineN])));
  }
          
  Serial.println(buffer);
  lcd.setCursor ( 0, 0 );
  lcd.print(buffer);

  // produce second line of output
  if (frame->type == FT_PROGMEM_PROGMEM) {
    strcpy_P(buffer, frame->bottomLine);
  }
  else if (frame->type == FT_SRAM_SRAM) {
    strcpy(buffer, frame->bottomLine);
  }
  else if (frame->type == FT_PROGMEM_SRAM) {
    strcpy(buffer, frame->bottomLine);
  }
  else if (frame->type == FT_PROGMEM_PROGMEM_2) {
    strcpy_P(buffer, (char*) pgm_read_word(&(cfg.curLang[frame->bottomLineN])));
  }
  else if (frame->type == FT_PROGMEM_SRAM_2) {
    strcpy(buffer, frame->bottomLine);
  }
  
  Serial.println(buffer);
  lcd.setCursor ( 0, 1 );
  lcd.print(buffer);

  if ((frame->flags & FRAME_FLAG_TL_ARROW) == FRAME_FLAG_TL_ARROW) {
    lcd.setCursor ( LCD_CHARS - 1, 0 );
    lcd.write(byte(1)); 
  }

  if ((frame->flags & FRAME_FLAG_BL_ARROW) == FRAME_FLAG_BL_ARROW) {
    lcd.setCursor ( LCD_CHARS - 1, 1 );
    lcd.write(byte(0));
  }

  if ((frame->flags & FRAME_FLAG_BL_UD_ARROW) == FRAME_FLAG_BL_UD_ARROW) {
    lcd.setCursor ( LCD_CHARS - 1, 1 );
    lcd.write(byte(3));
  }
}


void beep(unsigned char delayms){
  analogWrite(BEEP_PIN, BEEP_TONE);      // Almost any value can be used except 0 and 255
                           // experiment to get the best tone
  delay(delayms);          // wait for a delayms ms
  analogWrite(BEEP_PIN, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}  

void startWait(Frame* frame) {
  delay(5000);
  cfg.curFrame++;
  cfg.reqToPrint = true;
}

void getDatetimeForPrint(Frame* frame) {
  dateTime = rtc.getDateTime();

  sprintf(frame->topLine, "%02d:%02d:%02d", dateTime.hour, dateTime.minute, dateTime.second);
  sprintf(frame->bottomLine, "%02d.%02d.%0d", dateTime.day, dateTime.month, dateTime.year);

  cfg.reqToPrint = true;
}

void datetimeFrame_btnOK(Frame* frame) {
  cfg.curFrame = 3;
}

void mainMenuScrollUp(Frame* frame) {
  int numOfItems = 7;
  cfg.curFrame = ((cfg.curFrame + numOfItems - 3 - 1) % numOfItems) + 3;
}

void mainMenuScrollDown(Frame* frame) {
  int numOfItems = 7;
  cfg.curFrame = ((cfg.curFrame + numOfItems - 3 + 1) % numOfItems) + 3;
}

void getLastDecontamination(Frame* frame) {
  //strcpy_P(frame->topLine, t1018);
  sprintf(frame->bottomLine, "neco");
  setLanguage(1);
}

void manDecFrame_btnOK(Frame* frame) {
  cfg.curFrame = 10;
}

void manDecMenuScrollUp(Frame* frame) {
  int numOfItems = 3;
  cfg.curFrame = ((cfg.curFrame + numOfItems - 10 - 1) % numOfItems) + 10;
}

void manDecMenuScrollDown(Frame* frame) {
  int numOfItems = 3;
  cfg.curFrame = ((cfg.curFrame + numOfItems - 10 + 1) % numOfItems) + 10;
}


Frame* frames[20];

void setup()
{
    /*DisplayFrame* frame000 = ;
    DisplayFrame* frame001 = ;
    DisplayFrame* frame002 = ;*/

    // inicialize serial console
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    char* dynamicTopLine = (char*) malloc(20);
    char* dynamicBottomLine = (char*) malloc(20);
    Frame* tmpFrame = NULL;

    setLanguage(0);
    Serial.println((int) cfg.curLang);
    
    /*frames[0] = new DisplayFrame(t1000, t1001);
    frames[1] = new DisplayFrame(t1002, t1003);
    frames[2] = new TimeFrame();
    frames[3] = new DisplayFrame(t1004, t1005);*/

    frames[0] = new Frame;
    tmpFrame = frames[0];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 0;
    tmpFrame->bottomLineN = 1;
    tmpFrame->afterPrint = &startWait;

    frames[1] = new Frame;
    tmpFrame = frames[1];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 2;
    tmpFrame->bottomLineN = 3;
    tmpFrame->afterPrint = &startWait;

    frames[2] = new Frame;
    tmpFrame = frames[2];
    tmpFrame->type = FT_SRAM_SRAM;
    tmpFrame->topLine = dynamicTopLine;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &getDatetimeForPrint;
    tmpFrame->btnOK = &datetimeFrame_btnOK;
    
    frames[3] = new Frame;
    tmpFrame = frames[3];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 4;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = &manDecFrame_btnOK;
    
    frames[4] = new Frame;
    tmpFrame = frames[4];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 6;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    //3
    frames[5] = new Frame;
    tmpFrame = frames[5];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 7;
    tmpFrame->bottomLineN = 8;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    frames[6] = new Frame;
    tmpFrame = frames[6];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 9;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    frames[7] = new Frame;
    tmpFrame = frames[7];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 10;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    frames[8] = new Frame;
    tmpFrame = frames[8];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 11;
    tmpFrame->bottomLineN = 12;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    frames[9] = new Frame;
    tmpFrame = frames[9];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 13;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &getLastDecontamination;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    //manualni dekontaminace vypnuta
    frames[10] = new Frame;
    tmpFrame = frames[10];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 15;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;

    //manualni dekontaminace casovana
    frames[11] = new Frame;
    tmpFrame = frames[11];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 16;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    
    //manualni dekontaminace okamzita
    frames[12] = new Frame;
    tmpFrame = frames[12];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 17;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    
    
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
    // inital sequence on display
    //printFrame(*(frames[cfg.curFrame]));
    
    /*delay(INITIAL_DELAY);
    lcd.clear(); 

    cfg.curFrame = 1;
    
    //printFrame(*(frames[cfg.curFrame]));
    delay(INITIAL_DELAY); */
    lcd.clear(); 
}

void loop()
{
    static unsigned long rpt(REPEAT_FIRST);              // a variable time that is used to drive the repeats for long presses
    char buffer[LCD_CHARS + 1];                                     // buffer for dispaly text
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
      /*strcpy_P(buffer, frame->topLine);
      Serial.println(buffer);
      strcpy_P(buffer, frame->bottomLine);
      Serial.println(buffer);*/
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
