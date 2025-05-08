/*
  DMX Receiver for ECE 287 Design Lab

  -Brenden Anthony, Stephanie Chang, Elise Polizzi
  2024-2025
*/

#include <ArduinoRS485.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NEOPIXEL_PIN 8 // Communication Pin to send data to NeoMatrix
#define NUMPIXELS 256 // Size of NeoMatrix
#define LACKET_SIZE 6 // Size of data packet (Brightness, Red, Green, Blue, Mode, Address)

#define ADDRESS 4 // Different for every receiver

int fadeValue = 20;
int fade_step = 3; // rate for fade-mode
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Begin Serial Communication @ 250,000 baud
  Serial.begin(250000);
  while (!Serial); // While Serial is closed, wait
  
  // Initialize DMX Communication
  RS485.begin(250000); // Begin RS485 communication @ 250,000 baud
  RS485.receive(); // enable reception, can be disabled with: RS485.noReceive();

  // Initialize Neopixel Matrix
  pixels.begin();
  pixels.clear();
}

// Function to print an array as a list
void printArray(int arr[], int size) {
  Serial.print("["); // Starting bracket
  for (int i = 0; i < size; i++) {
    Serial.print(arr[i]);
    if (i < size - 1) {
      Serial.print(", "); // Print comma and space for all but the last element
    }
  }
  Serial.println("]"); // Ending bracket and newline
}

void Flip_Flop(int lacket[], int matrix_size) {
  int left_colour = lacket[1]; // R Channel becomes Left_Hue
  int center_location = lacket[2]/8; // G Channel becomes location of center line (Ambitious?)
  int right_colour = lacket[3]; // B Channel becomes Right_Hue

  // Hue Assignment
  uint32_t left = pixels.ColorHSV(left_colour*left_colour);
  uint32_t right = pixels.ColorHSV(right_colour*right_colour);
  
  // Matrix Updater
  pixels.setBrightness(lacket[0]);
  pixels.fill(left, 0, center_location*8);
  pixels.fill(right, center_location*8+8, matrix_size);
  //pixels.show();
}

void loop() {
  //RS485.receive();
  // Create packet and read incoming data
  int packet[LACKET_SIZE + 2] = {};
  for(int i=0; i<LACKET_SIZE + 2; i++){
    while(!RS485.available());
    packet[i] = (RS485.read());
  }
  // Trim packet and store usable data
  int lacket[LACKET_SIZE]={}; // lacket = {Brightness, R, G, B, Mode}
  for(int l=0; l<LACKET_SIZE; l++){
    lacket[l] = packet[l+2];
  }
  printArray(lacket,LACKET_SIZE);

  if(lacket[5] == ADDRESS or lacket[5] == 99){  
    // Brightness Cutoff
    if(lacket[0] < 10){
      pixels.clear();
      pixels.show();
      return;
    }

  // MODES AND DEMOS

    uint32_t colour_profile;

    if (lacket[4] == 0){ // RGB MODE
      colour_profile = pixels.Color(lacket[1], lacket[2], lacket[3]);
    }

    if (lacket[4] == 1){ // REACTIVE WHITE MODE
      // Responsive White Mode Using Dial 1
      float tmpKelvin = lacket[1]*(3000/255);

      if (tmpKelvin < 1000){
        tmpKelvin = 1000;
      }
      if (tmpKelvin > 40000){
        tmpKelvin = 40000;
      }

      tmpKelvin = tmpKelvin/100;
      float tmpCalc;
      int r;
      int g;
      int b;

      // Red Channel Calc
      if (tmpKelvin <= 66){
        r = 255;
      }
      else{
        tmpCalc = tmpKelvin - 60;
        tmpCalc = 329.698727446 * pow(tmpCalc,-0.1332047592);
        r = tmpCalc;
        if (r < 0){
          r = 0;
        }
        else if (r > 255){
          r = 255;
        }
      }

      // Green Channel Calc
      if (tmpKelvin <= 66){
        tmpCalc = tmpKelvin;
        tmpCalc = 99.4708025861 * log(tmpCalc) - 161.1195681661;
        g = tmpCalc;
      }
      else{
        tmpCalc = tmpKelvin - 60;
        tmpCalc = 288.1221695283 * pow(tmpCalc,-0.0755148492);
        g = tmpCalc;
      }

      if (g < 0){
        g = 0;
      }
      else if (g > 255){
        g = 255;
      }

      // Blue Channel Calc
      if (tmpKelvin >= 66){
        b = 255;
      }
      else if (tmpKelvin <= 15){
        b = 0;
      }
      else {
        tmpCalc = tmpKelvin - 10;
        tmpCalc = 138.5177312231 * log(tmpCalc) - 305.0447927307;
        b = tmpCalc;

        if (b < 0){
          b = 0;
        }
        else if (b > 255){
          b = 255;
        }
      }

      lacket[1] = r;
      lacket[2] = g;
      lacket[3] = b;
      colour_profile = pixels.Color(lacket[1], lacket[2], lacket[3]);
    
    }  

    if (lacket[4] == 2){ // HSV MODE (currently HV mode)
      colour_profile = pixels.ColorHSV(lacket[1]*lacket[1]);
    }

    if (lacket[4] == 3){ // FADE IN/OUT MODE

      if (fadeValue >= 200){
        fade_step = -fade_step;
      }
      if (fadeValue <= 15){
        fade_step = -fade_step;
      }
      fadeValue += fade_step;
      
      lacket[0] = fadeValue;
      colour_profile = pixels.Color(lacket[1], lacket[2], lacket[3]);
      //delay(30);
    }

    if (lacket[4] == 4){ // RAINBOW MODE
      pixels.rainbow(lacket[1]*lacket[1], lacket[3]/10 + 1, 255, lacket[0]/2 + 100, true);
      pixels.show();
    //pixels.rainbow(starting hue, repetitions of rainbow, saturation, brightness, gamma-colour-correction)
      return;
    }

    if (lacket[4] == 5){
      Flip_Flop(lacket, NUMPIXELS);
      
      pixels.show();
      return;
    }
      

    pixels.setBrightness(lacket[0]);
    pixels.fill(colour_profile, 0, NUMPIXELS);
    pixels.show();
  }
}

