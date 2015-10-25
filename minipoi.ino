/*
 * Led poi powered by FastLED and RF24
 *
 * Using an arduino nano and nrf2401l
 *
 * Master also hse a HC-06 bluetooth module
 */

#include <SPI.h>
#include "RF24.h"
#include <FastLED.h>

#define NUM_LEDS 66
#define ZEBRA 10
#define MASTER true 
#define DATA_PIN 4
#define CLOCK_PIN 5

uint8_t mainHue = 0;
uint8_t secondHue = 0;

CRGB mainColour = CRGB(255, 0, 0);
CRGB secondColour = CRGB(0, 255, 0);

int masterTime = 0;

int index = 0;

uint8_t brightness = 40;

int ledMode = 1;

int frame = 0;

int hueJump = 1;

boolean timeout = false;

CRGB leds[NUM_LEDS];

struct dataStruct {
  CRGB colour;
  uint8_t value;
  uint8_t mode;
} myData;


/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9, 10);
byte addresses[][6] = {"1Node", "2Node"};

void setup() {

  Serial.begin(9600);
  //Setup RF24
  radio.begin();

  //radio.enableAckPayload();                     // Allow optional ack payloads
  //radio.enableDynamicPayloads();                // Ack payloads are dynamic payloads
  //radio.setPALevel(RF24_PA_LOW);

  if (MASTER) {
    radio.openWritingPipe(addresses[1]);        // Both radios listen on the same pipes by default, but opposite addresses
    radio.openReadingPipe(1, addresses[0]);     // Open a reading pipe on address 0, pipe 1
  } else {
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1, addresses[1]);
  }
  radio.startListening();

  //Setup FastLED
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(12)>(leds, NUM_LEDS);
  // set master brightness control
  FastLED.setBrightness(brightness);

  delay(5000);
  Serial.println("Board started");
}

void loop() {
  // put your main code here, to run repeatedly:

  if (MASTER) {
    if (Serial.available()) {
    bt();
    }
  } else {
    if ( radio.available()) {
      rf();
    }
  }

  switch (ledMode) {
    case 0:
      //Do nothing
      break;
    case 1:
      showMainColour();
      break;
    case 2:
      zebraPoi();
      break;
    case 3:
      mainColourStrobe();
      break;
    case 4:
      rainbow();
      break;
    case 5:
      rainbowStrobe();
      break;
    case 6:
      confetti();
      break;
    case 7:
      fullSpectrumConfetti();
      break;
    case 8:
      mainColourPulse();
      break;
    case 9:
      rgbRibbon();
      break;
    case 10:
      rgbStrobe();
      break;
    case 11:
      sixColorRibbon();
      break;
    case 12:
      sixColorStrobe();
      break;
    case 13:
      zebraRainbow();
      break;
    case 14:
      rainbowFill();
      break;
    case 15:
      spiral();
      break;
    default:
      //do nothing
      break;
  }


}


/*
 * Bluetooth function
 */
void bt() {
  char val = Serial.read();  //sync or not 
  
  int mode = Serial.parseInt();
  myData.mode = mode;
  switch (mode) {
    case 0:
      ledMode = Serial.parseInt();
      //if (val == '£' || val == '$') {
        myData.value = ledMode;
     // }
      break;
    case 1:
      mainColour = readColour();
      
      //if (val == '£' || val == '$') {
        myData.colour = mainColour;
      //}
      break;
    case 2:
      secondColour = readColour();
      
      //if (val == '£' || val == '$') {
        myData.colour = secondColour;
     //}
      break;
    case 3:
      masterTime = Serial.parseInt();

      myData.value = masterTime;
      break;
    case 4:
      brightness = Serial.parseInt();
      myData.value = brightness;
      FastLED.setBrightness(brightness);
      break;
      case 5:
    hueJump = Serial.parseInt();
    myData.value = hueJump;
    break;
    case 6:
      reset();
      break;
    default:
      //nothing
      break;

  }


 // if (val == '£' || val == '$') {
    //timeout = false;                                   // Set up a variable to indicate if a response was received or not
    sendRF();
    //waitForTimeout();
  //}

  Serial.flush();
}

void sendRF() {
  radio.stopListening();
  if (!radio.write( &myData, sizeof(myData) )) {
    sendRF();
  }
  radio.startListening();
}

void waitForTimeout() {
unsigned long started_waiting_at = micros();
  while (! radio.available() ) {                            // While nothing is received
    if (micros() - started_waiting_at > 200000 ) {        // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }
  }

  if ( timeout ) {                                            // Describe the results
    sendRF();
    waitForTimeout();
    Serial.write("Timeout");
  }
}

/*
 * Radio Frequenct function
 */
void rf() {
  while (radio.available()) {                          // While there is data ready
    radio.read( &myData, sizeof(myData) );             // Get the payload
  }
  radio.stopListening();                               // First, stop listening so we can talk
  radio.write( &myData, sizeof(myData));              // Send the final one back.
  radio.startListening();                              // Now, resume listening so we catch the next packets.
  switch (myData.mode) {
    case 0:
      ledMode = myData.value;
      break;
    case 1:
      mainColour = myData.colour;
      break;
    case 2:
      secondColour = myData.colour;
      break;
    case 3:
      masterTime = myData.value;
      break;
    case 4:
      brightness = myData.value;
      FastLED.setBrightness(brightness);
      break;
    case 5:
    hueJump = myData.value;
    break;
    case 6:
      reset();
      break;
    default:
      //nothing
      break;

  }
}

CHSV readColour(){
//First int is Hue
    int h = Serial.parseInt(); 

    //Second is saturation
    int s = Serial.parseInt();

    //Third is vibrancy
    int v = Serial.parseInt();

    return CHSV(h,s,v);
  
}

void showMainColour() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = mainColour;
  }
  FastLED.show();
}

void rainbow() {
  mainHue+=hueJump;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(mainHue, 255, 255);
  }
  FastLED.show();
  delay(masterTime);
}



void zebraPoi() {
  int i = 0;
  for (i = i; i < ZEBRA; i++) {
    leds[i] = secondColour;
  }
  for (i = i; i < NUM_LEDS - ZEBRA; i++) {
    leds[i] = mainColour;
  }

  for (i = i; i < NUM_LEDS; i++) {
    leds[i] = secondColour;
  }
  FastLED.show();
}

void  mainColourStrobe() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = mainColour;
  }
  FastLED.show();
  delay(masterTime);
  off();
}
void rainbowStrobe() {
  rainbow();
  off();
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( mainHue + random8(64), 200, 255);
  FastLED.show();
}

void fullSpectrumConfetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] = CHSV( random(255), 255, 255);
  FastLED.show();
}

void mainColourPulse() {
  mainColourStrobe();
}

void rgbRibbon() {
  frame++;
  if (frame >= 3)frame = 0;

  switch (frame) {
    case 0:
      mainColour = CRGB(255, 0, 0);
      break;
    case 1:
      mainColour = CRGB(0, 255, 0);
      break;
    case 2:
      mainColour = CRGB(0, 0, 255);
      break;
  }
  showMainColour();
  delay(masterTime);


}

void rgbStrobe() {
  frame++;
  if (frame >= 3)frame = 0;

  switch (frame) {
    case 0:
      mainColour = CRGB(255, 0, 0);
      break;
    case 1:
      mainColour = CRGB(0, 255, 0);
      break;
    case 2:
      mainColour = CRGB(0, 0, 255);
      break;
  }
  showMainColour();
  delay(masterTime);
  off();

}
void sixColorRibbon() {
  frame++;
  if (frame > 5)frame = 0;

  switch (frame) {
    case 0:
      mainColour = CRGB(255, 0, 0);
      break;
    case 1:
      mainColour = CRGB(255, 255, 0);
      break;
    case 2:
      mainColour = CRGB(0, 255, 0);
      break;
    case 3:
      mainColour = CRGB(0, 255, 255);
      break;
    case 4:
      mainColour = CRGB(0, 0, 255);
      break;
    case 5:
      mainColour = CRGB(255, 0, 255);
      break;
  }
  showMainColour();
  delay(masterTime);

}
void sixColorStrobe() {
  frame++;
  if (frame > 5) {
    frame = 0;
  }

  switch (frame) {
    case 0:
      mainColour = CRGB(255, 0, 0);
      break;
    case 1:
      mainColour = CRGB(255, 255, 0);
      break;
    case 2:
      mainColour = CRGB(0, 255, 0);
      break;
    case 3:
      mainColour = CRGB(0, 255, 255);
      break;
    case 4:
      mainColour = CRGB(0, 0, 255);
      break;
    case 5:
      mainColour = CRGB(255, 0, 255);
      break;
  }
  showMainColour();
  delay(masterTime);
  off();

}

void zebraRainbow() {
mainHue+= hueJump;
secondHue+= hueJump;
  int i = 0;
  for (i = i; i < ZEBRA; i++) {
    leds[i] = CHSV(secondHue, 255, 255);
  }
  for (i = i; i < NUM_LEDS - ZEBRA; i++) {
    leds[i] = CHSV(mainHue, 255, 255);
  }

  for (i = i; i < NUM_LEDS; i++) {
    leds[i] = CHSV(secondHue, 255, 255);
  }
  FastLED.show();
  delay(masterTime);
}

void rainbowFill() {

  fill_rainbow(leds, NUM_LEDS, 0, hueJump);
  FastLED.show();
}
void spiral() {
  leds[index] = CRGB(0, 0, 0);
  index++;
  if (index > NUM_LEDS) {
    index = 0;
  }
  leds[index] = mainColour;
  FastLED.show();
}

void off() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  delay(masterTime);

}

void reset() {
  mainHue = 0;
  secondHue = 127;
  mainColour = CRGB(255, 0, 0);
  secondColour = CRGB(0, 255, 0);
  masterTime = 0;
  index = 0;
  brightness = 40;
  ledMode = 1;
  frame = 0;
  hueJump = 1;

}


