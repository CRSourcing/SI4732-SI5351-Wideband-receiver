void FourthScreen() { // FourthScreen contains tools and helpers

  if (!altStyle)  // clear  background
    tft.fillRect(2, 61, 337, 228, TFT_BLACK);
  else
    drawButton(2, 61, 337, 228, TFT_NAVY, TFT_DARKGREY);  //Background
  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);


  redrawMainScreen = true;
  drawFoBtns();
  readFoBtns();
}

//##########################################################################################################################//

void drawFoBtns() {

  if (altStyle)
    etft.setTextColor(textColor);
  else
    etft.setTextColor(TFT_ORANGE);

  struct Button {
    int x;
    int y;
    const char* label;
  };

  Button buttons[] = {
    { 20, 132, "Sweep" }, { 20, 153, "0-30M" }, { 105, 130, "Sweep" }, { 105, 151, "200M" }, { 185, 132, "Sweep" }, { 178, 151, "118/137" }, 
    { 265, 132, "Swp" }, { 270, 151, "FM" }, { 20, 188, "Prop." }, { 20, 210, "Test" }, { 105, 188, "Audio" }, { 98, 210, "Spectr." }, { 185, 190, "Station" }, { 188, 210, "Scan" }, { 265, 190, "Audio" }, { 270, 210, "WF" }, { 265, 245, "Touch" }, { 263, 268, "Sound" }, { 183, 245, "Manual" }, { 183, 268, "Filter" }, { 100, 245, "GPIO" }, { 100, 268, "Test" }
  };


  etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_YELLOW);

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


  drawButton(8, 236, 74, 49, TFT_MIDGREEN, TFT_DARKGREEN);
  etft.setCursor(20, 254);
  etft.print("BACK");
  etft.setTextColor(textColor);
  tDoublePress();
}
//##########################################################################################################################//

void readFoBtns() {

  if (!pressed) return;
  int buttonID = getButtonID();

  if (row < 2 || row > 4 || column > 4)
    return;  // outside of area

  switch (buttonID) {
#ifdef TINYSA_PRESENT
    case 21:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 15M");
      delay(100);
      Serial.println("sweep span 30M");
      break;
    case 22:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 100M");
      delay(100);
      Serial.println("sweep span 200M");
      break;
    case 23:
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep start 118M");
      delay(100);
      Serial.println("sweep stop 137M");
      break;
#endif
    case 24: 
      Serial.println("rbw 100");
      delay(100);
      Serial.println("sweep center 98M");
      delay(100);
      Serial.println("sweep span 20M");
      break;
    case 31:
      propertyTest();
      break;
    case 32:
    audioFreqAnalyzer(); // located in FFT.ino
    tRel();
      break;
    case 33:
      slowSan = true;
      SlowScan();   // located in FFT.ino
      tRel();
      tRel(); 
      tx = ty = pressed = 0;
      rebuildMainScreen(0);
      return; 
    case 34:
        showAudioWaterfall = !preferences.getBool("audiowf", 0);
      preferences.putBool("audiowf", showAudioWaterfall);
      tft.setCursor(10, 65);
      tft.printf("Audio Waterfall: %s\n", showAudioWaterfall ? "ON" : "OFF");
      if (showAudioWaterfall){
      tft.setCursor(10, 83);
      tft.printf("after %ds inactivity", TIME_UNTIL_AUDIO_WATERFALL);
      }
       delay(1000);
      break;
    case 41:
      return;
    case 42:
      gpioTest();
      tPress();
      break;
    case 43:
      setFilterManually = !setFilterManually;
      tft.setCursor(10, 75);
      tft.printf("Manual Filter: %s %d\n", setFilterManually ? "Enabled" : "Disabled", filterNumber);
      delay(500);
      if (!setFilterManually)
        return;
      manualFilterSelector();
      return;
      break;
    case 44:
      pressSound = !pressSound;
      preferences.putBool("pressSound", pressSound);
      ;
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

void gpioTest() {  // experimental, set GPIO's manually for filter debugging

  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  int cursorPositions[][2] = {
    { 15, 190 }, { 15, 213 }, { 100, 190 }, { 100, 213 }, { 182, 190 }, { 182, 213 }, { 265, 190 }, { 263, 213 }, { 262, 245 }, { 262, 268 }, { 178, 245 }, { 178, 268 }, { 100, 245 }, { 100, 268 }
  };

  const char* texts[] = {
    "GPIO", " 13", "GPIO", " 14", "GPIO", " 25",
    "GPIO", " 27", "", "", "", "",
    "", ""
  };

  for (int i = 0; i < 14; i++) {
    etft.setCursor(cursorPositions[i][0], cursorPositions[i][1]);
    etft.print(texts[i]);
  }

  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(27, OUTPUT);


  int gpioPins[] = { 13, 14, 25, 27 };
  bool gpioStates[] = { LOW, LOW, LOW, LOW };
  int circles[][2] = {
    { 60, 213 },
    { 140, 213 },
    { 222, 213 },
    { 305, 213 }
  };


  while (true) {
    for (int i = 0; i < 4; i++) {
      gpioStates[i] = digitalRead(gpioPins[i]);
      tft.fillCircle(circles[i][0], circles[i][1], 4, gpioStates[i] ? TFT_GREEN : TFT_RED);
    }

    tRel();

    int buttonID = getButtonID();
    switch (buttonID) {
      case 31:
        gpioStates[0] = !gpioStates[0];
        digitalWrite(gpioPins[0], gpioStates[0]);
        break;
      case 32:
        gpioStates[1] = !gpioStates[1];
        digitalWrite(gpioPins[1], gpioStates[1]);
        break;
      case 33:
        gpioStates[2] = !gpioStates[2];
        digitalWrite(gpioPins[2], gpioStates[2]);
        break;
      case 34:
        gpioStates[3] = !gpioStates[3];
        digitalWrite(gpioPins[3], gpioStates[3]);
        break;
      case 41:
      case 44:
        redrawMainScreen = true;
        tx = ty = pressed = 0;
        return;
      case 42:
      case 43:
        break;
    }
    tx = ty = pressed = 0;
    delay(100);
  }
}



//##########################################################################################################################//

void propertyTest() { // experimental for SI4735 properties 

  static uint16_t val = 0x00;

  encLockedtoSynth = false;
  clearStatusBar();
  displayText(10, 75, 0, 0, "Property 0x3703 test");
  displayText(10, 90, 0, 0, "AGC Release Rate in ms");

  while (digitalRead(ENC_PRESSED) == HIGH) {

    delay(50);
    if (clw)
      val += 4;
    if (cclw)
      val -= 4;

    if (clw || cclw) {
    si4735.setProperty(0x3703, val);
    clw = false;
    cclw = false; 
  
  tft.fillRect(10,75, 300, 16, TFT_BLACK);
  tft.setCursor (10,75);
  tft.printf("Value: %d", val);
    }

  }
  while (digitalRead(ENC_PRESSED) == LOW)  
    ;
  encLockedtoSynth = true;
  clearStatusBar();
}