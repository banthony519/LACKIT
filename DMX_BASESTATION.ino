/*
  DMX Generator for ECE 287

  -Brenden Anthony, Stephanie Chang, Elise Polizzi
  2024
*/

#include <ArduinoRS485.h> // the ArduinoDMX library depends on ArduinoRS485
#include <ArduinoDMX.h>   // ArduinoDMX library
#include <TM1637Display.h>       // Seven Segment Display Library

#define DIO 3
#define CLK 4
#define MODE_BUTTON_PIN 12
#define ADDRESS_PIN 10
#define ON_OFF_PIN 11
#define MAX_LIGHTS 16
#define LED_PIN 5

#define LACKET_SIZE 6 // Size of data packet;  // Number of channels to write; # of bytes to be sent in a packet

TM1637Display display(CLK, DIO);

// LACKET Parameters Init
int LACKET[LACKET_SIZE];
int brightness;
int red;
int green;
int blue;
int channel = 1;
int mode = 0;
int address = 0;
int phase = 0;

// MODE Logic Init
int mode_switch_check = 1;
int mode_switcher = 0;
int on = 0;

// ADDRESS Logic Init
int add_switch_check = 1;
int add_switcher = 0;

void setup() {
  Serial.begin(9600);                        // Initialize Serial Comm
  while (!Serial);                           // Wait for Serial to start
  RS485.setPins(1, 5, 4); 
  
  // initialize inputs
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ON_OFF_PIN, INPUT_PULLUP);
  pinMode(ADDRESS_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  display.setBrightness(0x06);
  // initialize the DMX library with the universe size
  if (!DMX.begin(LACKET_SIZE)) {
    Serial.println("Failed to initialize DMX!");
    while (1); // wait for ever
  
  }
}

void loop() {
  mode_switch_check = mode_switcher;
  mode_switcher = digitalRead(MODE_BUTTON_PIN);
  if (mode_switch_check == 0 & mode_switcher == 1){
    mode += 1;
    if(mode == 3){
      mode = 4;
    }
    if (mode >= 6){
      mode = 0;
    }
  }

  add_switch_check = add_switcher;
  add_switcher = digitalRead(ADDRESS_PIN);
  if (add_switch_check == 0 & add_switcher == 1){
    address += 1;
    if (address == MAX_LIGHTS-1){
      address = 99;
    }
    if (address == 100){
      address = 0;
    }

  }
  int displaynum = address*100 + mode;
  display.showNumberDec(displaynum,true);

  on = digitalRead(ON_OFF_PIN);
  
  if(on){
    digitalWrite(LED_PIN,on);
  }

  if(!on){
    digitalWrite(LED_PIN,LOW);
    // send packet
    DMX.beginTransmission(); 
    DMX.write(channel, brightness);
    if (mode != 4){
      DMX.write(channel+1, red);
    }
    else if (mode == 4){
      DMX.write(channel+1, phase);
    }
    DMX.write(channel+2, green);
    DMX.write(channel+3, blue);
    DMX.write(channel+4, mode);
    DMX.write(channel+5, address);
    DMX.endTransmission();
  }
  
  brightness = analogRead(A1);
  brightness = int(brightness * (255.0 / 1023.0));
  red = analogRead(A2);
  red = int(red * (255.0 / 1023.0));
  green = analogRead(A3);
  green = int(green * (255.0 / 1023.0));
  blue = analogRead(A4);
  blue = int(blue * (255.0 / 1023.0));

  if (mode == 4){
    phase += red/20;
  }
  
  delay(30);
}

