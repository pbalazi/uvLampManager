

/*
 * According to numeric value set current language pointer
 * to array of texts in program memory
 */
void setLanguage(byte lang) {
  if (lang == (byte) 0) {
    cfg.curLang = (char**) czech;
  }
  else if (lang == (byte) 1) {
    cfg.curLang = (char**) english;
  }
}



/*
 * Print frame on a display
 */
void printFrame(Frame* frame) {
  char buffer[LCD_CHARS + 1];



  // produce first line of output
  // fill buffer by text and then print first line of LCD display
  
  if ((frame->type == FT_PROGMEM_PROGMEM) || (frame->type == FT_PROGMEM_SRAM)) {
    strcpy_P(buffer, frame->topLine);
  }
  else if (frame->type == FT_SRAM_SRAM) {
    strcpy(buffer, frame->topLine);
  }
  else if ((frame->type == FT_PROGMEM_PROGMEM_2) || frame->type == FT_PROGMEM_SRAM_2) {
    strcpy_P(buffer, (char*) pgm_read_word(&(cfg.curLang[frame->topLineN])));
  }
          
  Serial.println(buffer);
  lcd.setCursor ( 0, 0 );
  lcd.print(buffer);



  // produce second line of output
  // fill buffer by text and then print second line of LCD display
  
  if (frame->type == FT_PROGMEM_PROGMEM) {
    strcpy_P(buffer, frame->bottomLine);
  }
  else if ((frame->type == FT_SRAM_SRAM) || (frame->type == FT_PROGMEM_SRAM) || (frame->type == FT_PROGMEM_SRAM_2)) {
    strcpy(buffer, frame->bottomLine);
  }
  else if (frame->type == FT_PROGMEM_PROGMEM_2) {
    strcpy_P(buffer, (char*) pgm_read_word(&(cfg.curLang[frame->bottomLineN])));
  }
  
  Serial.println(buffer);
  lcd.setCursor ( 0, 1 );
  lcd.print(buffer);


  // print display arrows according to flags in frame
  
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



/*
 * Beep on speaker
 */
void beep(unsigned char delayms){
  analogWrite(BEEP_PIN, BEEP_TONE);      // Almost any value can be used except 0 and 255
                           // experiment to get the best tone
  delay(delayms);          // wait for a delayms ms
  analogWrite(BEEP_PIN, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}  


/*
 * Boot frames (0 and 1) afterPrint function
 * 
 * Delay and increment current frame number
 */
void startWait(Frame* frame) {
  delay(INITIAL_DELAY);
  cfg.curFrame++;
  cfg.reqToPrint = true;
}


/*
 * Wait on the current frame and after 5 seconds
 * without event jump to default datetime frame
 */
void wait5sec(Frame* frame) {
  dateTime = rtc.getDateTime();

  //diff is bigger then 5 seconds
  if ((dateTime.unixtime - lastDateTime) > 5) {
    cfg.curFrame = 2; //jump to default datetime frame
  }
}

/*
 * Default datetime frame beforePrint function
 * 
 * Fill the first line to print by time, the second by date
 */
void getDatetimeForPrint(Frame* frame) {
  dateTime = rtc.getDateTime();

  sprintf(frame->topLine, "%02d:%02d:%02d", dateTime.hour, dateTime.minute, dateTime.second);
  sprintf(frame->bottomLine, "%02d.%02d.%0d", dateTime.day, dateTime.month, dateTime.year);

  cfg.reqToPrint = true;
}

/*
 * Change current frame according to offset (e.g. 1 or -1) and based on lowest frame index
 * and number of items
 * 
 * Frames in (sub)menu has to be adjacent!
 */
void scrollCurFrameInMenu(byte lowestFrameIndex, byte numberOfItems, int offset) {
  cfg.curFrame = ((cfg.curFrame + numberOfItems - lowestFrameIndex + offset) % numberOfItems) + lowestFrameIndex;
}

/*
 * Scroll menu "up"
 */
void mainMenuScrollUp(Frame* frame) {
  scrollCurFrameInMenu(3, 7, -1);
}


/*
 * Scroll menu "down"
 */
void mainMenuScrollDown(Frame* frame) {
  scrollCurFrameInMenu(3, 7, 1);
}

/*
 * beforePrint function for last decontamination frame
 * 
 * TODO: fill the second line by date and time
 */
void getLastDecontamination(Frame* frame) {
  //strcpy_P(frame->topLine, t1018);
  sprintf(frame->bottomLine, "neco");
}

/*
 * Scroll "up" in manual decontamination menu
 */
void manDecMenuScrollUp(Frame* frame) {
  scrollCurFrameInMenu(10, 3, -1);
}

/*
 * Scroll "down" in manual decontamination menu
 */
void manDecMenuScrollDown(Frame* frame) {
  scrollCurFrameInMenu(10, 3, 1);
}

/*
 * Change value of spare bytes in dynamic memory for lines
 */
void changeDynamicByte_btnUp_btnDown(char* linePointer, byte bytePointer, byte maxLength, int offset) {
  *(linePointer + bytePointer) = (byte) ((*(linePointer + bytePointer) + maxLength + offset) % maxLength);
}

/*
 * Fill second line of frame for set timed decontamination by time
 * 
 * Use spare bytes of dynamic memory for time store
 */
void setDecTime_beforePrint(char* linePointer, byte hourOrMinute) {
  *(linePointer + 17) = (byte) (((byte) *(linePointer + 17)) % 24);
  *(linePointer + 18) = (byte) (((byte) *(linePointer + 18)) % 60);
  
  byte hour = (byte) *(linePointer + 17);
  byte minute = (byte) *(linePointer + 18);

  if (hourOrMinute == 0) {
    sprintf(linePointer, ">%02d:%02d", hour, minute);
  }
  else {
    sprintf(linePointer, " %02d>%02d", hour, minute);
  }
}

/*
 * beforePrint function of hour setting
 */
void setDecHourTime_beforePrint(Frame* frame) {
  setDecTime_beforePrint(frame->bottomLine, 0);
}

/*
 * Increment hour setting
 */
void setDecHourTime_btnUp(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 24, 1);
}

/*
 * Decrement hour setting
 */
void setDecHourTime_btnDown(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 24, -1);
}

/*
 * beforePrint function of minute setting
 */
void setDecMinuteTime_beforePrint(Frame* frame) {
  setDecTime_beforePrint(frame->bottomLine, 1);
}

/*
 * Increment minute setting
 */
void setDecMinuteTime_btnUp(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 60, 1);
}

/*
 * Decrement minute setting
 */
void setDecMinuteTime_btnDown(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 60, -1);
}


/*
 * Fill second line of frame for length of decontamination setting
 * 
 * Use spare bytes of dynamic memory for length store
 */
void setDecLength_beforePrint(char* linePointer, byte bytePointer, byte maxLength) {
  *(linePointer + bytePointer) = (byte) (((byte) *(linePointer + bytePointer)) % maxLength);

  byte dlength = ((byte) *(linePointer + bytePointer))*5 + 15;

  sprintf(linePointer, "%02d min", dlength);
}


/*
 * beforePrint function of timed decontamination frame
 */
void setTimedDecLength_beforePrint(Frame* frame) {
  setDecLength_beforePrint(frame->bottomLine, 17, 6);
}

/*
 * Increment length of decontamination (timed)
 */
void setTimedDecLength_btnUp(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 6, 1);
}

/*
 * Decrement length of decontamination (timed)
 */
void setTimedDecLength_btnDown(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 17, 6, -1);
}

/*
 * beforePrint function of instant decontamination frame
 */
void setInstantDecLength_beforePrint(Frame* frame) {
  setDecLength_beforePrint(frame->bottomLine, 18, 6);
}

/*
 * Increment length of decontamination (instant)
 */
void setInstantDecLength_btnUp(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 6, 1);
}

/*
 * Decrement length of decontamination (instant)
 */
void setInstantDecLength_btnDown(Frame* frame) {
  changeDynamicByte_btnUp_btnDown(frame->bottomLine, 18, 6, -1);
}




void initializeFrames(Frame* frames[]) {
  Frame* tmpFrame = NULL;

  // 20 bytes -- 17 bytes for display row (16 bytes + '\0') + 3 bytes for general use
  char* dynamicTopLine = (char*) malloc(20);
    
  // 20 bytes -- 17 bytes for display row (16 bytes + '\0') + 3 bytes for general use
  char* dynamicBottomLine = (char*) malloc(20);
  
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
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 3; };

    //1 main menu -- manual start of decontamination
    frames[3] = new Frame;
    tmpFrame = frames[3];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 4;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 10; };

    //2 main menu -- auto start of decontamination
    frames[4] = new Frame;
    tmpFrame = frames[4];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 6;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 18; };

    //3 main menu -- start time setting of decontamination
    frames[5] = new Frame;
    tmpFrame = frames[5];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 7;
    tmpFrame->bottomLineN = 8;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 20; };

    //4 main menu -- length setting of decontamination
    frames[6] = new Frame;
    tmpFrame = frames[6];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 9;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 22; };

    //5 main menu -- active decontamination
    frames[7] = new Frame;
    tmpFrame = frames[7];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 10;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };

    //6 main menu -- other setting
    frames[8] = new Frame;
    tmpFrame = frames[8];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 11;
    tmpFrame->bottomLineN = 12;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    //7 main menu -- last decontamination (date and time)
    frames[9] = new Frame;
    tmpFrame = frames[9];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 13;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &getLastDecontamination;
    tmpFrame->btnUp = &mainMenuScrollUp;
    tmpFrame->btnDown = &mainMenuScrollDown;

    //manual decontamination -- turned off
    frames[10] = new Frame;
    tmpFrame = frames[10];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 15;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;

    //manual decontamination -- timed
    frames[11] = new Frame;
    tmpFrame = frames[11];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 16;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 13; };
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    
    //manual decontamination -- instant
    frames[12] = new Frame;
    tmpFrame = frames[12];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 14;
    tmpFrame->bottomLineN = 17;
    tmpFrame->btnUp = &manDecMenuScrollUp;
    tmpFrame->btnDown = &manDecMenuScrollDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 14; };
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;

    //timed decontamination has been turned off
    frames[13] = new Frame;
    tmpFrame = frames[13];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 18;
    tmpFrame->bottomLineN = 19;
    tmpFrame->beforePrint = &wait5sec;

    //for continue close the cover
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

    //decontamination has been interrupted - continue?
    frames[15] = new Frame;
    tmpFrame = frames[15];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLineN = 23;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 16; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 14; };

    //decontamination has been interrupted - abort?
    frames[16] = new Frame;
    tmpFrame = frames[16];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLineN = 24;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 15; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 17; };

    //decontamination has been aborted
    frames[17] = new Frame;
    tmpFrame = frames[17];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 22;
    tmpFrame->bottomLine = (char*) "";
    tmpFrame->beforePrint = &wait5sec;

    //auto decontamination - turned on
    frames[18] = new Frame;
    tmpFrame = frames[18];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 25;
    tmpFrame->bottomLineN = 26;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 19; };
    //tmpFrame->btnOK = TODO

    //auto decontamination - turned off
    frames[19] = new Frame;
    tmpFrame = frames[19];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 25;
    tmpFrame->bottomLineN = 27;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 18; };
    //tmpFrame->btnOK = TODO

    //start time of decontamination - hour setting
    frames[20] = new Frame;
    tmpFrame = frames[20];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 28;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setDecHourTime_beforePrint;
    tmpFrame->btnUp = &setDecHourTime_btnUp;
    tmpFrame->btnDown = &setDecHourTime_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 21; };

    //start time of decontamination - minute setting
    frames[21] = new Frame;
    tmpFrame = frames[21];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 28;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setDecMinuteTime_beforePrint;
    tmpFrame->btnUp = &setDecMinuteTime_btnUp;
    tmpFrame->btnDown = &setDecMinuteTime_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 20; };

    //length of timed decontamination
    frames[22] = new Frame;
    tmpFrame = frames[22];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 29;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 24; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 23; };

    //length of timed decontamination - length setting
    frames[23] = new Frame;
    tmpFrame = frames[23];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 30;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setTimedDecLength_beforePrint;
    tmpFrame->btnUp = &setTimedDecLength_btnUp;
    tmpFrame->btnDown = &setTimedDecLength_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 22; };

    //length of instant decontamination
    frames[24] = new Frame;
    tmpFrame = frames[24];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 31;
    tmpFrame->bottomLineN = 5;
    tmpFrame->flags = FRAME_FLAG_TL_ARROW | FRAME_FLAG_BL_ARROW;
    tmpFrame->btnUp = tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 22; };
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 25; };


    //length of instant decontamination - length setting
    frames[25] = new Frame;
    tmpFrame = frames[25];
    tmpFrame->type = FT_PROGMEM_SRAM_2;
    tmpFrame->topLineN = 30;
    tmpFrame->bottomLine = dynamicBottomLine;
    tmpFrame->beforePrint = &setInstantDecLength_beforePrint;
    tmpFrame->btnUp = &setInstantDecLength_btnUp;
    tmpFrame->btnDown = &setInstantDecLength_btnDown;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 24; };

    //status: turned on without UV
    frames[26] = new Frame;
    tmpFrame = frames[26];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 32;
    tmpFrame->bottomLineN = 33;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 27; };

    //mode: cover opened
    frames[27] = new Frame;
    tmpFrame = frames[27];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 35;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 30; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 28; };

    //mode: cover closed
    frames[28] = new Frame;
    tmpFrame = frames[28];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 36;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 27; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 29; };

    //mode: during decontamination
    /*
     * dalsi ramce se nevlezou do pameti UNA, takze dal nemuzu testovat
     
    frames[29] = new Frame;
    tmpFrame = frames[29];
    tmpFrame->type = FT_PROGMEM_PROGMEM_2;
    tmpFrame->topLineN = 34;
    tmpFrame->bottomLineN = 37;
    tmpFrame->flags = FRAME_FLAG_BL_UD_ARROW;
    tmpFrame->btnOK = [](Frame* f) { cfg.curFrame = 26; };
    tmpFrame->btnUp = [](Frame* f) { cfg.curFrame = 28; };
    tmpFrame->btnDown = [](Frame* f) { cfg.curFrame = 30; };
    */

    //mode: permanently
    /*
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
}

