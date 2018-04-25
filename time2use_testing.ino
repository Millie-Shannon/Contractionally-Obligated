#include <SPI.h>          // f.k. for Arduino-1.5.2
#include <TimerFreeTone.h>
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;

// Assign human-readable names to some common 16-bit color values:
#define TONE_PIN   51
#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

#define NOTE_C4  262
#define NOTE_G3  196
#define NOTE_A3  220
#define NOTE_B3  247
#define NOTE_01  280

int melody[] = {
  NOTE_C4, NOTE_C4, NOTE_C4, 0, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

int melody2[] = {
  0, 0, 0, 0
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  2, 8, 4, 1, 4, 4, 4, 4
};

boolean display1 = true;
double ox , oy ;

int x = 10;

int button1;
int button2;
int button3;
int button4;
int time1 = 0;
int time2 = 10;
int tmax = 60;
int state = 1; // default state is 3
int state2 = 1; // state2 defines if alarm is on/off

int frequency[] = {7, 8, 8, 7, 7, 8};
int current_freq = frequency[5];
int duration[] = {3, 6, 3, 8, 4, 2};
int current_dur = duration[5];
int dur_freq_ind = 0; // this is an index; note to self: dur_freq_ind may actually be backwards

int iter = 0;
int barray[2];
int greenLEDPin = 39;
int redLEDPin = 41;

void setup(void);
void loop(void);

uint16_t g_identifier;

void setup(void) {

  // STEP 1: SETUP Screen 
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if (identifier == 0x7783) {
    Serial.println(F("Found ST7781 LCD driver"));
  } else if (identifier == 0x8230) {
    Serial.println(F("Found UC8230 LCD driver"));
  }
  else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else if (identifier == 0x9481)
  {
    Serial.println(F("Found 0x9481 LCD driver"));
  }
  else if (identifier == 0x9486)
  {
    Serial.println(F("Found 0x9486 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier = 0x9486;

  }

  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.fillScreen(BLACK);
} //END OF SETUP

#if defined(MCUFRIEND_KBV_H_)
uint16_t scrollbuf[320];    // my biggest screen is 320x480
#define READGRAM(x, y, buf, w, h)  tft.readGRAM(x, y, buf, w, h)
#else
uint16_t scrollbuf[320];    // Adafruit only does 240x320
// Adafruit can read a block by one pixel at a time
int16_t  READGRAM(int16_t x, int16_t y, uint16_t *block, int16_t w, int16_t h)
{
  uint16_t *p;
  for (int row = 0; row < h; row++) {
    p = block + row * w;
    for (int col = 0; col < w; col++) {
      *p++ = tft.readPixel(x + col, y + row);
    }
  }
}
#endif


void loop(void) {
  uint8_t aspect;
  uint16_t pixel;
  char *aspectname[] = {
    "PORTRAIT", "LANDSCAPE", "PORTRAIT_REV", "LANDSCAPE_REV"
  };
  char *colorname[] = { "BLUE", "GREEN", "RED", "GRAY" };
  uint16_t colormask[] = { 0x001F, 0x07E0, 0xF800, 0xFFFF };
  uint16_t dx, rgb, n, wid, ht;
  tft.setRotation(0);

  pinMode(31, INPUT);
  digitalWrite(31, HIGH);
  pinMode(33, INPUT);
  digitalWrite(33, HIGH);
  pinMode(35, INPUT);
  digitalWrite(35, HIGH);
  pinMode(37, INPUT);
  digitalWrite(37, HIGH);

  button1 = digitalRead(31);
  button2 = digitalRead(33);
  button3 = digitalRead(35);
  button4 = digitalRead(37);

  // STATE 3: Current State Screen
  if (state == 3) {
    tft.setRotation(3);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(5);
    tft.setTextWrap(true);
    tft.println("Current State");
    tft.println(" ");
    tft.print("Frequency: ");
    tft.println(current_freq);
    tft.println(" ");
    tft.print("Duration: ");
    tft.print(current_dur);
  }

  // STATE 0: HISTORY Screen 
  if (state == 0) {
    // BUTTON 1: scroll forward in time
    if (button1 == LOW && time2 != tmax) {
      time1 = time1 + 10;
      time2 = time2 + 10;
      dur_freq_ind = dur_freq_ind + 1;
      tft.fillScreen(BLACK);
    }
    //BUTTON 2: scroll backward in time
    if (button2 == LOW && time1 != 0) {
      tft.fillScreen(BLACK);
      time1 = time1 - 10;
      time2 = time2 - 10;
      dur_freq_ind = dur_freq_ind - 1;
    }

    tft.setRotation(3);
    tft.setCursor(0, 0);
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.setTextWrap(true);
    tft.print("Time: ");
    tft.println(" ");
    tft.print(time1);
    tft.print(" - ");
    tft.print(time2);
    tft.println(" min.");
    tft.println(" ");
    tft.print("Frequency: ");
    tft.println(frequency[dur_freq_ind]);
    tft.println(" ");
    tft.print("Duration: ");
    tft.print(duration[dur_freq_ind]);

    Serial.print(dur_freq_ind);
    Serial.print("/t");
    Serial.print(frequency[dur_freq_ind]);
    Serial.print("/t");
    Serial.println(duration[dur_freq_ind]);
    
  } // end of STATE 0


  // BUTTON 3: Set State
  if (button3 == LOW) {
    if (state == 0) {
      tft.fillScreen(BLACK);
      state = 1;
    } else if (state == 2) {
      tft.fillScreen(BLACK);
      state = 3;
      //    redraw = true;
    } else if (state == 3) {
      tft.fillScreen(BLACK);
      state = 0;
    }
  }

  // BUTTON 4: Set State2
  if (button4 == LOW) {
    if (state2 == 1) {
      state2 = 0;
    } else if (state2 == 0) {
      state2 = 1;
    }
  }

  // STATE 1: Plot Trace
  if (state == 1) {
    double x, y;
    tft.fillScreen(BLACK);
    tft.setRotation(3);

    //             inputs, gx, gy, w,  h,    xlo,xhi,xinc,ylo,yhi,yinc
    Graph_Setup(tft, 0, 0, 50, 290, 390, 260, 0, 10, 1, -10, 20, 10, "Contractions", " Time [s]", "UC [arb. units]", DKBLUE, RED, GREEN, WHITE, BLACK);

    // 2400 pts = 10 minutes of data at 4 hz
    // graph one point at a time
    for (x = 0; x <= 120; x += .1) {
      y = 3*sin((0.42)*x) +3;
      //       inputs, gx, gy, w,  h,    xlo,xhi,xinc,ylo,yhi,yinc
      Graph(tft, x, y, 50, 290, 390, 260, 0, 120, 10, -10, 20, 50, "CONTRACTIONS", " Time [s]", "UC [arb. units]", DKBLUE, RED, GREEN, WHITE, BLACK);
    }
    
    // switch to state 2 to stop from continuously plotting 
    state = 2;

  }

  // STATE 2: Do Nothing
  if (state == 2) {
  }

  
  // Check for dangerous ranges!
  if (frequency[5] > 6 || frequency[5] < 3) {
    if (frequency[4] > 6 || frequency[4] < 3) {
      if (frequency[3] > 6 || frequency[3] < 3) {
        // if frequency has been high or low for more than 30 minutes do the following...
        digitalWrite(redLEDPin, HIGH); //turn ON red LED
        digitalWrite(greenLEDPin, LOW);
        if (state2 == 1) { // play tone
          Serial.println("Playing Tone!");
          
          for (int thisNote = 0; thisNote < 4; thisNote++) {

            // to calculate the note duration, take one second
            // divided by the note type.
            //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
            int noteDuration = 250 / noteDurations[thisNote];
            TimerFreeTone(TONE_PIN, melody[thisNote], noteDuration);

            // to distinguish the notes, set a minimum time between them.
            // the note's duration + 30% seems to work well:
            int pauseBetweenNotes = noteDuration * 5;
          }
        }
      }
    }
  } 
  else { // if not dangerous... 
    digitalWrite(redLEDPin, LOW); //turn OFF red LED
    digitalWrite(greenLEDPin, HIGH);
  }

  delay(300);

} //END OF VOID LOOP



void Graph(Adafruit_GFX & d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor) {

  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  double i;
  double temp;
  int rot, newrot;

  //graph drawn now plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized as static above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox, oy, x, y, pcolor);
  d.drawLine(ox, oy + 1, x, y + 1, pcolor);
  d.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

}

void Graph_Setup(Adafruit_GFX & d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor) {

  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  double i;
  double temp;
  int rot, newrot;

  ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  // draw y scale
  for ( i = ylo; i <= yhi; i += yinc) {
    // compute the transform
    temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

    if (i == 0) {
      d.drawLine(gx, temp, gx + w, temp, acolor);
    }
    else {
      d.drawLine(gx, temp, gx + w, temp, gcolor);
    }

    d.setTextSize(1);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx - 40, temp);
    // precision is default Arduino--this could really use some format control
    d.println(i);
  }
  // draw x scale
  for (i = xlo; i <= xhi; i += xinc) {

    // compute the transform

    temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
    if (i == 0) {
      d.drawLine(temp, gy, temp, gy - h, acolor);
    }
    else {
      d.drawLine(temp, gy, temp, gy - h, gcolor);
    }

    d.setTextSize(1);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(temp, gy + 10);
    // precision is default Arduino--this could really use some format control
    d.println(i);
  }

  //now draw the labels
  d.setTextSize(2);
  d.setTextColor(tcolor, bcolor);
  d.setCursor(gx , gy - h - 30);
  d.println(title);

  d.setTextSize(1);
  d.setTextColor(acolor, bcolor);
  d.setCursor(gx , gy + 20);
  d.println(xlabel);

  d.setTextSize(1);
  d.setTextColor(acolor, bcolor);
  d.setCursor(gx - 30, gy - h - 10);
  d.println(ylabel);

}
