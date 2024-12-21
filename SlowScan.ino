
// slowly scan through struct SlowScanFrequencies
void SlowScan() {

  if (!slowSan)
    return;

  syncStatus = syncEnabled;                  // save sync status
  syncEnabled = false;                       // do not use tinySA for Smeter
  tft.fillRect(330, 4, 145, 20, TFT_BLACK);  // Overwrite "No tinySA message"
  audioMuted = false;
  si4735.setAudioMute(false);                // needs audio signal 
  si4735.setHardwareAudioMute(false);
  modType = 1;  //use AM only
  loadSi4735parameters();
  DrawSmeter();
  etft.setTTFFont(Arial_14);
  drawSlowScanButtons();  
  settleSynth();
  currentStationIndex = 0;

  lastSwitchTime = millis();
  
  while (1) {

    // Show bands in tinySA
    if (showBands) {
       freq = (bands[currentStationIndex].startFreqKHz + bands[currentStationIndex].stopFreqKHz) * 500; // calculate center freq in Hz 
       setRadio(freq, 1); 
    }   

    else  // Show selected stations
         setRadio(stations[currentStationIndex].freq, stations[currentStationIndex].modT); // show stations

        
    if (!slowSan)  // in case "Leave" was pressed
      break;

    unsigned long currentTime = millis();  

    if (!readSlowScanButtons()){ // function returns 0 when Exit pressed
      return; // Exit
    }  
    // Switch to the next station after duration. switch immediately if EXIT or NEXT are found in Bandinfo
    if ( (currentTime - lastSwitchTime >= (stationDuration * 1000) && !stopScan)) { // 2 seconds is a good value
      lastSwitchTime = currentTime;  // Reset the timer
      currentStationIndex++;         // Move to the next station
      audioPeakVal = 0;
    }
    
   audioScan();

    if (nextStation == true) {
      currentStationIndex++;
      nextStation = false;
      stopScan = true;
    }

    if (lastStation == true && currentStationIndex) {
      currentStationIndex--;
      lastStation = false;
      stopScan = true;
    }


    if (currentStationIndex >= numStations) {
      currentStationIndex = 0;  // Loop back to the first station
    }

    if ((stopState && SNR) || (stopState && audioPeakDetected)) {
      stopScan = true;
    }

    statusIndicator();

   drawCircles();

  }
}

//##########################################################################################################################//

// Function to set radio to a specific frequency and modulation
void setRadio(long freq, char modT) {
  char buffer[50];
  static int ctr = 0;
  ctr++;

  if (ctr == 15) {
    ctr = 0;
    Smeter();
  }

  if (FREQ_OLD == freq)
    return;

  tft.fillRect(170, 180,167, 55, TFT_BLACK);  // overwrite station and audio message
  etft.fillRect(5, 100, 330, 20, TFT_BLACK);  // overwrite last station name
  etft.setCursor(10, 100);
  etft.setTextColor(TFT_SKYBLUE);
  etft.setTTFFont(Arial_14);
  etft.printf("%d: ", currentStationIndex + 1);
  
  if (! showBands)
     etft.print(stations[currentStationIndex].desc);
  else {

     etft.print(bands[currentStationIndex].bandName);
     
     if (bands[currentStationIndex].isAmateurRadioBand)
      etft.print(" Amateur band");
     else
      etft.print(" Broadcast band");   
  } 

 etft.setTTFFont(Arial_14);
  etft.setTextColor(TFT_GREEN);

  FREQ = freq;
  modType = modT;
  FREQCheck();        //check whether within FREQ range
  displayFREQ(FREQ);  // display new FREQ
  tuneSI5351();       // and tune it in
  FREQ_OLD = FREQ;
  SNR = 0;

#ifdef TINYSA_PRESENT
  sprintf(buffer, "sweep center %ld", FREQ);  // sync TSA center frequency with receiver frequency
  Serial.println(buffer);
#endif
}


//##########################################################################################################################//

void drawSlowScanButtons() {


  draw12Buttons(TFT_BTNCTR, TFT_BTNBDR);

  tft.fillRect(3, 235, 336, 57, TFT_BLACK);  // clear lower row
  tft.fillRect(175, 180, 160, 55, TFT_BLACK);

  etft.setTextColor(TFT_GREEN);
  etft.setTTFFont(Arial_14);

  etft.setCursor(21, 140);
  etft.print("Pause");

  etft.setCursor(110, 140);
  etft.print("Last");

  etft.setCursor(195, 140);
  etft.print("Next");

  etft.setCursor(275, 140);
  etft.print("Exit");

  etft.setCursor(20, 190);
  etft.print("Signal");

  etft.setCursor(20, 208);
  etft.print("Stop");


#ifdef TINYSA_PRESENT
  etft.setCursor(110, 190);
  etft.print("Show");

  etft.setCursor(110, 208);
  etft.print("Bands");
#endif

}

//##########################################################################################################################//

bool readSlowScanButtons() {



  pressed = get_Touch();

  displayHoldStatus();

  if (pressed) {
    int buttonID = getButtonID();

switch (buttonID) {
  case 21:
    stopScan = !stopScan;
    if (!stopScan) {
      audioPeakDetected = false;
      SNR = 0;
      delay(200);
    }
    tRel();
    return true;

  case 22:
    lastStation = true;
    audioPeakVal = 0;
    audioPeakDetected = false;
    SNR = 0;
    stopScan = true;
    delay(200);
    return true;

  case 23:
    nextStation = true;
    audioPeakVal = 0;
    audioPeakDetected = false;
    SNR = 0;
    stopScan = true;
    delay(200);
    return true;

  case 24:
    syncEnabled = syncStatus;  // restore TSA sync status
    nextStation = false;
    lastStation = false;
    stopScan = false;
    slowSan = false;
    currentStationIndex = -1;
    etft.setTextColor(TFT_GREEN);
    for (int i = 0; i < 255; i++) // reset to 0 to avoid artefacts when starting mini spectrum 16
      Rpeak[i] = 0;
    tx = ty = pressed = 0;
    slowSan = false;
    return false;

  case 31:
    stopState = !stopState;
    etft.setTextColor(TFT_GREEN);
    etft.setTTFFont(Arial_14);
    etft.fillRect(20, 208, 58, 18, TFT_BLACK);
    etft.setCursor(20, 208);
    if (stopState) etft.print("Contin.");
    else etft.print("Stop");
    tRel();
    return true;

  case 32:
    showBands = !showBands;
    delay(200);
    break;

  default:
    break;
}

    buttonID = 0;
  
  }  // end if pressed

return true;

}

//##########################################################################################################################//

void displayHoldStatus() {
  static bool oldstatus;


  if (oldstatus == stopScan)
    return;

  etft.setTextColor(TFT_GREEN);
  etft.setTTFFont(Arial_14);

  etft.fillRect(18, 138, 62, 20, TFT_BLACK);

  if (stopScan == false) {
    etft.setCursor(21, 140);
    etft.print("Pause");
  }

  else {
    etft.setTextColor(TFT_RED);
    etft.setCursor(18, 140);
    etft.print("Restart");
    etft.setTextColor(TFT_GREEN);
  }

  oldstatus = stopScan;
}


//##########################################################################################################################//

void statusIndicator() {

  static int oldSNRValue;
  if (SNR != oldSNRValue && SNR) {
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(180, 185);
    tft.print("Carrier found");
     tft.setTextColor(TFT_GREEN);
    oldSNRValue = SNR;
  }


  if (audioPeakVal > audioTreshold) {
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(180, 210);
    tft.print("Audio found");
    tft.setTextColor(TFT_GREEN);
    audioPeakDetected = true;
  }

  else audioPeakDetected = false;
}



void drawCircles(){

const int dist = (480/numStations);
const int xPos = 10 + dist * currentStationIndex; 
const int yPos = 306;

static int lastXpos = 0;


if (xPos > 477) return;

tft.fillCircle(lastXpos, yPos, 2, TFT_BLACK);

if ( !audioPeakDetected && !SNR)
  tft.fillCircle(xPos, yPos, 4, TFT_GREY);


if (audioPeakDetected)
  tft.fillCircle(xPos, yPos, 4, TFT_YELLOW);


if (SNR)
  tft.fillCircle(xPos, yPos, 4, TFT_ORANGE);

if ( audioPeakDetected && SNR)
  tft.fillCircle(xPos, yPos, 4, TFT_GREEN);

 tft.fillCircle(xPos, yPos, 2, TFT_RED);

 lastXpos = xPos;

}


//##########################################################################################################################//
