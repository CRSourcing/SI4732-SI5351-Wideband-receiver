
void SecScreen() {

  clearStatusBar();
  redrawMainScreen = true;
  drawSecBtns();
  readSecBtns();
}

//##########################################################################################################################//

void drawSecBtns() {


  if (!altStyle)  // restore background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
 
  else
    drawButton(2, 60, 337, 229, TFT_NAVY, TFT_DARKGREY);  // plain buttons background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  struct Button {
    int x;
    int y;
    const char* label;
  };


  Button buttons[] = {
    { 20, 134, "TinySA" }, { 25, 152, "Mode" }, { 100, 132, "TinySA" }, { 100, 152, "Preset" }, { 186, 132, "TinySA" }, { 185, 152, "Sync" }, { 270, 132, "Draw" }, { 270, 155, "3D" }, { 18, 190, "Quick" }, { 18, 210, "Save" }, { 100, 190, "Quick" }, { 100, 210, "Load" }, { 190, 200, "BFO" }, { 182, 208, " " }, { 273, 200, "Attn." }, { 265, 208, " " }, { 270, 255, "Style" }, { 185, 245, "Sprite" }, { 185, 265, "Style" }, { 100, 245, "Web" }, { 100, 265, "Tools" }, { 20, 254, "More" }
  };

  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_GREEN);

#ifdef TINYSA_PRESENT
  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }
#endif
#ifndef TINYSA_PRESENT
  for (int i = 6; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].label);
  }
#endif


  if (modType != AM) {  // AGC only working in AM
    etft.setCursor(273, 200);
    etft.setTextColor(TFT_DARKGREY);
    etft.print("Attn.");
    etft.setTextColor(TFT_GREEN);
  }

  etft.setTextColor(TFT_GREEN);
  tDoublePress();
}
//##########################################################################################################################//

void readSecBtns() {

  if (!pressed) return;

  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

  tft.setTextColor(textColor);
  switch (buttonID) {

#ifdef TINYSA_PRESENT
    case 21:
      tinySACenterMode = !tinySACenterMode;
      tft.setCursor(10, 85);
      if (tinySACenterMode) {
        tft.print("Center Mode");
        Serial.println("color 6 0x000000");
        delay(50);
        Serial.println("color 7 0x0009ff");
      } else {
        tft.printf("Window mode");
        Serial.println("color 7 0x00ff04");
      }
      break;
    case 22:
      tinySAScreen();
      tRel();
      break;
    case 23:
      tft.fillRect(330, 8, 135, 15, TFT_BLACK);  // overwrite last microvolt indication
      tft.fillRect(250, 294, 229, 25, TFT_BLACK);
      tft.fillCircle(470, 20, 2, TFT_BLACK); // overwrite communication indicator
      syncEnabled = !syncEnabled;
      tft.setCursor(10, 90);
      tft.printf("Sync with TinySA %s\n", syncEnabled ? "Enabled" : "Disabled");
      delay(500);
      preferences.putBool("useTSADBm", syncEnabled);
      tRel();
      break;
#endif
    case 24:
      drawTrapezoid = true;
      touchTune = !touchTune;
      tRel();
      break;
    case 31:
     preferences.putLong("savFreq", FREQ);  // quicksave
      preferences.putInt("bandWidth", bandWidth);
      preferences.putChar("modType", modType);
      break;
      break;
    case 32:
      FREQ = preferences.getLong("savFreq", 0);  // quickload
      bandWidth = preferences.getInt("bandWidth", 0);
      modType = preferences.getChar("modType", 0);
      loadSi4735parameters();
      displayFREQ(FREQ);
      break;
    case 33:
      setBFO();
      tRel();
      break;
    case 34:
      setAGCMode(0);
      tRel();
      break;
    case 41:
      ThirdScreen();
      break;
    case 42:
      drawIBtns();
      readIBtns();
      tRel();
      break;
    case 43:
      selectButtonStyle();
      break;
    case 44:
      altStyle = !altStyle;  // change between plain and sprite style
      preferences.putBool("lastStyle", altStyle);
      drawBigBtns();  // redraw with new style
      break;
    default:
      redrawMainScreen = true;
      tx = ty = pressed = 0;
      return;
  }
  redrawMainScreen = true;
  tRel();
}

//##########################################################################################################################//
void setClockMode() {  // switch clocks btw tx and rxmode
  restartSynth();      // need to restart so that clock switches reliably
  if (txmode) {
    rxmode = true;
    txmode = false;
    si5351.output_enable(SI5351_CLK1, 0);
    si5351.output_enable(SI5351_CLK2, 1);
    tft.fillRect(320, 5, 155, 18, TFT_BLACK);

  } else if (rxmode) {
    txmode = true;
    rxmode = false;
    si5351.output_enable(SI5351_CLK1, 1);
    si5351.output_enable(SI5351_CLK2, 0);
  }
  tft.setTextColor(textColor);
  tuneSI5351();
  FREQ_OLD -= 1;  // trigger  FREQ display update
}


//##########################################################################################################################//

void setAGCMode(int mode) {
  // AGCDIS: AGC enabled (0) or disabled (1)
  // AGCIDX: AGC Index (0 = max gain, 1-36 = intermediate, >36 = min gain)
  // mode 0 = button control, mode -1 or 1 = icon control

  if (modType != AM) {
    return;
  }

  if (mode) {
    // Handle icon control for AGC adjustment
    while (pressed) {
      pressed = tft.getTouch(&tx, &ty);
      if (mode == 1) {
        AGCIDX++;
      } else if (mode == -1 && AGCIDX) {
        AGCIDX--;
      }

      if (AGCIDX > 36) {
        AGCIDX = 36;
      }

      AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0
      writeAGCMode();
      printAGC();
      delay(50);
    }
    return;
  }

  encLockedtoSynth = false;

  clearNotification();
  tft.setCursor(5, 75);
  tft.print("Use encoder to change Attn.");

  while (digitalRead(ENC_PRESSED) == HIGH) {
    delay(50);

    if (clw) {
      AGCIDX++;
    } else if (cclw) {
      AGCIDX--;
    }

    if (AGCIDX > 36) {
      AGCIDX = 0;
    }

    AGCDIS = (AGCIDX != 0);  // Disable AGC if AGCIDX is 0, enable otherwise
    writeAGCMode();

    if (clw != cclw) {
      printAGC();
    }

    clw = false;
    cclw = false;
  }

  // Prevent jumping into setStep function
  while (digitalRead(ENC_PRESSED) == LOW)
    ;

  encLockedtoSynth = true;
  clearNotification();
  tx = ty = pressed = 0;
}


//##########################################################################################################################//

void writeAGCMode() {

  if (modType == AM)
    si4735.setAutomaticGainControl(AGCDIS, AGCIDX);
  if (modType == LSB || modType == USB || modType == SYNC) {
    si4735.setSsbAgcOverrite(1, AGCIDX);  // disable AGC to eliminate SSB humming noise
    printAGC();
  }
}
//##########################################################################################################################//


void printAGC() {

  tft.setTextColor(TFT_GREEN);
  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(102, 26);
    spr.pushImage(0, 0, 102, 26, (uint16_t*)Oval102);
    spr.pushSprite(225, 92);
    spr.deleteSprite();
  }

  tft.setCursor(235, 98);
  if (modType == AM)
    tft.printf(AGCDIS == false ? "AGC:ON" : "Attn:%d ", AGCIDX);
  else
    tft.print("AGC:OFF");
  tft.setTextColor(textColor);
}
//##########################################################################################################################//

void setBFO() { // uses seperate BFO`s for USB/LSB Hi and Low injection

  int offset = 0, oldOffset = 0;

  encLockedtoSynth = false;

  tft.setCursor(10, 75);
  tft.print("Use encoder to change BFO.");
  tft.setCursor(10, 93);
  tft.print("Press encoder to save.");
  delay(1000);


  // using 4 BFO's for SSB:

  if (modType == USB && LOAboveRF)
    offset = preferences.getInt("B1", 0);
  if (modType == USB && !LOAboveRF)
    offset = preferences.getInt("B2", 0);
  if (modType == LSB && LOAboveRF)
    offset = preferences.getInt("B3", 0);
  if (modType == LSB && !LOAboveRF)
    offset = preferences.getInt("B4", 0);



  if (modType == SYNC)
    offset = preferences.getInt("SYNCBfoOffset", 0);
  if (modType == CW)
    offset = preferences.getInt("CWBfoOffset", 0);

  while (digitalRead(ENC_PRESSED) == HIGH) {

    if (oldOffset != offset) {
      tft.fillRect(10, 75, 320, 17, TFT_BLACK);
      tft.setCursor(10, 75);
      tft.printf("BFO Offset: %d", offset);



      clearStatusBar();
      tft.setCursor(5, 300);
      tft.printf("B1:%ld B2:%ld B3:%ld B4:%ld", preferences.getInt("B1", 0), preferences.getInt("B2", 0), preferences.getInt("B3", 0), preferences.getInt("B4", 0));
      oldOffset = offset;
    }

    delay(50);
    if (clw)
      offset += 25;
    if (cclw)
      offset -= 25;

    si4735.setSSBBfo(offset);

    clw = false;
    cclw = false;
  }

  while (digitalRead(ENC_PRESSED) == LOW)
    ;


  if (modType == USB && LOAboveRF)
    preferences.putInt("B1", offset);

  if (modType == USB && !LOAboveRF)
    preferences.putInt("B2", offset);

  if (modType == LSB && LOAboveRF)
    preferences.putInt("B3", offset);

  if (modType == LSB && !LOAboveRF)
    preferences.putInt("B4", offset);


  if (modType == SYNC)
    preferences.putInt("SYNCBfoOffset", offset);

  if (modType == CW)
    preferences.putInt("CWBfoOffset", offset);

  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//

void selectButtonStyle() {  // selects btw. different sprites for the buttons

  tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  for (int i = 0; i < 8; i++) {
    spr.createSprite(SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT);
    spr.pushImage(0, 0, SPRITEBTN_WIDTH, SPRITEBTN_HEIGHT, (uint16_t*)buttonImages[i]);  // Use the corresponding button image

    if (i < 4)
      spr.pushSprite(8 + i * 83, 178);
    else
      spr.pushSprite(8 + (i - 4) * 83, 235);
    spr.deleteSprite();
  }
  tPress();
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 20) / vTouchSpacing);
  if (row > 4 || column > 4)
    return;
  buttonSelected = (column - 1) + 4 * (row - 3);
  preferences.putInt("sprite", buttonSelected);  // write selection to EEPROM
}

//##########################################################################################################################//

void clearStatusBar() {

  tft.fillRect(0, 294, 479, 25, TFT_BLACK);
}

//##########################################################################################################################//
void clearNotification() {

  tft.fillRect(5, 75, 330, 16, TFT_BLACK);
}
