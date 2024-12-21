void mainScreen() {  // main screen does not block loop(), all other screens do

  if (redrawMainScreen) {  // redraw main screen when coming from a function that overwrites it
    drawMainButtons();
    DrawSmeter();
    printModulation();
    printBandWidth();
    showTinySAMode();
    showInjectionMode();
    printAGC();
    readSquelchPot(true);  // read and draw position
    redrawMainScreen = false;
  }
  readMainBtns();
}


//##########################################################################################################################//
void rebuildIndicators() {

  tft.fillRect(3, 52, 336, 30, TFT_BLACK);  // overwrite area for spectrum
  tft.fillRect(3, 82, 336, 40, TFT_BLACK);  // overwrite area for waterfall
  DrawSmeter();
  printModulation();
  printBandWidth();
  showTinySAMode();
  printAGC();
  redrawMainScreen = true; // this forces   readSquelchPot() to redraw the squelch line and circle 
  readSquelchPot(true);
  redrawMainScreen = false;
}


//##########################################################################################################################//

void drawMainButtons() {


  if (!altStyle)  // restore background
    tft.fillRect(2, 56, 337, 233, TFT_BLACK);
  else
    drawButton(2, 56, 337, 233, TFT_NAVY, TFT_DARKGREY);  // plain buttons background

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  struct Button {
    int x;
    int y;
    const char *label;
  };


  Button buttons[] = {
    { 275, 245, "Set" }, { 185, 245, "Select" }, { 110, 255, "Scan" }, { 100, 198, "Bandw" }, { 25, 198, "Step" }, { 185, 188, "Save" }, { 270, 188, "Load" }, {96, 151, "Injection" }, { 188, 132, "View" }, { 270, 132, "View" }, { 21, 151, " " } ,  { 185, 265, "Band" }, { 275, 265, "Freq" }, { 185, 208, "Memo" }, { 267, 208, "Memo" }, { 185, 151, "Range" }, { 268, 151, "Band" }, { 20, 132, "Touch" }, { 20, 153, "Tune" }
  };

  etft.setTTFFont(Arial_14);
  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.setTextColor(textColor);
    if (i == 2 && scanMode)
      etft.setTextColor(TFT_SKYBLUE);
    etft.print(buttons[i].label);
  }

  drawButton(8, 234, TILE_WIDTH, TILE_HEIGHT, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 254);
  etft.print("More");
}

//##########################################################################################################################//

void readMainBtns() {

  if (!pressed)
    return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4 || ty > 293)
    return;                                   // outside of keypad area
  redrawMainScreen = true;                    // save freq for returning from waterfall
  tft.fillRect(135, 295, 92, 24, TFT_BLACK);  // overwrite frozen spectrum window
  switch (buttonID) {
    case 21:
      drawTrapezoid = false;
      touchTune = !touchTune;
      tRel();
      break;
    case 22:
      low_High_Injection();
      tRel();
      redrawMainScreen = false;   
      break;
    case 23:
      FREQ_OLD = FREQ;
      waterFall(true);  // use keypad for waterfall
      tRel();
      tinySA_RF_Mode = true;
      loadRFMode();      // reload FREQ into tinySA
      showTinySAMode();  // update indicator
      displayFREQ(FREQ);
      tuneSI5351();  // return to previous frequenc
      while (digitalRead(ENC_PRESSED) == LOW)
        ;
      break;
    case 24:
      FREQ_OLD = FREQ;
      setBand(true);  // select band for waterfall
      tRel();
      tinySA_RF_Mode = true;
      loadRFMode();      // reload FREQ into tinySA
      showTinySAMode();  // update indicator
      displayFREQ(FREQ);
      tuneSI5351();
      while (digitalRead(ENC_PRESSED) == LOW)
        ;
      break;
    case 31:
      setSTEP(0);  // use touchbuttons
      break;
    case 32:
      setBandwidth(0);  // use touchbuttons
      tRel();
      break;
    case 33:
      showMemo(false);
      writeMemo();
      tRel();
      break;
    case 34:
      showMemo(true);
      readMemo();
      tft.fillRect(5, 294, 374, 25, TFT_BLACK);  // overwrite remanents of station names
      displayDebugInfo = true;                   // restore debug info
      tRel();
      break;
    case 41:
      SecScreen();
      tRel();
      break;
    case 42:
      ScanMode();
      tRel();
      tx = ty = pressed = 0;
      break;
    case 43:
      setBand(false);  // Select Band
      tRel();
      break;
    case 44:
      freqScreen();
      break;
    default:
      tx = ty = pressed = 0;
      tRel();
      return;
  }
}

//##########################################################################################################################//

void setBandwidth(int mode) {  // mode 0 = bandwidth selected from menu. mode -1 and 1 are touching the indicator area
  const int TEXT_Y = 195;
  const int positions[] = { 24, 106, 191, 272 };

  const uint8_t usbLsbBandWidths[] = { 0, 1, 2, 3 };
  const uint8_t amBandWidths[] = { 3, 2, 1, 0 };
  const uint8_t cwBandWidths[] = { 4, 5, 3, 3 };

  const char *amBandwidth[] = { "2KHz", "3KHz", "4KHz", "6KHz" };
  const char *ssbBandwidth[] = { "1.2KHz", "2.2KHz", "3KHz", "4KHz" };
  const char *cwBandwidth[] = { "0.5KHz", "1.0KHz", "", "" };
  const char **bandwidth = nullptr;



  if (modType == WBFM)
    return;

  if (mode == 0) {
    for (int j = 0; j < 4; j++) {
      drawButton(8 + j * 83, 235, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
      drawButton(8 + j * 83, 178, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    }

    tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest row

    etft.setTTFFont(Arial_14);
    etft.setTextColor(TFT_GREEN);

    if (modType == AM)
      bandwidth = amBandwidth;
    else if (modType == USB || modType == LSB || modType == SYNC)
      bandwidth = ssbBandwidth;
    else if (modType == CW)
      bandwidth = cwBandwidth;


    if (bandwidth != nullptr) {
      for (int i = 0; i < 4; ++i) {

        if (modType == AM)
          etft.setCursor(positions[i], TEXT_Y);
        if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
          etft.setCursor(positions[i] - 10, TEXT_Y);  // text is wider

        etft.print(bandwidth[i]);
      }
      etft.setTextColor(textColor);  // Reset to default text color
    }


    tDoublePress();

    delay(10);  // Wait until pressed

    row = 1 + ((ty - 20) / vTouchSpacing);
    column = 1 + (tx / HorSpacing);


    if (row == 3) {

      if (modType == USB || modType == LSB || modType == SYNC) {
        bandWidth = usbLsbBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      } else if (modType == AM) {
        bandWidth = amBandWidths[column - 1];
        lastAMBandwidth = bandWidth;
      } else if (modType == CW) {
        bandWidth = cwBandWidths[column - 1];
        lastSSBBandwidth = bandWidth;
      }
    }

  }  // endif mode == 0


  else {  // mode -1 and mode 1 are used when touching the indicator area on the left or right

    const uint8_t *bandWidths;
    int maxIndex = 3;
    int i = 0;
    bool isIncrement = (mode == 1);

    if (modType == AM) {
      bandWidths = amBandWidths;
    } else if (modType == USB || modType == LSB || modType == SYNC) {
      bandWidths = usbLsbBandWidths;
    } else if (modType == CW) {
      bandWidths = cwBandWidths;
      maxIndex = 1;
    } else {
      return;
    }

    // find index
    while (i < maxIndex && bandWidths[i] != bandWidth) {
      i++;
    }

    // modify index
    if (isIncrement && i < maxIndex) {
      i++;
    } else if (!isIncrement && i > 0) {
      i--;
    }

    // update bandwidth
    bandWidth = bandWidths[i];
  }
  // set bandwidth
  if (modType == LSB || modType == USB || modType == SYNC || modType == CW) {
    si4735.setSSBAudioBandwidth(bandWidth);
    if (bandWidth <= 2) {
      si4735.setSBBSidebandCutoffFilter(0);
    } else {
      si4735.setSBBSidebandCutoffFilter(1);
    }
  }

  if (modType == AM) {
    si4735.setBandwidth(bandWidth, 1);
  }
  printBandWidth();
}

//##########################################################################################################################//

void printBandWidth() {

  if (altStyle)
    tft.fillRoundRect(75, 95, 92, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(92, 26);
    spr.pushImage(0, 0, 92, 26, (uint16_t *)Oval92);
    spr.pushSprite(75, 92);
    spr.deleteSprite();
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(77, 98);

  switch (modType) {
    case WBFM:
      tft.print(" 300KHz");
      break;
    case NBFM:
      tft.print(" 10KHz");
      break;
    case CW:
      if (bandWidth == 4)
        tft.print(" 0.5KHz");
      if (bandWidth == 5)
        tft.print(" 1.0KHz");
      break;
    case USB:
    case LSB:
    case SYNC:
      tft.print(bandWidth == 0 ? " 1.2KHz" : bandWidth == 1 ? " 2.2KHz"
                                           : bandWidth == 3 ? " 4.0KHz"
                                                            : " 3.0KHz");
      break;
    case AM:
      tft.print(bandWidth == 0 ? " 6.0KHz" : bandWidth == 1 ? " 4.0KHz"
                                           : bandWidth == 2 ? " 3.0KHz"
                                                            : " 2.0KHz");
      break;
  }
}
//##########################################################################################################################//


void showTinySAMode() {

  if (altStyle)
    tft.fillRoundRect(174, 95, 45, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(55, 26);
    spr.pushImage(0, 0, 55, 26, (uint16_t *)Oval55);
    spr.pushSprite(170, 92);
    spr.deleteSprite();
  }

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(185, 98);

  if (tinySA_RF_Mode) {
    tft.setTextColor(TFT_YELLOW);
    tft.print("RF");
  } else {
    tft.setTextColor(TFT_GREEN);
    tft.print("IF");
  }

  tft.setTextColor(textColor);
}


//##########################################################################################################################//

void ScanMode() {
  scanMode = !scanMode;

  if (!scanMode)
    showScanRange = true;  // reset to show the range next time it will be set
  drawBigBtns();
}

//##########################################################################################################################//

int getButtonID(void) {


  if (ty > 293)  // no buttons there
    return 0;
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 25) / vTouchSpacing);
  int buttonID = row * 10 + column;
  return buttonID;
}

//##########################################################################################################################//
void low_High_Injection() {  //changes injection mode manually


selectedInjectionMode++;

if (selectedInjectionMode == 1) {
  manualInjectionMode = true; 
  LOAboveRF = false;
} 

if (selectedInjectionMode == 2) {
  manualInjectionMode = true;
  LOAboveRF = true;
} 


if (selectedInjectionMode == 3) { // inject as defined in  struct FilterInfo
  manualInjectionMode = false; 
  LOAboveRF = true; // initially true
  selectedInjectionMode = 0;
} 

  
showInjectionMode(); 

  restartSynth();
  tuneSI5351();


  if (modType == LSB || modType == USB)
    loadSi4735parameters();  // if LOAboveRF correct sideband inversion

  tft.setTextColor(textColor);
  tft.fillRect(4, 294, 300, 25, TFT_BLACK);
  if (LOAboveRF)
    displayText(5, 300, 300, 19, "LO frequency above RF");
  else
    displayText(5, 300, 300, 19, "LO frequency below RF");

  delay(500);
  displayText(5, 300, 300, 19, "                      ");

  displayFREQ(FREQ);

}


//##########################################################################################################################//

void showInjectionMode() {

    etft.setTTFFont(Arial_14);


    const char* injectionText;
    uint16_t color = textColor;

    switch (selectedInjectionMode) {
        case 1:
            injectionText = "Low";
            color = TFT_YELLOW;
            break;
        case 2:
            injectionText = "High";
            color = TFT_GREEN;
            break;
        case 0:
        case 3:
        default:
            injectionText = "Default";
            break;
    }

    tft.fillRect(100, 132, 65, 18, TFT_BLACK);
    etft.setTextColor(color);
    etft.setCursor(100, 132);
    etft.print(injectionText);
    etft.setCursor(96, 151);
    etft.print("Injection");
    etft.setTextColor(textColor);
}
