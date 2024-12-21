//##########################################################################################################################//
void setMode() {  // gets called from loop, selects btw. normal or scan mode. Changes btw normal and scan mode. Calls selectModulation() too.

  static uint16_t dly = 0;
  static bool stop = false;
  const int btLeftBorder = 350;


  if (!scanMode && lim1 && lim2) {  // reset scan parameters
    lim1 = 0;
    lim2 = 0;
    showScanRange = false;
    stop = true;
  }

  if (tx > btLeftBorder && !scanMode) {  // UP or DOWN pressed


    if (!pressed)
      return;
    disableFFT = true;
    freq_UP_DOWN();
  }


  if (scanMode) {
    if (keyVal && showScanRange && lim1 && lim2) {
      tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite row 3 to make room for scan parameters
      printScanRange();                          // print scan parameters once (not in every cycle)
      showScanRange = false;
    }

    si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength to compare against squelch
    signalStrength = si4735.getCurrentRSSI();
    SNR = si4735.getCurrentSNR();


    if ((!SNRSquelch && (signalStrength > currentSquelch)) || (SNRSquelch && SNR) || clw || cclw) {  // Scan stops when signal or encoder moved
      stop = true;
      dly = 0;
    }


    if ((signalStrength < currentSquelch && keyVal == 2) || (SNRSquelch && !SNR && keyVal == 2))  // Scan continues when signal drops
      dly++;

    if (dly == 50) {  // insert some seconds of delay before continuing to scan
      stop = false;
      dly = 0;
    }

    if (pressed)
      stop = false;

    if (lim1 && lim2) {
      if (FREQ >= lim1)  // keep frequency within boundaries
        FREQ = lim2;
      if (FREQ < lim2)
        FREQ = lim1;
    }

    SCANfreq_UP_DOWN(stop);
  }

  if ((ty > 135 && ty < 205) && !scanMode && tx > btLeftBorder) {  // Mode button pressed
    tRel();
    selectModulation();
  }
}

//##########################################################################################################################//

void selectModulation() {

  int lastmodType = modType;
  int h = 8;


  for (int j = 0; j < 4; j++) {
    drawButton(h + j * 83, 235, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
    drawButton(h + j * 83, 178, TILE_WIDTH, TILE_HEIGHT, TFT_BTNCTR, TFT_BTNBDR);
  }

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest row
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(21, 198);
  tft.print("AM");
  tft.setCursor(105, 198);
  tft.print("LSB");
  tft.setCursor(188, 198);
  tft.print("USB");
  tft.setCursor(280, 198);
  tft.print(" ");
  tft.setCursor(20, 254);
  tft.print("WBFM");
  tft.setCursor(275, 198);
  tft.print("NBFM");
  tft.setCursor(188, 255);
  tft.print("CW");
  tft.setCursor(280, 245);
  tft.print("CW");
  tft.setCursor(275, 265);
  tft.print("DECO");
  tft.setCursor(105, 255);
  tft.print("SYNC");

  tPress();


  uint16_t row = 0, column = 0;
  column = 1 + (tx / HorSpacing);  // get row and column
  row = 1 + ((ty - 20) / vTouchSpacing);
  int buttonID = row * 10 + column;

  switch (buttonID) {
    case 31:
      modType = AM;
      break;
    case 32:
      modType = LSB;
      break;
    case 33:
      modType = USB;
      break;
    case 34:
      modType = NBFM;
      break;
    case 41:
      lastAMFREQ = FREQ;  // save last non WBFM FREQ
      modType = WBFM;
      WBFMactive = true;
      break;
    case 42:
      modType = SYNC;
      break;
    case 43:
      modType = CW;
      break;
    case 44:
      modType = CW;  // CW decoder
      loadSi4735parameters();
      CWDecoder();
      rebuildMainScreen(1);
      loadSi4735parameters();
      modType = USB;  // need to change so that the decoder gets reloaded when selected CW again
      STEP = DEFAULT_SSB_STEP;
      si4735.setSSBAudioBandwidth(3);  // 4 KHz
      break;
    default:
      break;
  }


  if (modType != lastmodType) {
    loadSi4735parameters();
    uint8_t vol = preferences.getChar("Vol", 0);  // need to reload volume after changind modulation
    si4735.setVolume(vol);
  }  // end if (modType != lastmodType)
  redrawMainScreen = true;
  tx = ty = pressed = 0;
  mainScreen();
}



//##########################################################################################################################//

void loadSi4735parameters() {


  if (modType != WBFM && WBFMactive == true) {  // restore last AM or SSB frequency and the step when coming from WBFM
    FREQ = lastAMFREQ;
    WBFMactive = false;
  }

  if (modType == AM) {
    SI4735directTune = false;
    bandWidth = lastAMBandwidth;
    STEP = DEFAULT_AM_STEP;
    roundFreqToSTEP();
    ssbLoaded = false;
    si4735.setAM(520, 29900, SI4735TUNED_FREQ, 1);
    si4735.setBandwidth(bandWidth, 1);
    //AMCHFLT	the choices are: 0 = 6 kHz Bandwidth 1 = 4 kHz Bandwidth 2 = 3 kHz Bandwidth 3 = 2 kHz Bandwidth 4 = 1 kHz Bandwidth 5 = 1.8 kHz Bandwidth 6 = 2.5 kHz Bandwidth, gradual roll off 7–15 = Reserved (Do not use).
    //AMPLFLT	Enables the AM Power Line Noise Rejection Filter.
  }

  if (modType == LSB || modType == USB || modType == SYNC) {
    SI4735directTune = false;
    STEP = DEFAULT_SSB_STEP;
    roundFreqToSTEP();
    bandWidth = lastSSBBandwidth;
    loadSSB();
  }




  //usblsb	upper or lower side band; 1 = LSB; 2 = USB

  int offset = 0;

  if (modType == USB && LOAboveRF)
    offset = preferences.getInt("B1", 0);
  if (modType == USB && !LOAboveRF)
    offset = preferences.getInt("B2", 0);

  if (modType == LSB && LOAboveRF)
    offset = preferences.getInt("B3", 0);
  if (modType == LSB && !LOAboveRF)
    offset = preferences.getInt("B4", 0);





  if (modType == USB) {
    if (LOAboveRF) {                                     // sidebands get inverted if LOAboveRF
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 1);  // Set LSB mode, USB with sideband inverted == LSB
      si4735.setSSBBfo(offset);
    } else {
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  // Set USB mode
      si4735.setSSBBfo(offset);
    }
  }



  if (modType == LSB) {
    if (LOAboveRF) {                                     // sidebands get inverted if LOAboveRF
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  // set USB, LSB with sideband inverted == USB //
      si4735.setSSBBfo(offset);
    } else {
      si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 1);
      si4735.setSSBBfo(offset);  // Normal LSB
    }
  }



  if (modType == SYNC) {
    STEP = DEFAULT_SYNC_STEP;
    roundFreqToSTEP();
    si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);  // SYNC uses USB mode
    si4735.setSSBConfig(bandWidth, 1, 3, 1, 0, 0);     // SYNC config mode
    si4735.setSSBBfo(preferences.getInt("SYNCBBfoOffset", 0));
    loadSSB();
    ssbLoaded = false;  // Reload SSB when changing to USB/LSB to disable AFC
  }
  if (modType == CW) {
    loadSSB();
    STEP = DEFAULT_CW_STEP;
    bandWidth = 5;  // 1KHz
    SI4735directTune = false;
    loadSSB();
    si4735.setSSB(520, 29900, SI4735TUNED_FREQ, 1, 2);
    si4735.setSSBBfo(preferences.getInt("CWBfoOffset", 500));
    si4735.setSSBAudioBandwidth(5);  // 1KHz
    si4735.setSBBSidebandCutoffFilter(0);
    ssbLoaded = false;  // need to reload SSB bandwidth
  }


  if (modType == NBFM) {  // NBFM is AM, but shifts IF  so that the center of the signal is on the slope of the crystal filter
    SI4735directTune = false;
    bandWidth = lastAMBandwidth;
    STEP = DEFAULT_NBFM_STEP;
    roundFreqToSTEP();
    ssbLoaded = false;
    si4735.setAM(520, 29900, SI4735TUNED_FREQ + NBFM_IF_OFFSET, 1);
    si4735.setBandwidth(bandWidth, 1);
  }


  if (modType == WBFM) {
    SI4735directTune = true;
    ssbLoaded = false;
    loopBands = false;  // deactivate band limits
    STEP = DEFAULT_WBFM_STEP;
    FREQ = FMSTARTFREQ * 10000;
    FREQ_OLD = FREQ;             // avoid initial  si4735.frequencyUp()
    si4735.setup(RESET_PIN, 0);  // FM Mode
    delay(10);
    si4735.setFM(6400, 10800, FMSTARTFREQ, DEFAULT_WBFM_STEP / 10000);
    si4735.setFmStereoOff();
    si4735.setVolume(preferences.getChar("Vol", 50));  // Switch to WBFM sometimes lowers volume. Why??  This is to set the volume back.
    displayFREQ(FREQ);
    settleSynth();
    tuneSI5351();  // initialize VFO so that spectrum analyzer is tuned
    return;
  }

  if (modType == LSB || modType == USB || modType == SYNC || modType == CW)
    si4735.setSsbAgcOverrite(1, AGCIDX);  // disable AGC to eliminateSSB humming noise

  si4735.setAMSoftMuteSnrThreshold(preferences.getChar("SMute", 0));  //needs to be reloaded
  si4735.setAvcAmMaxGain(preferences.getChar("AVC", 0));              //needs to be reloaded
}
//##########################################################################################################################//
void printModulation() {


  if (altStyle)
    tft.fillRoundRect(11, 95, 55, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(55, 26);
    spr.pushImage(0, 0, 55, 26, (uint16_t *)Oval55);
    spr.pushSprite(11, 92);
    spr.deleteSprite();
  }


  tft.setCursor(20, 98);
  tft.setTextColor(TFT_GREEN);
  switch (modType) {

    case WBFM:
      tft.print("WFM");
      break;
    case AM:
      tft.print("AM");
      break;
    case NBFM:
      tft.print("NFM");
      break;
    case LSB:
      tft.print("LSB");
      break;
    case USB:
      tft.print("USB");
      break;
    case SYNC:
      tft.print("SYN");
      break;
    case CW:
      tft.print("CW");
      break;
  }
  tft.setTextColor(textColor);
}

//###############################################################################################//

void use1MhzEncoderStep() {  // press encoder for 1MHz steps, press again, or use a marker to get back to previous step sizw. 

  if (digitalRead(ENC_PRESSED) == LOW && use1MHzSteps == false)  {
    use1MHzSteps = true;
    OLDSTEP = STEP;
    STEP = 1000000;
    displaySTEP();  
    while (digitalRead(ENC_PRESSED) == LOW)
      ;
  }

  if (digitalRead(ENC_PRESSED) == LOW && use1MHzSteps == true)  {
    use1MHzSteps = false;
    STEP = OLDSTEP;
    displaySTEP();  
    while (digitalRead(ENC_PRESSED) == LOW)
      ;
  }
}

//###############################################################################################//
