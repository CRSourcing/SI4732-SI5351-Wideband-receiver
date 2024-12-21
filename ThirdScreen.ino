void ThirdScreen() {
  clearStatusBar();
  redrawMainScreen = true;
  drawThiBtns();
  readThiBtns();
}

//##########################################################################################################################//

void drawThiBtns() {

  draw12Buttons(TFT_VIOLET, TFT_PURPLE);
  etft.setTTFFont(Arial_14);

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_PINK);


  struct Button {
    int x;
    int y;
    const char *text;
  };

  Button buttons[] = {
    { 18, 134, "Smooth" }, { 20, 152, "Waterf." }, { 105, 132, "7Seg" }, { 105, 152, "Font" }, { 188, 132, "Clock" }, { 190, 151, "Out" }, { 270, 132, "SNR" }, { 270, 151, "Sqlch" }, { 16, 190, "Set IF" }, { 16, 210, "SI4732" }, { 105, 190, "Loop" }, { 105, 210, "Bands" }, { 190, 190, "Set" }, { 190, 210, "AVC" }, { 272, 190, "SSB" }, { 267, 210, "Cutoff" }, { 267, 245, "Calib" }, { 267, 268, "5351" }, { 182, 245, "Master" }, { 178, 268, "Volume" }, { 100, 245, "Set" }, { 100, 268, "SMUTE" }
  };


  for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
    etft.setCursor(buttons[i].x, buttons[i].y);
    etft.print(buttons[i].text);
  }

  drawButton(8, 236, 75, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setTextColor(TFT_GREEN);
  etft.setCursor(20, 245);
  etft.print("Back /");
  etft.setCursor(20, 267);
  etft.print("More");
  etft.setTextColor(textColor);
  tDoublePress();
}


//##########################################################################################################################//

void readThiBtns() {
  if (!pressed) return;

  int buttonID = getButtonID();

  if (row > 4 || column > 4) return;  // outside of key area

  static uint8_t filter = false;
  tft.setTextColor(TFT_GREEN);
  switch (buttonID) {
    case 21:
      smoothColorGradient = !smoothColorGradient;
      preferences.putBool("smoothWF", smoothColorGradient);
      tft.setCursor(10, 75);
      tft.printf("Smooth waterf. colors: %s", smoothColorGradient ? "Yes" : "No");
      delay(1000);
      break;
    case 22:
      tft.fillRect(5, 5, 325, 38, TFT_BLACK);
      sevenSeg = !sevenSeg;
      displayFREQ(FREQ);
      preferences.putBool("sevenSeg", sevenSeg);
      break;

    case 23:
      setClockMode();
      break;

    case 24:
      SNRSquelch = !SNRSquelch;  //  SNRSquelch use SNR to trigger audio mute and the squelch pot will not be used. This only works in AM mode
      preferences.putBool("SNRSquelch", SNRSquelch);
      tft.setCursor(10, 75);
      tft.printf("Use SNR for Squelch: %s", SNRSquelch ? "Yes" : "No");
      delay(1000);
      break;

    case 31:
      setIF();
      break;
    case 32:
      loopBands = preferences.getBool("uBL", 0);  // activate or deactivate band limits
      if (loopBands) {
        loopBands = false;
        preferences.putBool("uBL", loopBands);
        tft.setCursor(10, 75);
        tft.print("Band looping disabled!");
        delay(1000);
        return;
      }
      if (!loopBands) {
        loopBands = true;
        preferences.putBool("uBL", loopBands);
        tft.setCursor(10, 75);
        tft.print("Band looping enabled!");
        delay(1000);
        return;
      }
      break;
    case 33:
      setAvcAmMaxGain();
      break;
    case 34:
      si4735.setSSBSidebandCutoffFilter(filter);
      tft.setCursor(10, 75);
      filter = !filter;
      tft.printf("SSB Cutoff: %s\n", filter ? "Enabled" : "Disabled");
      delay(1000);
      break;
    case 41:
      clearStatusBar();
      displayText(10, 75, 0, 0, "Long press for more");
      if (longPress())
        FourthScreen();

      else {
        return;
      }
      break;
    case 42:
      setSMute();
      break;
    case 43:
      setVol();
      break;
    case 44:
      calibSI5351();
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

void calibSI5351() {  // calibrates the SI5351. I tried two, one had a value of 119000, the other one 153000

  long calibr = preferences.getLong("calib", 1200000);  // load last calibration factor

  encLockedtoSynth = false;
  clearStatusBar();
  while (digitalRead(ENC_PRESSED) == HIGH) {
    clearNotification();

    if (clw)
      calibr += 200;
    if (cclw)
      calibr -= 200;
    clw = 0;
    cclw = 0;

    si5351.set_correction(calibr, SI5351_PLL_INPUT_XO);
    tft.setCursor(5, 75);
    tft.printf("SI5351 calibration.:%ld ", calibr);
    delay(50);
  }
  preferences.putLong("calib", calibr);
  encLockedtoSynth = true;
}

//##########################################################################################################################//

void setVol() {  //sets SI4732 volume. For FFT to work the volume needs to be around 50

  uint8_t vol = preferences.getChar("Vol", 0);
  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 75, 0, 0, "Use encoder to set volume");
  delay(1000);
  clearNotification();
  while (digitalRead(ENC_PRESSED) == HIGH) {
    if (vol > 63)
      vol = 0;
    if (vol < 1)
      vol = 1;
    clearNotification();
    tft.setCursor(10, 75);
    tft.printf(" Master Volume: %d", vol);
    delay(50);
    if (clw) {
      vol++;
      si4735.setVolume(vol);
    }

    if (cclw) {
      vol--;
      si4735.setVolume(vol);
    }

    clw = false;
    cclw = false;
  }
  preferences.putChar("Vol", vol);
  while (digitalRead(ENC_PRESSED) == LOW)  // wait so that it does not jump into the setStep function
    ;
  encLockedtoSynth = true;
}
//##########################################################################################################################//

void setIF() {  // adjusts 21.4 MHz IF up and down so that center frequency of crystal filter can be matched

  encLockedtoSynth = false;
  audioMuted = false;

  displayText(10, 75, 0, 0, "Use encoder to set IF.");
  delay(1000);
  clearNotification();
  tft.setCursor(10, 75);
  tft.printf("IF %d KHz", si4735.getCurrentFrequency());

  while (digitalRead(ENC_PRESSED) == HIGH) {

    delay(50);

    if (clw) {
      FREQ++;
      si4735.setFrequencyUp();
    }
    if (cclw) {
      FREQ--;
      si4735.setFrequencyDown();
    }

    clearNotification();
    tft.setCursor(5, 75);
    SI4735TUNED_FREQ = si4735.getCurrentFrequency();
    tft.printf("IF: %d KHz", SI4735TUNED_FREQ);
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENC_PRESSED) == LOW)
    ;
  preferences.putLong("IF", SI4735TUNED_FREQ);
  encLockedtoSynth = true;
}

//##########################################################################################################################//
void setAvcAmMaxGain() {


  encLockedtoSynth = false;
  displayText(10, 75, 0, 0, "Use encoder to change AVC");

  uint8_t AvcAmMaxGain = preferences.getChar("AVC", 0);
  si4735.setAvcAmMaxGain(AvcAmMaxGain);

  while (digitalRead(ENC_PRESSED) == HIGH) {

    delay(20);
    if (clw)
      AvcAmMaxGain += 10;
    if (cclw)
      AvcAmMaxGain -= 10;

    if (AvcAmMaxGain > 127)
      AvcAmMaxGain = 0;

    si4735.setAvcAmMaxGain(AvcAmMaxGain);

    if (clw != cclw) {
      printAvcAmMaxGain(AvcAmMaxGain);
    }
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENC_PRESSED) == LOW)
    ;

  preferences.putChar("AVC", AvcAmMaxGain);

  encLockedtoSynth = true;
}

//##########################################################################################################################//

void printAvcAmMaxGain(uint8_t AvcAmMaxGain) {

  tft.setTextColor(TFT_GREEN);

  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(102, 26);
    spr.pushImage(0, 0, 102, 26, (uint16_t *)Oval102);
    spr.pushSprite(225, 92);
    spr.deleteSprite();
  }

  tft.setCursor(235, 98);
  tft.printf("AVC:%d", AvcAmMaxGain);
  tft.setTextColor(textColor);
}

//##########################################################################################################################//


void setSMute() {

  static uint8_t AMSoftMuteSnrThreshold = 0;

  //void SI4735::setAMSoftMuteSlop	(	uint8_t 	parameter	)
  //void SI4735::setAmSoftMuteMaxAttenuation	(	uint8_t 	smattn = 0	)
  //Specified in units of dB. Default maximum attenuation is 8 dB. It works for AM and SSB.

  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 75, 0, 0, "Set AM SoftMuteSnrThreshold");

  AMSoftMuteSnrThreshold = preferences.getChar("SMute", 0);
  si4735.setAMSoftMuteSnrThreshold(AMSoftMuteSnrThreshold);

  while (digitalRead(ENC_PRESSED) == HIGH) {

    delay(50);
    if (clw)
      AMSoftMuteSnrThreshold += 4;
    if (cclw)
      AMSoftMuteSnrThreshold -= 4;

    if (AMSoftMuteSnrThreshold > 63)
      AMSoftMuteSnrThreshold = 0;

    si4735.setAMSoftMuteSnrThreshold(AMSoftMuteSnrThreshold);

    if (clw != cclw) {
      printSmute(AMSoftMuteSnrThreshold);
    }
    clw = false;
    cclw = false;
  }

  while (digitalRead(ENC_PRESSED) == LOW)  // wait so that it does not jump into the setStep function
    ;
  preferences.putChar("SMute", AMSoftMuteSnrThreshold);
  encLockedtoSynth = true;
  clearStatusBar();
}

//##########################################################################################################################//

void printSmute(uint8_t AMSoftMuteSnrThreshold) {

  tft.setTextColor(TFT_GREEN);

  if (altStyle)
    tft.fillRoundRect(225, 95, 100, 22, 10, TFT_BLUE);

  else {

    spr.createSprite(102, 26);
    spr.pushImage(0, 0, 102, 26, (uint16_t *)Oval102);
    spr.pushSprite(225, 92);
    spr.deleteSprite();
  }
  tft.setCursor(232, 98);
  tft.printf("SMut:%d ", AMSoftMuteSnrThreshold / 4);
  tft.setTextColor(textColor);
}


//##########################################################################################################################//
