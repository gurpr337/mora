#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Wire.h>

#define LED_PIN 6
#define BRIGHTNESS 90

Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, LED_PIN, NEO_GRB + NEO_KHZ800);

//MIDI defines
#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define MAX_MIDI_VELOCITY 127
#define CONTROL_CHANGE_CMD 0xB0

//MIDI baud rate
#define SERIAL_RATE 115200

int Ainputs[] = {A0,A1,A2,A3};
int Acontrols[4];

float convert = .1241; // for converting from x/1023 (analogRead) to x/127 (MIDI)


void setup()
{
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);

  
  Serial.begin(SERIAL_RATE);
  
  strip.setBrightness(BRIGHTNESS); //initialize NeoPizel LED strip
  strip.begin();
  strip.show();
  
  Wire.begin();                // join i2c bus 
}

void loop()
{
  
  uint16_t i, j;
  for(j=0; j<256; j++) 
  {
    
    for(i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    
    strip.show();

    Wire.requestFrom(8, 3);    // request 3 bytes from slave device #8
    while (Wire.available())
    {                       // loop through the bytes sent by the slave
      byte a = Wire.read(); // receive byte as a byte
      Serial.write(a);      // write the byte to be read by hairless midi
    }

   
    for(int i=0;i<(sizeof(Acontrols)/2);i++) // then check if the knobs controlled here have changed
    {
      if(AcontrolUpdate(i,Acontrols[i]))
      {
        break;
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


void rainbow(uint8_t wait) 
{
  uint16_t i, j;
  for(j=0; j<256; j++) 
  {
    for(i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}




void midiControl(byte c, byte val) // send a midi control change
{
  Serial.write(CONTROL_CHANGE_CMD); // write the byte for control change
  Serial.write(c); // write the channel int (as a byte)
  Serial.write(val); // write the value ( as a byte)
}

int AcontrolUpdate(int dex, int k1)
{ // function to check and update analog pins
int k1n=analogRead(Ainputs[dex]); // read in the current state of the analog pin at k1 - (k1 new)   
  
  float hold = k1n * convert;
  k1n = (int) hold; // convert the k1 value to out of 127 for midi and other purposes

 if(abs(k1-k1n)>10) // if the new value is different than the one we have...
 {
   k1=k1n; // assign the new value
   Acontrols[dex]=k1;
   midiControl(dex,k1); // send the midi message on the channel associated with the index 
   return 1;
 }
 else
 {
   return 0;
 }
}

