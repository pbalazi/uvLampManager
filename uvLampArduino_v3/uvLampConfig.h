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
    //INITIAL_DELAY = 10000;
    INITIAL_DELAY = 5000; 
const unsigned int
    TIME_DISPLAY_DELAY = 300;

// define beep tone and delay
const byte
    BEEP_PIN(22),
    BEEP_TONE(70),
    BEEP_LENGTH(200);

//frame flags
const byte
    FRAME_FLAG_TL_ARROW = 0b00000010,
    FRAME_FLAG_BL_ARROW = 0b00000001,
    FRAME_FLAG_BL_UD_ARROW = 0b00000100;

//frame types
enum frameTypes {
  FT_PROGMEM_PROGMEM,
  FT_SRAM_SRAM,
  FT_PROGMEM_SRAM,
  FT_PROGMEM_PROGMEM_2,
  FT_PROGMEM_SRAM_2,
};

//frame definition
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

// texts for menu - all languages
/*const char t0011_cz[] PROGMEM = "PCR Box"; 
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
const char* const t0032[] PROGMEM = {t0032_cz, t0032_en, t0032_de};*/


const char t1000[] PROGMEM = "PCR Box";
const char t1001[] PROGMEM = "Zephyrus";

const char t1002[] PROGMEM = "Elisabeth";
const char t1003[] PROGMEM = "Pharmacon";

//const char t1004[] PROGMEM = 
//const char t1005[] PROGMEM = 

const char t1006[] PROGMEM = "1.Manualni zap.";
const char t1007[] PROGMEM = "dekontaminace";

const char t1008[] PROGMEM = "2.Automat. zap.";
//const char t1009[] PROGMEM = "dekontaminace";

const char t1010[] PROGMEM = "3.Nast. casu";
const char t1011[] PROGMEM = "zahajeni dek.";

const char t1012[] PROGMEM = "4.Nast. delky";
//const char t1013[] PROGMEM = "dekontaminace";

const char t1014[] PROGMEM = "5.Aktivni";
//const char t1015[] PROGMEM = "dekontaminace";

const char t1016[] PROGMEM = "6.Ostatni";
const char t1017[] PROGMEM = "nastaveni";

const char t1018[] PROGMEM = "7.Posledni dek.:";

const char t1019[] PROGMEM = "Man. zap. dek.:";
const char t1020[] PROGMEM = "Vypnuta";
const char t1021[] PROGMEM = "Casovana";
const char t1022[] PROGMEM = "Okamzita";

const char t1023[] PROGMEM = "Casovana dekon-";
const char t1024[] PROGMEM = "taminace vypnuta";

const char t1025[] PROGMEM = "Zavrete viko";
const char t1026[] PROGMEM = "pro pokracovani";

const char t1027[] PROGMEM = "Dekon. prerusena";
const char t1028[] PROGMEM = "Pokracovat?";
const char t1029[] PROGMEM = "Prerusit?";

const char t1030[] PROGMEM = "Automat. dek.:";
const char t1031[] PROGMEM = "Zapnuto";
const char t1032[] PROGMEM = "Vypnuto";

const char t1033[] PROGMEM = "Zadejte cas dek:";

const char t1034[] PROGMEM = "Delka casovane";
const char t1035[] PROGMEM = "Delka dekontam.:";

const char t1036[] PROGMEM = "Delka okamzite";

const char t1037[] PROGMEM = "Stav:";
const char t1038[] PROGMEM = "Zapnuto bez UV";

const char t1039[] PROGMEM = "Rezim:";
const char t1040[] PROGMEM = "Otevreny kryt";
const char t1041[] PROGMEM = "Zavreny kryt";
const char t1042[] PROGMEM = "Pri dekont.";
const char t1043[] PROGMEM = "Stale";

const char t1044[] PROGMEM = "";
const char t1045[] PROGMEM = "";
const char t1046[] PROGMEM = "";
const char t1047[] PROGMEM = "";
const char t1048[] PROGMEM = "";
const char t1049[] PROGMEM = "";
const char t1050[] PROGMEM = "";
const char t1051[] PROGMEM = "";
const char t1052[] PROGMEM = "";
const char t1053[] PROGMEM = "";
const char t1054[] PROGMEM = "";
const char t1055[] PROGMEM = "";
const char t1056[] PROGMEM = "";
const char t1057[] PROGMEM = "";
const char t1058[] PROGMEM = "";
const char t1059[] PROGMEM = "";
const char t1060[] PROGMEM = "";
const char t1061[] PROGMEM = "";
const char t1062[] PROGMEM = "";
const char t1063[] PROGMEM = "";
const char t1064[] PROGMEM = "";
const char t1065[] PROGMEM = "";
const char t1066[] PROGMEM = "";
const char t1067[] PROGMEM = "";
const char t1068[] PROGMEM = "";
const char t1069[] PROGMEM = "";
const char t1070[] PROGMEM = "";
const char t1071[] PROGMEM = "";
const char t1072[] PROGMEM = "";
const char t1073[] PROGMEM = "";
const char t1074[] PROGMEM = "";
const char t1075[] PROGMEM = "";
const char t1076[] PROGMEM = "";
const char t1077[] PROGMEM = "";
const char t1078[] PROGMEM = "";
const char t1079[] PROGMEM = "";

const char* const czech[] PROGMEM = { t1000, t1001, t1002, t1003, t1006, t1007, t1008, t1010, t1011, t1012, //00-09
                                      t1014, t1016, t1017, t1018, t1019, t1020, t1021, t1022, t1023, t1024, //10-19
                                      t1025, t1026, t1027, t1028, t1029, t1030, t1031, t1032, t1033, t1034, //20-29
                                      t1035, t1036, t1037, t1038, t1039, t1040, t1041, t1042, t1043, t1044, //30-39
                                      t1045, t1046, t1047, t1048, t1049, t1050, t1051, t1052, t1053, t1054, //40-49
                                      t1055, t1056, t1057, t1058, t1059, t1060, t1061, t1062, t1063, t1064, //50-59
                                      t1065, t1066, t1067, t1068, t1069, t1070, t1071, t1072, t1073, t1074, //60-69
                                      t1075, t1076, t1077, t1078, t1079 };                                  //70-74


const char t2000[] PROGMEM = "PCR Box";
const char t2001[] PROGMEM = "Zephyrus";

const char t2002[] PROGMEM = "Elisabeth";
const char t2003[] PROGMEM = "Pharmacon";

//const char t2004[] PROGMEM = 
//const char t2005[] PROGMEM = 

const char t2006[] PROGMEM = "1.Manual start";
const char t2007[] PROGMEM = "decontamination";

const char t2008[] PROGMEM = "2.Auto start";
//const char t2009[] PROGMEM = "dekontaminace";

const char t2010[] PROGMEM = "3.Time set of";
const char t2011[] PROGMEM = "start decontam.";

const char t2012[] PROGMEM = "4.Nast. delky";
//const char t2013[] PROGMEM = "dekontaminace";

const char t2014[] PROGMEM = "5.Aktivni";
//const char t2015[] PROGMEM = "dekontaminace";

const char t2016[] PROGMEM = "6.Ostatni";
const char t2017[] PROGMEM = "nastaveni";

const char t2018[] PROGMEM = "7.Posledni dek.:";

const char t2019[] PROGMEM = "Man. zap. dek.:";
const char t2020[] PROGMEM = "Vypnuta";
const char t2021[] PROGMEM = "Casovana";
const char t2022[] PROGMEM = "Okamzita";

const char t2023[] PROGMEM = "Casovana dekon-";
const char t2024[] PROGMEM = "taminace vypnuta";

const char t2025[] PROGMEM = "Zavrete viko";
const char t2026[] PROGMEM = "pro pokracovani";

const char t2027[] PROGMEM = "Dekon. prerusena";
const char t2028[] PROGMEM = "Pokracovat?";
const char t2029[] PROGMEM = "Prerusit?";

const char t2030[] PROGMEM = "";
const char t2031[] PROGMEM = "";
const char t2032[] PROGMEM = "";
const char t2033[] PROGMEM = "";
const char t2034[] PROGMEM = "";
const char t2035[] PROGMEM = "";
const char t2036[] PROGMEM = "";
const char t2037[] PROGMEM = "";
const char t2038[] PROGMEM = "";
const char t2039[] PROGMEM = "";

const char* const english[] PROGMEM = { t2000, t2001, t2002, t2003, t2006, t2007, t2008, t2010, t2011, t2012, //00-09
                                        t2014, t2016, t2017, t2018, t2019, t2020, t2021, t2022, t2023, t2024, //10-19
                                        t2025, t2026, t2027, t2028, t2029, t2030, t2031, t2032, t2033, t2034, //20-29
                                        t2035, t2036, t2037, t2038, t2039 };



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
  char** curLang = NULL;
  //
  int curFrame = 0;
  //
  bool reqToPrint = true;
};

byte downArrow[8] = {
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b00000, //   
  0b11111, // *****
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b11111, // *****
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

byte upDownArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b11111, // *****
  0b00000, //   
  0b00000, //   
  0b11111, // *****
  0b01110, //  ***
  0b00100  //   *  
};
