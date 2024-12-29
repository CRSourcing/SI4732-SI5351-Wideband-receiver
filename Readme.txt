Here is the description of a wideband receiver project which covers 0.1-500 MHz. It is intended as an alternative to a SDR receiver. It fits in a small box and does not require
a PC or RPi for signal processing.
It uses ESP32, ILI9488 touch display, SI4732, SI5351 and AD831 as main components. A tinySA is used as optional panorama adapter. It uses potentiometers for volume, squelch and fine tune.
The software is a bit rough since I am not a programmer. It does include some nice features though, such as morse decoder, slow scan waterfall, memory bank scanning, web tools, etc.
To build the hardware you must have experience with RF circuits and the appropiate toolset. For building the filters you need a network analyzer or nanoVNA.
There is no schematics available, a lot of the hardware is based on try and error, but if you have experience with RF circuits you will be able to build it.


Brief hardware description:

1.RF enters a  wideband LNA. It uses a MAR-6 + SBA-4089  2 stage LNA which  has about 30 dB of aplification. Important is to select a LNA that goes down to 0 MHz.
There is a similar commercial product available on Ebay and Aliexpress.

2. FM input of SI4732 is connected via 30dB attenuator to the output of the LNA.

3. RF then enters the filter bank. The filters are crucial and the most difficult part of the project. The RF gets filtered by one of 6 filters:
0-30MHz lowpass = Filter 1, 25 - 65 bandpass = 2, 65 - 135 bandpass = 3, 116 - 148 steep bandpass = 4 , 148 - 220 = bandpass 5, above 220 highpass = 6
I used hand wound inductors and tuned them with a NanoVNA.This is not a good solution, there is not enough dampening for far off frequencies and losses are high.
Better would be to use smd inductors and a properly designed PCB. If you can't do that, use a veroboard, place input and output SMA connectors
close together, connect them with a thick ground wire and build the filter for the highest frequency closest to the connectors and the lowest frequencies furthest from the connectors.

Frequency ranges are described in struct FilterInfo and can be adjusted as needed. I had to adjust several, since the real life filter parameters
did not exactly meet the designed frequencies. The software activates the filter in use.
Filters are Chebyshev 5th order filters.
Filters were designed with an online filter tool and a ripple of 1db.  The selected filter gets connected to the output of the LNA and the input of the mixer
through two pin diodes (BAR64).
The pin diodes are placed as close to the SMA connectors as possible and connect to the repective filters via RG174 cables.
Current for each pin diode is about 5ma. For low frequencies you could also use 1N4148.
The 0-30 MHz low pass has two additional inductors from input and output (100uH) to ground to provide a DC path for the pin diodes. All other filters are ground shunted.
A 74LS138 3:8 decoder is connected to 3 GPIO's of the ESP32 and its outputs drive the bases of 6 PNP transistor through 680 Ohms resistors.
Their emitters are connected to +5V and the collectors drive the respective pair of pin diodes through 1K resistors.


4. The filtered RF then gets mixed down in an AD831 to the IF of 21.4MHz. I used the AD831 schematics from the application note with a 9V supply.
This is the only component that requires +9V supply. I also tried an ADE-1 diode ring mixer, but it required a much higher oscillator level, producing more spurs and interferences.
The AD 831 LO input is connected to two -6dB attenuators. One of them is connected directly to CLK2 of the SI5351 and is used for 0-200 MHz reception. The other one is connected
to CLK0 through a Chebyshev 7th order high pass. It supresses the fundamental and lets pass the 3rd harmonics which is used for 200-500MHz reception. This should better be a band pass,since it lets also pass the 5th harmonics
which still produces mirrors of digital television. If CLK0 is selected in software, the VFO gets programmed with 1/3 of its frequency.

CLK2 uses mainly high injection (LO above RF), but this can be configured in struct FilterInfo. If high injection produces too many mirrors, try low injection and viceversa.
This can be done in segments, so you can for example have too many mirrors in the 2m amateur band, define start and end frequencies in the structure and change injection mode.
CLK0 MHz uses mainly low injection (LO below RF). 500MHz RF is not the limit, I have tried up to 650MHz, but sensitivity drops rapidly.

5. The 3rd clock from the SI5351 (CLK1) can be selected in software and used as clock source for a transmitter.

6. The IF side of the AD831 is terminated with 50 Ohms and enters a crystal filter which is made of 5 cheap 21.4 MHz 2 pole crystal filters (Aliexpress) in series and ground capacitors
in between. The bandwith is +- 5 KHz, with around 70dB dampening at +- 10 KHz. There is a depression of around 2dB at the center frequency.
This filter unexpectedly allows excellent FM narrowband flank demodulation, so I did not build a seperate NBFM demodulator like originally planned. The ground capacitors were hand picked with a
nanoVNA. The filters flanks should not be too steep, otherwise FM demodulation will get distorted. It would even be better to use 2 separate filters, a narrow one for SSB and a wider one for AM/NBFM.
My filter's center frequency is off around 3KHz (21.397MHz) for some reason, but this can be compensated in software. The filter is critical for not overloading the SI4732.
The IF can be configured in the software, anything up to 30MHz is possible, so you can also use filters for different frequencies. The higher the IF, the better.

7. A tinySA is used as optional panorama adapter. Software connects it through micro relay to either the IF output of AD831 (through a 20dB attenuator), or to the output of the LNA via 6 dB attenuator.
   Please be aware that the tinySA has a quite limited resolution.

8. The crystal filter output connects to a SI4732, terminated with 50 Ohms. There is quite a loss (10 - 15 db estimated) through the crystal filter, since it does not use impendance transformers,
but the active mixer provides enough amplification to overcome it.
The SI4732 is controlled by an ESP32 development board. SI4732 is in the standard configuration with a 32.768 KHz crystal,
but it's frequency stability is not great.The software for FM radio reception is quite basic.
The audio outputs are connected together and go to a squelch transistor which is driven by the one of the ESP32 GPIO's. It grounds the audio signal when triggered and  eliminates noise and the hard cracks when switching mode.
From the squelch transistor the signal goes to the volume potentiomenter and then to a LM386 audioamplifier which drives headphones and speaker. DAC_CHANNEL_2  is connected via 330KOhms
to the input of the audio amplifier and provides a short touch sound.

9. The ESP32 drives the SI4732 and SI5351 on the same I2C bus. A squelch potentiometer and a fine tune potentiometer are connected to the ESP32.
The fine tune potentiometer is also used to adjust color spectrum in waterfall mode. The fine tune pot needs to be of good quality, since any noise causes frequency jumps.
It is important to use a 38pin version of the ESP32 board since almost all of the GPIO's will be used.
The ESP32 also drives the 3.5" ILI9488 touch display in a standard configuration for the tft eSPI library.
This code will only work with ILI9488 480*320 pixel displays and is not adaptable for smaller displays.

10. 12V DC input input gets regulated through two linear regulators down to +5 and +9V. Power consumption is about 400ma.

11. For the FFT analysis and the morse decoder, audio output goes via 4.7K and 1 microfarad to the base of a npn transistor amplifier. Emitter via 100 Ohms to ground, base via 47K to collector and collector via 1K to +3.3V.
Collector then goes to pin 36 of the ESP which does the sampling. Gain is about 20 dB. Both applications require the volume of the SI4732 to be set to a fixed level, in this case 50,
setVolume(50).

12. If you use a tinySA, connect GPIO 1 of the ESP32 with the RX pin of the tinySA and GPIO 3 with the TX pin (close to the battery connector).
If you do this, please note that the tinySA needs to be switched off when loading firmware into the ESP32.
This will allow control of tinySA parameters from the receiver menu. Connect the tinySA audio output via 10K and 0.1uF in series to the audio output of the SI4732.
You can then listen to the tinySA. Check the Youtube video to see how it works.
Configure the TSA to use serial connection 115200 so that it accepts the commands from the ESP32. In "Preset" menu enable "Save Settings".
tinySA firmware version >= v1.4-105 2023-07-21 needed.


13. I have not designed a PCB, nor drawn full schematics. The ESP board is directly attached to the backside of the display. The HF part is build upon the copper sides of two seperate
(unetched) PCB's with the SI4732, AD831, crystal filter and audio amp on one and the LNA and the filters on the other one.
SI5351 is placed in a metal box and connected through coax cables with the mixer.There is a shielding plate between the display and the RF boards.



Software hints:

There is little error checking implemented, so if you press combinations that make no sense you will get a result that makes no sense.

Options to select a frequency: 1.Set Frequency manually via touchpad, 2. Load Memo, 3. Select a Band, or 4. Use Up- Down keys.
"Select Band": Loads a predefined band from struct BandInfo. Configurable, tuning either without limits (can leave the band) or loop mode (stays within band).
Scan: Seek up, Seek down, or set a range. Scan is squelch driven. If band loop mode is enabled, Seek will stay within the band.

TinySA: Touch indicator area (RF/IF) to select mode. In IF mode the IF spectrum will be displayed. Caveat: If Hi injection is enabled (default), the spectrum is mirrored.
In RF mode the tinySA is connected with the RF input. This mode is slower since the tinySA gets updated via serial commands. It has 2 submodes:

In track mode it syncs with the receiver and the tuned frequency is displayed in the center of the screen. This is slow.
In window mode, outside of a preselected band, a 1MHz window get displayed and marker 1 shows the tuned frequency. Tuning out of the window switches to the next window.
In window mode using a preselected band, the entire band will be displayed and and marker 1 shows the tuned frequency.
Change of track/window mode requires reloading the frequency.
In Config tinySA you can set parameters and also listen to the tinyAS

SNRSquelch: Uses SNR to trigger audio mute and the squelch pot will not be used.This only works in AM mode.

Waterfall (slow scan): Touch on a spot on the lower horizontal signal bar (while it says "SCANNING") and enter listen mode.
Touch on the waterfall to go back to scan mode, or touch SET to set the frequency and leave.

Touch Tune: Shows 1MHz around tuned frequency, use encoder to change frequency. Touch a spot on the signal area and it tunes in and you can listen to frequency.
Use encoder to fine tune. Touch "Cont." to leave frequency and continue.Touch back to go back to main menu without setting the frequency. Touch "Set" to go back to main menu and set frequency.

2D/3D Waterfall displays a trapezoid that shows frequency time relationship and gives the illusion of a 3D waterfall.

After pressing the encoder,step size will be changed to 1MHz. This helps to rapidly view 1MHz segments on the tinySA. Press encoder again to return to previous step size.



V.0.321

Hardware: Changed LNA to a 2 stage LNA and connected SI4732 WBFM input to LNA output via attenuator.
Added 6x 2.2K resistors from the collecors of the pin diode driver transistors to ground, to eliminate stray voltages.
Added TX and RX connection to tinySA for serial commands.
Added Audio connection to the tinySA, to listen to tinySA audio output.
Changed switch for tinySA input from pin diodes to relay.
Software changes:
Sketch now needs to much space for default partition scheme. Use 2MB APP, 2MB SPIFFS!!!
Fixed a bug that made SCAN UP and FREQUENCY UP buttons work unreliably.
Fixed bug in WBFM initialization.
Fixed bug that prevented SI4735 from using sideband inversion when switching injection mode.
Changed BFO code to use independent BFO's for LSB/USB  in Low/Hi injection.
Fixed bug that would hide the mini window  permanently (miniWindowMode > 5 was allowed).
Fixed a bug that would hide the clock display when returning from some menues.
Moved mainScreen(); for better touch responsiveness.
Moved clock display (in lower right corner) to separate function.
Moved  colorselector for frequency display to separate function.
Rewrote setSquelch() to avoid a potential bug when switching squelch mode.
Changed RSSI indicator from dBuV to dBm and calibrated at 10 microvolts.
Added NBFM button and NBFM steps. NBFM = AM but sets SI4732 at a frequency offset for better slope decoding.
Finetune now uses a fixed +-2 KHz in all modes and is diabled in NBFM.
Tuning through memory bank with encoder now loads modulation type.
WBFM now also tunes the SI5351, so that the tinySA displays the WBFM spectrum.
Modified displayFREQ() to remove redundancies.
Added menu for loading tinySA configurations. Added menu to listen to the audio output of the tinySA.
Added tracking when tinySA is in RF mode. TSA center frequency can now be synchronized with receiver frequency.
TSA mode gets now saved and loads either preset 0 or preset 1 of the TSA.
Added code for alternative waterfall colors.
Added code to select whether freq stays within a selected band (auto loop).
Added code to sync tinySA with waterfall frequencies (Band view).
Added code to display saved settings when booting.
Added code to select whether tinySA centers receiver frequency or displays a fixed segment and marker 1 shows receiver frequency within the segment
Added code to tap and listen when in waterfall mode.
Added code where you touch the screen to listen to a station (Touch tune) to main menu.
Added an envelope display to the mini window and made it's waterfall display run horizontally.
Added code to display a 1 MHz segment as 2D/3D waterfall (trapezoid).
Added code to import markers from tinySA and added two buttons to tune receiver to them.
Added code to import marker dBm's from tinySA. TinySA signal strength in microvolts gets now displyed in upper right corner.
Added code to change FREQ in 1 MHz steps after encoder pressed.
Added code to configure tinySA via quickbutton ("Cfg" button in lower right corner).
Added code to quick sync the tinySA (Orange "S" button)
Added code for a 240 bin audio waterfall that gets displayed during user inactivity.
Changed I2C bus speed to 2Mhz. This seems to work without issues.
Added code for a slow scan of SW radio stations. Define stations in struct SlowScanFrequencies.
