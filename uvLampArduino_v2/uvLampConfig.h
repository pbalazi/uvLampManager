// Arduino Uv lamp variables
// Copyright (C) 2018 by Petr Balazi

// pin assignments for Buttons - each button has its own pin
const byte
    LEFT_PIN(8),
    RIGHT_PIN(7),
    UP_PIN(6),
    DOWN_PIN(5),
    OK_PIN(4),
    LIT_PIN(13),
    UV_PIN(3),
    LIGHT_PIN(2); 

// variables for setup Buttons
const unsigned long
    REPEAT_FIRST(700),              // ms required before repeating on long press
    REPEAT_INCR(150);               // repeat interval for long press

// definitons for display
const byte
    DISPALY_LENGTH(16),
    DISPALY_ROWS(2),
    LCD_ADDRESS = 0x3F,
    LCD_LINES = 2,
    LCD_CHARS = 16;
const unsigned int
    INITIAL_DELAY = 10000; 
const unsigned int
    TIME_DISPLAY_DELAY = 300;

// define beep tone and delay
const byte
    BEEP_PIN(22),
    BEEP_TONE(70),
    BEEP_LENGTH(200);

// texts for menu - all languages
const char t0011_cz[] PROGMEM = "PCR Box"; 
const char t0011_en[] PROGMEM = "na";
const char t0011_de[] PROGMEM = "na";
const char* const t0011[] PROGMEM = {t0011_cz, t0011_en, t0011_de};
const char t0012_cz[] PROGMEM = "Zephyrus"; 
const char t0012_en[] PROGMEM = "na";
const char t0012_de[] PROGMEM = "na";
const char* const t0012[] PROGMEM = {t0012_cz, t0012_en, t0012_de};

const char t0021_cz[] PROGMEM = "Elisabeth"; 
const char t0021_en[] PROGMEM = "na";
const char t0021_de[] PROGMEM = "na";
const char* const t0021[] PROGMEM = {t0021_cz, t0021_en, t0021_de};
const char t0022_cz[] PROGMEM = "Pharmacon"; 
const char t0022_en[] PROGMEM = "na";
const char t0022_de[] PROGMEM = "na";
const char* const t0022[] PROGMEM = {t0022_cz, t0022_en, t0022_de};

const char t0031_cz[] PROGMEM = "1. Manualni zap."; 
const char t0031_en[] PROGMEM = "na";
const char t0031_de[] PROGMEM = "na";
const char* const t0031[] PROGMEM = {t0031_cz, t0031_en, t0031_de};
const char t0032_cz[] PROGMEM = "dekontaminace"; 
const char t0032_en[] PROGMEM = "na";
const char t0032_de[] PROGMEM = "na";
const char* const t0032[] PROGMEM = {t0032_cz, t0032_en, t0032_de};


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
  int curLang = 0;
};

byte downArrow[8] = {
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b11111, // *****
  0b01010, //  * *
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01010, //  ***
  0b11111, // * * *
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000  //   
}; 

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};
