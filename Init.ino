// initialize HW, draw permanent buttons and  screen elements

void bootScreen() {
  int16_t si4735Addr = 0;
  tft.init();
  tft.setRotation(1);
  tft.setTextColor(TFT_FOREGROUND);
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);

  bool fastBoot = preferences.getBool("fastBoot", 0);
  if (!fastBoot) {
    tft.setTextSize(3);
    tft.setCursor(0, 30);
    tft.println("SI4732 + SI5351\n\nDSP Wideband Receiver\n");
    tft.setTextSize(2);
    si5351.update_status();
    tft.setCursor(0, 130);
    tft.print("SI5351 status: ");
    tft.print(si5351.dev_status.SYS_INIT);
    tft.print(si5351.dev_status.LOL_A);
    tft.print(si5351.dev_status.LOL_B);
    tft.print(si5351.dev_status.LOS);

    tft.setCursor(0, 160);
    if (si4735Addr == 0) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.print("Si4732 not detected");
    } else {
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      Serial.println("Si4732 found");
      tft.print("Si4732 found, Addr: ");
      tft.println(si4735Addr, HEX);
    }

    tft.setTextColor(TFT_GREY);
    tft.setCursor(0, 260);
    tft.print("Touch screen to see settings");
    tft.setTextColor(textColor);
    tft.setCursor(0, 200);
    tft.print(ver);

    delay(2000);
  }

  if (si4735Addr == 17) {
    si4735.setDeviceI2CAddress(0);
  } else {
    si4735.setDeviceI2CAddress(1);
  }
  preferences.putBool("fastBoot", 0);  // remove fastboot flag
}

//##########################################################################################################################//

void spriteBorder() {

  spr.createSprite(480, 1);
  spr.pushImage(0, 0, 480, 1, (uint16_t *)border480);
  spr.pushSprite(0, 0);
  spr.pushSprite(0, 1);
  spr.pushSprite(0, 292);
  spr.pushSprite(0, 293);
  spr.deleteSprite();

  spr.createSprite(1, 292);
  spr.pushImage(0, 0, 1, 292, (uint16_t *)border320);
  spr.pushSprite(0, 0);
  spr.pushSprite(1, 0);
  spr.pushSprite(479, 0);
  spr.pushSprite(478, 0);
  spr.deleteSprite();
}

//##########################################################################################################################//

void drawFrame() {

  tft.fillScreen(TFT_BLACK);
  spriteBorder();  // loade border lines
  tft.fillRect(0, 0, DISP_WIDTH, DISP_HEIGHT, TFT_BLACK);
  tft.drawFastHLine(0, 46, DISP_WIDTH, TFT_GRID);
  tft.drawFastVLine(340, 46, 246, TFT_GRID);
  tft.fillCircle(162 + 13, 47, 3, TFT_GREEN);  //draw a circle to display finetune pot center
}


//##########################################################################################################################//

void SI5351_Init() {

  tft.setTextSize(2);

  for (int i = 0; i < 10; i++) {
    bool i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
    if (!i2c_found) {
      tft.setCursor(20, 50 + 20 * i);
      tft.print("SI5351 not detected!");
      tft.println();
      delay(1000);
    }
  }
  si5351.set_correction(preferences.getLong("calib", 0), SI5351_PLL_INPUT_XO);  // read calibration from preferences
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLB);                               // Clock for RX high band // assign to the other PLL to avoid 100 MHz issue
  si5351.output_enable(SI5351_CLK2, 1);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA);  // Clock for RX low band
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);  // Clock for RX high band
  si5351.output_enable(SI5351_CLK1, 0);                  //Clock for TX
  si5351.output_enable(SI5351_CLK0, 0);
}
//##########################################################################################################################//


void radioInit() {

  si4735.setAudioMuteMcuPin(MUTEPIN);
  digitalWrite(MUTEPIN, HIGH);
  si4735.setup(RESET_PIN, 1);

  if (modType == USB || modType == LSB)
    STEP = DEFAULT_SSB_STEP;
  else if (modType == CW)
    STEP = DEFAULT_CW_STEP;
  else if (modType == SYNC)
    STEP = DEFAULT_SYNC_STEP;
  else if (modType == AM)
    STEP = DEFAULT_AM_STEP;
  else if (modType == NBFM)
    STEP = DEFAULT_NBFM_STEP;
  loadSi4735parameters();
  si4735.setVolume(preferences.getChar("Vol", 50));
  si4735.setAMSoftMuteSnrThreshold(preferences.getChar("SMute", 0));
  si4735.setAvcAmMaxGain(preferences.getChar("AVC", 0));
  digitalWrite(MUTEPIN, LOW);
}

//##########################################################################################################################//

void tinySAInit() {


  //Serial.println("color 8 0x0009ff"); // trace 3 blue
  //delay(50);

#ifdef TINYSA_PRESENT
  if (tinySA_RF_Mode) {
    loadRFMode();
    delay(50);
    Serial.println("marker 2 peak");
  } else
    loadIFMode();


#endif
}

/*
tinySA support color console command (i port it fom NanoVNA code):
color index hex_color

For get color in hex you can use any soft like
https://colorscheme.ru/color-converter.html

Example for change trace 1 color to hex = #de0707:
'color 6 0xde0707'

read current colors:
'color'
Color indexes:
#define LCD_BG_COLOR             0
#define LCD_FG_COLOR             1
#define LCD_GRID_COLOR           2
#define LCD_MENU_COLOR           3
#define LCD_MENU_TEXT_COLOR      4
#define LCD_MENU_ACTIVE_COLOR    5
#define LCD_TRACE_1_COLOR        6
#define LCD_TRACE_2_COLOR        7
#define LCD_TRACE_3_COLOR        8
#define LCD_TRACE_4_COLOR        9
#define LCD_NORMAL_BAT_COLOR    10
#define LCD_LOW_BAT_COLOR       11
#define LCD_TRIGGER_COLOR       12
#define LCD_RISE_EDGE_COLOR     13
#define LCD_FALLEN_EDGE_COLOR   14
#define LCD_SWEEP_LINE_COLOR    15
#define LCD_BW_TEXT_COLOR       16
#define LCD_INPUT_TEXT_COLOR    17
#define LCD_INPUT_BG_COLOR      18
#define LCD_BRIGHT_COLOR_BLUE   19
#define LCD_BRIGHT_COLOR_RED    20
#define LCD_BRIGHT_COLOR_GREEN  21
#define LCD_DARK_GREY           22
#define LCD_LIGHT_GREY          23
#define LCD_HAM_COLOR           24
#define LCD_GRID_VALUE_COLOR    25
#define LCD_M_REFERENCE         26
#define LCD_M_DELTA             27
#define LCD_M_NOISE             28
#define LCD_M_DEFAULT           29

*/



//##########################################################################################################################//

void drawBigBtns() {


  drawButton(345, 49, 130, 78, TFT_BLUE, TFT_NAVY);   // UP button
  drawButton(345, 209, 130, 78, TFT_BLUE, TFT_NAVY);  // DOWN button
  drawButton(345, 129, 130, 78, TFT_BLUE, TFT_NAVY);  // MODE button
  
  tft.setTextSize(3);

  if (!scanMode) {

    tft.setTextColor(TFT_GREEN);
    tft.setCursor(375, 158);
    tft.print("MODE");
    tft.setCursor(390, 80);
    tft.print("UP");
    tft.setCursor(375, 240);
    tft.print("DOWN");
  }

  else {
    tft.setTextColor(TFT_SKYBLUE);
    tft.setCursor(370, 145);
    tft.print("SET");
    tft.setCursor(370, 175);
    tft.print("RANGE");
    tft.setCursor(375, 65);
    tft.print("SEEK");
    tft.setCursor(375, 98);
    tft.print("UP");
    tft.setCursor(375, 220);
    tft.print("SEEK");
    tft.setCursor(375, 255);
    tft.print("DOWN");
    tft.setTextSize(2);
    tft.setTextColor(textColor);
  }
  tft.setTextSize(2);
}


//##########################################################################################################################//

void loadLastSettings() {

  FREQ = preferences.getLong("lastFreq", 0);                 // load last Freq
  bandWidth = preferences.getInt("lastBw", 0);               // last bandwidth
  modType = preferences.getChar("lastMod", 0);               //last modulation type
  altStyle = preferences.getBool("lastStyle", 0);            //plain or sprite style
  pressSound = preferences.getBool("pressSound", 0);         // short beep when pressed
  miniWindowMode = preferences.getChar("spectr", 0);         // audio spectrum analyzer mode
  sevenSeg = preferences.getBool("sevenSeg", 0);             // frequency display font
  syncEnabled = preferences.getBool("useTSADBm", 0);         // use tinySA for DBm 
  SNRSquelch = preferences.getBool("SNRSquelch", 0);         // SNR controls squelch, this is less sensible, but does not need squelchpot. Does not work with NBFM signals
  buttonSelected = preferences.getInt("sprite", 4);          // load sprite for buttons
  tinySA_RF_Mode = preferences.getBool("tinySARFMode", 0);   // set tinySA mode
  loopBands = preferences.getBool("uBL", 0);                 // use band limits or not
  smoothColorGradient = preferences.getBool("smoothWF", 0);  // smooth waterfall colors
  SI4735TUNED_FREQ = preferences.getLong("IF", 21397);        // IF in KHz
  showAudioWaterfall = preferences.getBool("audiowf", 0);    // Audio waterfall after 2 minutes of inactivity
  pressed = get_Touch();

  if (pressed) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println("Settings loaded:\n");
    tft.printf("FREQ: %ld Khz\n", FREQ / 1000);
    tft.printf("IF: %d Khz\n", SI4735TUNED_FREQ);
    tft.printf("Mod Type: %d\n", modType);
    tft.printf("Alt Style: %s\n", altStyle ? "Enabled" : "Disabled");
    tft.printf("Touch Sound: %s\n", pressSound ? "On" : "Off");
    tft.printf("Spectrum Mode: %d\n", miniWindowMode);
    tft.printf("Seven Seg Font: %s\n", sevenSeg ? "Enabled" : "Disabled");
    tft.printf("TinySA Integration: %s\n", syncEnabled ? "Yes" : "No");
    tft.printf("SNR Squelch: %s\n", SNRSquelch ? "Enabled" : "Disabled");
    tft.printf("Sprite Style: %d\n", buttonSelected);
    tft.printf("tinySA Mode: %s\n", tinySA_RF_Mode ? "RF" : "IF");
    tft.printf("Loop bands when tuning: %s\n", loopBands ? "Yes" : "No");
    tft.printf("Smooth Waterfall Colors: %s\n", smoothColorGradient ? "Yes" : "No");
    tft.printf("Audio Waterfall: %s\n", showAudioWaterfall ? "Yes" : "No");
    tft.printf("Touch tune mode: %d\n", preferences.getChar("tGr", 0));
    tft.printf("Fastboot: %d\n", preferences.getBool("fastBoot", 0));
    tft.print("\nTouch to continue...");
    tDoublePress();
  }
}

//##########################################################################################################################//