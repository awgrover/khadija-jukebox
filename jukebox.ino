/* Install
  install the "Adafruit VS1053 Library by adafruit"

  Assume wiring as per 
  https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
  Which uses pins: 3,4,8,9,10,12,13

  Pins A0..Annn are the trigger pins. low to trigger. See Buttons list below.
  Sound is: 000.mp3, where 000 is replaced with the pin number, e.g. A0 = 000.mp3, A10 = 010.mp3

  Code based on adafruit example
  License: GNU GENERAL PUBLIC LICENSE Version 3
*/

#define MP 1 // 0 for disable musicplayer
#define IFMP if (MP) // enable/disable musicplayer for debugging

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins. 
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

// create breakout-example object!
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
 

const int Buttons[] = { A0, A1, A2, A3, A4, A5, -1 }; // PINS: on LOW, play 00n.ogg. END LIST WITH -1. Assume consecutive
char track[] = "000.mp3";  // hardcoded assumption below

void setup() {
  Serial.begin(9600);
  Serial.println("Setup");

  Serial.print("A0 ");Serial.println(A0);
  for (int *pin=Buttons; *pin != -1; pin++ ) {
    pinMode( *pin, INPUT_PULLUP );
    Serial.print("Using ");Serial.print(*pin);Serial.print(" as ");Serial.println(*pin - A0);
  }
  Serial.print("Pins setup ");Serial.println((int)Buttons);

  if (MP && ! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
  if (MP && !SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println(F("SD Setup"));

  // list files
  IFMP printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  IFMP musicPlayer.setVolume(0,0);

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  IFMP musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  Serial.println("Ready");
 }

void loop() {
  
  int current_button = which_button( Buttons );
  if (current_button != -1) {
    IFMP musicPlayer.stopPlaying();

    current_button = current_button - A0; // Use A0 as 0, A1 as 1,...

    // Fixup file name
    // first digit is 0
    // second digit is tens
    if (current_button / 10) track[1] = '0' + (current_button / 10); 
    // third digit is ones
    track[2] = '0' + (current_button % 10);
    Serial.print("Play ");Serial.println(track);
    IFMP musicPlayer.startPlayingFile("track002.mp3");
    }
  }

int which_button( int *pinlist ) {
  // returns -1 if no change
  // debounces
  static int last_button = -1;
  static unsigned long last_detect = 0;

  // debounce
  if (millis() - last_detect < 1050) {
    return -1;
    }

  // Serial.print("Pins at ");Serial.println((int)pinlist);

  for (int *pin = pinlist; *pin != -1; pin++ ) {
    // Serial.print(" ");Serial.print(*pin);
    if (! digitalRead( *pin ) ) {
      // Serial.print("Down ");Serial.println(*pin);
      // only counts if diff
      if ( last_button != *pin ) {
        Serial.print("Changed from ");Serial.print(last_button);Serial.print(" to ");Serial.println(*pin);
        last_button = *pin;
        last_detect =  millis();
        return *pin;
        }
    }
  }

  return -1; // nothing changed
}

void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
