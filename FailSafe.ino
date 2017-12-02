int ledPin = 13;                // Connect LED on pin 13, or use the onboard one
int KEY = 3;                 // Connect Touch sensor on Digital Pin 3

#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247

int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);      // Set ledPin to output mode
  pinMode(KEY, INPUT);       //Set touch sensor pin to input mode
}
void loop()
{
Serial.println(digitalRead(KEY));
// outputs 0 when sensor is not being touched
// outputs 1 when sensor is being touched 
   if(digitalRead(KEY)==LOW)       //Read Touch sensor signal
 
     { 
        digitalWrite(ledPin, HIGH);   // if Touch sensor is HIGH, then turn on
        for (int thisNote = 0; thisNote < 8; thisNote++) {
    
        // to calculate the note duration, take one second
        // divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(8, melody[thisNote], noteDuration);
        
        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        // stop the tone playing:
        noTone(8);
       }
     }
   else
     {
        digitalWrite(ledPin, LOW);    // if Touch sensor is LOW, then turn off the led
     }
}
