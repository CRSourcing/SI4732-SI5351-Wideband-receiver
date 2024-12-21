//#########################################################################################################################//

void setScanRange() {

  if (!getFreqLimits()) {  // keypad return pressed
    redrawMainScreen = true;
    return;
  }
  tft.fillRect(5, 3, 330, 40, TFT_BLACK);
  tft.setCursor(12, 7);
  tft.print("1 and RETURN = stop mode");
  tft.setCursor(12, 26);
  tft.print("2 and RETURN = cont. mode");
  tPress();
  readKeypadButtons();
  tft.fillRect(5, 3, 330, 40, TFT_BLACK);
  redrawMainScreen = true;
  mainScreen();
  tft.setCursor(10, 123);
  printScanRange();
  ty = 100;  // changes ty so that SCANfreq_UP_DOWN() assumes a SEEK UP press and starts to scan from low to high freeq
}

//##########################################################################################################################//

void printScanRange() {

  if (showScanRange == false)
    return;

  tft.fillRect(3, 119, 334, 58, TFT_BLACK);  // overwrite highest button row
  tft.setCursor(10, 123);
  if (keyVal == 1)
    tft.print("Stay when signal drops");
  if (keyVal == 2)
    tft.print("Move on when signal drops");
  tft.setCursor(10, 150);
  tft.printf("Range:%ldKHz - %ldKHz", lim2 / 1000, lim1 / 1000);
}
//##########################################################################################################################//

void SCANfreq_UP_DOWN(bool stop) {  // changes Freq when SCAN buttons are pressed

  int16_t stpsize;
  if ((STEP > DEFAULT_AM_STEP) && (modType == AM))
    stpsize = DEFAULT_AM_STEP;
  else
    stpsize = STEP;

  if (ty > 47 && ty < 131 && tx > 350 && FREQ < (HI_FREQ - STEP) && !stop) {  // UP button
    FREQ += stpsize;
  }

  if (ty > 210 && ty < 290 && tx > 350 && FREQ > (LOW_FREQ + STEP) && !stop) {  // Down button
    FREQ -= stpsize;
  }

  if (pressed && tx > 350 && ty > 135 && ty < 205)  // SET RANGE button
    setScanRange();
}


//##########################################################################################################################//

void freq_UP_DOWN() {  // changes freq when up or down is pressed, starts slow and gets faster

  long stpsize;
  static int dly = 150;
  static long before;
  long cycleTime;
  long now = millis();  // measure loop time
  cycleTime = now - before;
  before = now;


  if ((STEP > DEFAULT_AM_STEP) && (modType == AM) && use1MHzSteps == false)
    stpsize = DEFAULT_AM_STEP;
  else
    stpsize = STEP;

  si4735.getCurrentReceivedSignalQuality(0);  // fetch signal strength to slow sdown when signal detected
  signalStrength = si4735.getCurrentRSSI();

  if (ty > 20 && ty < 131 && FREQ < (HI_FREQ - STEP)) {  // UP
    FREQ += stpsize;
  }
  if (ty > 210 && ty < 290 && FREQ > (LOW_FREQ + STEP)) {  // DOWN
    FREQ -= stpsize;
  }
  delay(dly + signalStrength);  // slow down when we have a signal

  dly -= 10;

  if (dly <= 5)
    dly = 5;

  if (cycleTime > 300)  // touch was released
    dly = 150;          // start slow again
}


//##########################################################################################################################//


bool getFreqLimits() {
  bool result;

  drawNumPad();
  drawKeypadButtons();
  displayText(12, 10, 320, 33, "Enter low limit");
  tPress();  // wait until pressed
  result = readKeypadButtons();
  if (!result)
    return false;

  lim1 = FREQ;
  displayText(12, 10, 320, 33, "Enter high limit:");
  tPress();
  result = readKeypadButtons();
  if (!result)
    return false;
  lim2 = FREQ;

  if (lim2 > lim1) {  // invert order
    long temp = lim2;
    lim2 = lim1;
    lim1 = temp;
  }

  return true;
}

//##########################################################################################################################//
