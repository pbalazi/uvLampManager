
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

void wait5sec(Frame* frame) {
  dateTime = rtc.getDateTime();
  
  if ((dateTime.unixtime - lastDateTime) > 5) {
    cfg.curFrame = 2;
  }
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

void changeDynamicByte_btnUp_btnDown(char* linePointer, byte bytePointer, byte maxLength, int offset) {
  *(linePointer + bytePointer) = (byte) ((*(linePointer + bytePointer) + maxLength + offset) % maxLength);
}

void setDecHourTime_beforePrint(Frame* frame) {
  *(frame->bottomLine + 17) = (byte) (*(frame->bottomLine + 17) % 24);
  *(frame->bottomLine + 18) = (byte) (*(frame->bottomLine + 18) % 60);
  
  byte hour = (byte) *(frame->bottomLine + 17);
  byte minute = (byte) *(frame->bottomLine + 18);

  sprintf(frame->bottomLine, ">%02d:%02d", hour, minute);
}

void setDecHourTime_btnUp(Frame* frame) {
  //*(frame->bottomLine + 17) = (byte) ((((int) *(frame->bottomLine + 17)) + 24 + 1) % 24);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 24, 1);
}

void setDecHourTime_btnDown(Frame* frame) {
  //*(frame->bottomLine + 17) = (byte) ((((int) *(frame->bottomLine + 17)) + 24 - 1) % 24);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 24, -1);
}

void setDecMinuteTime_beforePrint(Frame* frame) {
  *(frame->bottomLine + 17) = (byte) (*(frame->bottomLine + 17) % 24);
  *(frame->bottomLine + 18) = (byte) (*(frame->bottomLine + 18) % 60);
  
  byte hour = (byte) *(frame->bottomLine + 17);
  byte minute = (byte) *(frame->bottomLine + 18);

  sprintf(frame->bottomLine, " %02d>%02d", hour, minute);
}

void setDecMinuteTime_btnUp(Frame* frame) {
  //*(frame->bottomLine + 18) = (byte) ((((int) *(frame->bottomLine + 18)) + 60 + 1) % 60);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 60, 1);
}

void setDecMinuteTime_btnDown(Frame* frame) {
  //*(frame->bottomLine + 18) = (byte) ((((int) *(frame->bottomLine + 18)) + 60 - 1) % 60);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 60, -1);
}

void setDecLength_beforePrint(char* linePointer, byte bytePointer, byte maxLength) {
  *(linePointer + bytePointer) = (byte) (((byte) *(linePointer + bytePointer)) % maxLength);

  byte dlength = ((byte) *(linePointer + bytePointer))*5 + 15;

  sprintf(linePointer, "%02d min", dlength);
}



void setTimedDecLength_beforePrint(Frame* frame) {
  setDecLength_beforePrint(frame->bottomLine, 17, 6);
}

void setTimedDecLength_btnUp(Frame* frame) {
  //*(frame->bottomLine + 17) = (byte) ((*(frame->bottomLine + 17) + 6 + 1) % 6);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 6, 1);
}

void setTimedDecLength_btnDown(Frame* frame) {
  //*(frame->bottomLine + 17) = (byte) ((*(frame->bottomLine + 17) + 6 - 1) % 6);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 6, -1);
}


void setInstantDecLength_beforePrint(Frame* frame) {
  setDecLength_beforePrint(frame->bottomLine, 18, 6);
}

void setInstantDecLength_btnUp(Frame* frame) {
  //*(frame->bottomLine + 18) = (byte) ((*(frame->bottomLine + 18) + 6 + 1) % 6);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 6, 1);
}

void setInstantDecLength_btnDown(Frame* frame) {
  //*(frame->bottomLine + 18) = (byte) ((*(frame->bottomLine + 18) + 6 - 1) % 6);
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 6, -1);
}



Frame* frames[32];

void setup()
{

    // inicialize serial console
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }


    char* dynamicTopLine = (char*) malloc(20);
    
    // 20 bytes -- 17 bytes for display row (16 bytes + '\0') + 3 bytes for general use
    char* dynamicBottomLine = (char*) malloc(20);


    
    Frame* tmpFrame = NULL;

    setLanguage(0);
    Serial.println((int) cfg.curLang);

    //boot frame 1
    frames[0] = new Frame;
    tmpFrame = frames[0];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 0;
    tmpFrame->bottomLineN = 1;
    tmpFrame->afterPrint = &startWait;

    //boot frame 2
    frames[1] = new Frame;
    tmpFrame = frames[1];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 2;
    tmpFrame->bottomLineN = 3;
    tmpFrame->afterPrint = &startWait;

    //datetime (main) frame
    frames[2] = new Frame;
    tmpFrame = frames[2];
    tmpFrame->type = FT_SRAM_SRAM;
    tmpFrame->topLine = dynamicTopLine;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &getDatetimeForPrint;
    tmpFrame->btnOK = &datetimeFrame_btnOK;

    //main menu -- entry #1
    frames[3] = new Frame;
    tmpFrame = frames[3];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 4;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 10; };
    
    frames[4] = new Frame;
    tmpFrame = frames[4];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 6;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 18; };

    //3
    frames[5] = new Frame;
    tmpFrame = frames[5];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 7;
    tmpFrame->bottomLineN = 8;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 20; };

    frames[6] = new Frame;
    tmpFrame = frames[6];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 9;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 22; };

    //5
    frames[7] = new Frame;
    tmpFrame = frames[7];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 10;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };

    frames[8] = new Frame;
    tmpFrame = frames[8];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 11;
    tmpFrame->bottomLineN = 12;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    //7 -- posledni dekontaminace (datum a cas)
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
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 13; };
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    
    //manualni dekontaminace okamzita
    frames[12] = new Frame;
    tmpFrame = frames[12];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 17;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 14; };
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;

    //casovana dekontaminace vypnuta
    frames[13] = new Frame;
    tmpFrame = frames[13];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 18;
    tmpFrame->bottomLineN = 19;
    tmpFrame->beforePrint = &wait5sec;

    //zavrete viko pro pokracovani
    frames[14] = new Frame;
    tmpFrame = frames[14];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 20;
    tmpFrame->bottomLineN = 21;
    tmpFrame->beforePrint = [](Frame* f) {
      dateTime = rtc.getDateTime();
      
      if ((dateTime.unixtime - lastDateTime) > 5) {
        cfg.curFrame = 15;
      }
    };

    //dekontaminace prerusena - pokracovat?
    frames[15] = new Frame;
    tmpFrame = frames[15];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLineN = 23;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 16; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 14; };

    //dekontaminace prerusena - prerusit?
    frames[16] = new Frame;
    tmpFrame = frames[16];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLineN = 24;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 15; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 17; };

    //dekontaminace prerusena
    frames[17] = new Frame;
    tmpFrame = frames[17];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLine = (char*) "";
    tmpFrame->beforePrint = &wait5sec;

    //automaticka dekontaminace - zapnuto
    frames[18] = new Frame;
    tmpFrame = frames[18];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 25;
    tmpFrame->bottomLineN = 26;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 19; };
    //tmpFrame->btnOK = TODO

    //automaticka dekontaminace - vypnuto
    frames[19] = new Frame;
    tmpFrame = frames[19];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 25;
    tmpFrame->bottomLineN = 27;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 18; };
    //tmpFrame->btnOK = TODO

    //nastaveni casu dekontaminace - nastaveni hodin
    frames[20] = new Frame;
    tmpFrame = frames[20];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 28;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setDecHourTime_beforePrint;
    tmpFrame->btnUp = &setDecHourTime_btnUp;
    tmpFrame->btnDown = &setDecHourTime_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 21; };

    //nastaveni casu dekontaminace - nastaveni minut
    frames[21] = new Frame;
    tmpFrame = frames[21];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 28;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setDecMinuteTime_beforePrint;
    tmpFrame->btnUp = &setDecMinuteTime_btnUp;
    tmpFrame->btnDown = &setDecMinuteTime_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 20; };

    //delka casovane dekontaminace
    frames[22] = new Frame;
    tmpFrame = frames[22];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 29;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 24; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 23; };

    //delka casovane dekontaminace - nastaveni delky
    frames[23] = new Frame;
    tmpFrame = frames[23];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 30;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setTimedDecLength_beforePrint;
    tmpFrame->btnUp = &setTimedDecLength_btnUp;
    tmpFrame->btnDown = &setTimedDecLength_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 22; };

    //delka okamzite dekontaminace
    frames[24] = new Frame;
    tmpFrame = frames[24];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 31;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 22; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 25; };


    //delka okamzite dekontaminace - nastaveni delky
    frames[25] = new Frame;
    tmpFrame = frames[25];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 30;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setInstantDecLength_beforePrint;
    tmpFrame->btnUp = &setInstantDecLength_btnUp;
    tmpFrame->btnDown = &setInstantDecLength_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 24; };

    //stav: zapnuto bez UV
    frames[26] = new Frame;
    tmpFrame = frames[26];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 32;
    tmpFrame->bottomLineN = 33;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 27; };

    //rezim: otevreny kryt
    frames[27] = new Frame;
    tmpFrame = frames[27];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 35;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 30; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 28; };

    //rezim: zavreny kryt
    frames[28] = new Frame;
    tmpFrame = frames[28];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 36;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 27; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 29; };

    //rezim: pri dekontaminaci
    frames[29] = new Frame;
    tmpFrame = frames[29];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 37;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 28; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 30; };

    //rezim: stale
    /*
    dalsi ramce se nevlezou do pameti UNA, takze dal nemuzu testovat
    
    frames[30] = new Frame;
    tmpFrame = frames[30];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 38;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 29; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 27; };
    */
    
    
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

    Serial.println(sizeof(Frame));
    
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
