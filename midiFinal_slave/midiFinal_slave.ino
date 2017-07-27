#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Wire.h>


#define LED_PIN 6
#define BRIGHTNESS 50

Adafruit_NeoPixel strip = Adafruit_NeoPixel(150, LED_PIN, NEO_GRB + NEO_KHZ800);


//MIDI defines
#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define MAX_MIDI_VELOCITY 127
#define CONTROL_CHANGE_CMD 0xB0

//MIDI baud rate
#define SERIAL_RATE 115200

int Acontrols[8];
int Dcontrols[7];
int notes[8];
int noteStatus[8];

int Ainputs[] = {A0,A1,A2,A3,A12,A13,A14,A15};
int Dinputs[] = {22,23,24,25,26,27,28};
int Ninputs[] = {A4,A5,A6,A7,A8,A9,A10,A11};

int NoteNames[] = {48,50,52,53,55,57,59,60};

float convert = .1241; // for converting from x/1023 (analogRead) to x/127 (MIDI)

void setup()
{
  strip.setBrightness(BRIGHTNESS);
  strip.begin();
  strip.show();
  
  Serial.begin(SERIAL_RATE);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  pinMode(A4,INPUT);
  pinMode(A5,INPUT);
  pinMode(A6,INPUT);
  pinMode(A7,INPUT);
  pinMode(A8,INPUT);
  pinMode(A9,INPUT);
  pinMode(A10,INPUT);
  pinMode(A11,INPUT);
  pinMode(A12,INPUT);
  pinMode(A13,INPUT);
  pinMode(A14,INPUT);
  pinMode(A15,INPUT);

  pinMode(22,INPUT_PULLUP);
  digitalWrite(22,HIGH);
  pinMode(23,INPUT_PULLUP);
  digitalWrite(23,HIGH);
  pinMode(24,INPUT_PULLUP);
  digitalWrite(24,HIGH);
  pinMode(25,INPUT_PULLUP);
  digitalWrite(25,HIGH);
  pinMode(26,INPUT_PULLUP);
  digitalWrite(26,HIGH);
  pinMode(27,INPUT_PULLUP);
  digitalWrite(27,HIGH);
  pinMode(28,INPUT_PULLUP);
  digitalWrite(28,HIGH);
  
  

  strip.setBrightness(BRIGHTNESS);
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
}






int bright = 50;

void loop()
{
 uint16_t i, j;
  for(j=0; j<256; j++) 
  {
      strip.setBrightness(bright);

    for(i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(20);
  }
}

void requestEvent()
{
  bool flag=1;

  if(flag)
  {
    for(int i=0;i<(sizeof(Dcontrols)/2);i++)
    {
     if(DcontrolUpdate(i,Dcontrols[i]))
      {
        break;
        flag=0;
     }
    }
  }

  for(int i=0;i<(sizeof(Acontrols)/2);i++)
  {
    if(AcontrolUpdate(i,Acontrols[i]))
    {
      flag=0;
      break;
    }
  }
  if(flag)
  {
    for(int i=0;i<(sizeof(notes)/2);i++)
    {
      if(noteUpdate(i,notes[i]))
      {
        flag=0;
        break;
      }
    }
   
  }

}


void rainbow(uint8_t wait) 
{
  uint16_t i, j;
  strip.setBrightness(bright);
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




void midiNoteOn(byte note, byte midiVelocity) // send a note on
{
  byte a[] = {NOTE_ON_CMD, note, midiVelocity};
  Wire.write(a,3);
}

void midiNoteOff(byte note, byte midiVelocity) // send a note off
{
  byte a[] = {NOTE_OFF_CMD, note, midiVelocity};
  Wire.write(a,3);
}

void midiControl(byte c, byte val) // send a midi control change
{ 
  byte a[] = {CONTROL_CHANGE_CMD,c,val};
  Wire.write(a,3);
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
   if (dex==5){
    bright=130-k1;
   }
   midiControl(dex,k1); // send the midi message on the channel associated with the index 
   //delay(100);
   return 1;
 }
 else
 {
   return 0;
 }
}


int DcontrolUpdate(int dex, int k1){ // literally the same but for digital controls.
int k1n=digitalRead(Dinputs[dex]);
 if(k1!=k1n)
 {
   k1=k1n;
   Dcontrols[dex]=k1;
   k1=k1*126;
   dex=dex+7;
   Serial.print(k1);
   midiControl(dex,k1); // add (arbitrary amount) to the index so the channels don't conflict with the analog controls
   //delay(100);
   return 1;
 }
 else
 {
   return 0;
 }
}


int noteUpdate(int dex, int k1)
{ // function to check and update analog pins
int k1n=analogRead(Ninputs[dex]); // read in the current state of the analog pin at k1 - (k1 new)
  float hold = k1n * convert;
  k1n = (int) hold;
 if(abs(k1-k1n)>8) // if the new value is different than the one we have...
 { 
  
  Serial.print("read value is: ");
   Serial.print(k1n);
   Serial.print(" , ");
   Serial.println(NoteNames[dex]);
  
   
  
   if((k1n>14) && (noteStatus[dex]==0)){
   midiNoteOn(NoteNames[dex],k1n); // send a note on
   Serial.println("on");
   noteStatus[dex]=1;

   }else if(k1n<14)
   {
    midiNoteOff(NoteNames[dex],k1n);
       Serial.println("off");
       noteStatus[dex]=0;
   }
   notes[dex]=k1n;
   //delay(100);
   return 1;
 }
 else
 {
   return 0;
 }
}
