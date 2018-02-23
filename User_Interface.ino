#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <TimerFreeTone.h> // for buzzer alarm 



// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to -1!
#define TFT_DC     8
#define TONE_PIN   53
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

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

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
int state = 3; 
int state2 = 1; 
int frequency[] = {2, 4, 8, 7, 1, 2}; 
int duration[] = {2, 6, 3, 8, 4, 2}; 
int count = 0; // note to self: count may actually be backwards 
int iter = 0; 
int barray[2];
int greenLEDPin = 48; 
int redLEDPin = 50; 

void setup() {
  // put your setup code here, to run once:
  tft.initR(INITR_BLACKTAB);
  //uint16_t time = millis();
  tft.fillScreen(ST7735_BLACK);
  //time = millis() - time;

  //Serial.println(time, DEC);
  //delay(500);
  Serial.begin(9600);
  pinMode(greenLEDPin, OUTPUT); 
  pinMode(redLEDPin, OUTPUT);  
}

void loop() {

  button1 = analogRead(A0);
  button2 = analogRead(A1); 
  button3 = analogRead(A2);
  button4 = analogRead(A3); 
  //Serial.println(button3);  
  
  // put your main code here, to run repeatedly:
  //tft.fillScreen(ST7735_BLACK);
  //testdrawtext("Frequency =", ST7735_WHITE);
//  testdrawtext(x, ST7735_WHITE);

if (iter > 0) {
  barray[0] = barray[1];
  barray[1] = button4; 
  iter = iter + 1; 
} else {
  barray[0] = button4; 
  iter = iter + 1; 
}
if (state == 3) {
    tft.setRotation(3);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(true);
    /*tft.print("Time: ");
    tft.print(time1);
    tft.print(" - ");
    tft.print(time2); 
    tft.println(" min.");
    tft.println(" "); */
    tft.println("Current State");
    tft.println(" "); 
    tft.print("Frequency: ");
    tft.println(frequency[5]);
    tft.println(" ");
    tft.print("Duration: ");
    tft.print(duration[5]); 
}
if (state == 0) {
  if (button1 == 1023 && time2 != tmax) {
    tft.fillScreen(ST7735_BLACK);
    time1 = time1 + 10; 
    time2 = time2 + 10; 
    count = count + 1; 
  }

  if (button2 == 1023 && time1 != 0) {
    tft.fillScreen(ST7735_BLACK);
    time1 = time1 - 10;
    time2 = time2 -10; 
    count = count -1; 
  }

//  if (state == 0) {
    tft.setRotation(3);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(2);
    tft.setTextWrap(true);
    tft.print("Time: ");
    tft.print(time1);
    tft.print(" - ");
    tft.print(time2); 
    tft.println(" min.");
    tft.println(" "); 
    tft.print("Frequency: ");
    tft.println(frequency[count]);
    tft.println(" ");
    tft.print("Duration: ");
    tft.print(duration[count]); 
  } 
// button 3 is the scrren switcher 
 if (button3 == 1023) {
  if (state == 0) {
    tft.fillScreen(ST7735_BLACK);
    state = 1; 
  } else if (state == 2) {
    tft.fillScreen(ST7735_BLACK);
    state = 3; 
//    redraw = true; 
  } else if (state == 3) {
    tft.fillScreen(ST7735_BLACK);
    state = 0;  
  }
}

  if (barray[1] > 1000 && barray[0] < 1000) {
    if (state2 == 1) {
      state2 = 0; 
    } else if (state2 == 0) {
      state2 = 1; 
    }
  }
/* Serial.println(button3); 
Serial.println(state); 
delay(1000); */


if (state == 1) {
    double x, y;
    tft.fillScreen(ST7735_BLACK);
    tft.setRotation(3);

    for (x = 0; x <= 10; x += .1) {
      y = sin(x) + 1;
      Graph(tft, x, y, 0, 120, 150, 115, 0, 10, 1, 0, 2, 2,DKBLUE, RED, YELLOW, BLACK, display1);
    }
    state = 2; 
    
  }

if (state == 2) { 
}

if (frequency[5] > 6 || frequency[5] < 3) {
  if (frequency[4] > 6 || frequency[4] < 3) {
    if (frequency[3] > 6 || frequency[3] < 3) {
      digitalWrite(redLEDPin, HIGH); 
      digitalWrite(greenLEDPin, LOW); 
      if (state2 == 1) {
          for (int thisNote = 0; thisNote < 4; thisNote++) {
      
          // to calculate the note duration, take one second
          // divided by the note type.
          //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
          int noteDuration = 1000 / noteDurations[thisNote];
          TimerFreeTone(TONE_PIN, melody[thisNote], noteDuration);
          
          // to distinguish the notes, set a minimum time between them.
          // the note's duration + 30% seems to work well:
          int pauseBetweenNotes = noteDuration * 1.30;
      }
    }
  } else {
    digitalWrite(redLEDPin, LOW); 
    digitalWrite(greenLEDPin, LOW); 
  }
 }
}
/*  if (button4 >= 1000) {
    if (state2 == 1) {
      state2 = 0; 
    } else if (state2 == 0) {
      state2 = 1; 
    }
  } */
Serial.println(button4);
Serial.println(state2);  
 // delay(100000);

delay(300);
}

void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void Graph(Adafruit_ST7735 &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, double ylo, double yhi, double yinc, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int bcolor, boolean &redraw) {

  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  double i;
  double temp;
  int rot, newrot;

  if (redraw == true) {

    redraw = false;
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

    /*  d.setTextSize(1);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      d.println(i); */
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

    /*  d.setTextSize(0.5);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(temp, gy + 10);
      // precision is default Arduino--this could really use some format control
      d.println(i); */
    }

 /*   //now draw the labels
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
    d.println(ylabel); */


  }

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
  redraw = true; 

}
