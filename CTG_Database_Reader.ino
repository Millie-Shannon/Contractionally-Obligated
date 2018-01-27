#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); 

String incomingByte;
const uint16_t samples = 1024; // Must be a power of 2
const double signalFrequency = 1000;
const double samplingFrequency = 5000;
const uint8_t amplitude = 100;
const uint16_t Fs = 4;
float lowF = 0.0005; 
float highF = 0.02; 
uint16_t array_cut_low = floor(1+(lowF/(Fs))*2*samples);
uint16_t array_cut_high = ceil(1+(highF/(Fs))*2*samples);
uint16_t slope_window= Fs*(60 * 1);
const uint16_t window=10; 
float ratio = 0.67;
int range;

double vReal[samples];
double vImag[samples]; 
double fltr[samples]; 
double phase_rec[samples]; 
double amplitude_rec[samples];
double reconstrOut[samples];
double slopes[samples];
double slopes2[samples];
double t[samples];
double thres[samples];
double array1[samples];
double slopes_temp[samples];
//double x[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02

void setup() 
{
  //Serial.begin(9600);
  Serial.begin(10800); 
}

void loop() {
  // Get signal from CTG database 
  if (Serial.available() > 0) { 
    for(int i = 0; i < samples; i++) {
      incomingByte = Serial.read();
      vReal[i] = incomingByte.toDouble();
      vImag[i] = 0.0; 
      fltr[i] = 0.0; // masking vector of zeros 
      slopes2[i] = i + 1; 
      t[i] = (0.25*i)/60;
      delay(1);
    }

    
    //PrintVector(vReal, samples, SCL_TIME);
    // Filter Signal 
    //FFT.Windowing(vReal,samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples,FFT_FORWARD); 
    // vReal now contains the real portion of the FFT output
    // vImag now contains the imaginary portion of the FFT output

    //PrintVector(vReal, samples, SCL_INDEX);
    //PrintVector(vImag, samples, SCL_INDEX);
    
   // apply filter to signal
    for (int i = (array_cut_low-1); i < array_cut_high; i++) {
      fltr[i] = 1; 
    }
    
    //PrintVector(fltr, samples, SCL_INDEX);
    
    for (int i = 0; i < samples; i++) {
      vReal[i] = vReal[i]*fltr[i];
      vImag[i] = vImag[i]*fltr[i]; 
    }

    //PrintVector(vReal, samples, SCL_INDEX);
    //PrintVector(vImag, samples, SCL_INDEX);
    
    // apply inverse fft using complex conjugates of forward fft 
    for (int i = 0; i < samples; i++) {
      vReal[i] = vReal[i]*-1;
      vImag[i] = vImag[i]*-1; 
    }

    //FFT.Windowing(vReal,samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples,FFT_FORWARD);

    for (int i = 0; i < samples; i++) {
      vReal[i] = (vReal[i]*-1);
      vImag[i] = (vImag[i]*-1); 
    }
    
     for (int i=0; i< samples; i++)
    {
      fltr[i] = vReal[i];
    }

     for (int i=0; i< samples; i++)
    {
      vReal[i] = fltr[samples-i]/samples;
    }

    for (int i=0; i< samples; i++)
    {
      fltr[i] = vImag[i];
    }

    for (int i=0; i< samples; i++)
    {
      vImag[i] = fltr[samples-i]/samples;
    }

    
    //PrintVector(vReal, samples, SCL_INDEX);
    //PrintVector(vImag, samples, SCL_INDEX);
    
    // go back to the time domain
    for (int i = 0; i < samples; i++) {
      phase_rec[i] = atan2(vImag[i],vReal[i]);
      amplitude_rec[i] = sqrt(sq(vImag[i]) + sq(vReal[i]));
      reconstrOut[i] = 2*amplitude_rec[i]*cos(phase_rec[i]); 
      //x[i] = i + 1; 
    }
    
    //PrintVector(reconstrOut, samples, SCL_INDEX);
    //PrintVector(phase_rec, samples, SCL_INDEX);
    //PrintVector(amplitude_rec, samples, SCL_INDEX);


   // 
   // Caulculate slopes of filtered signal

   for (int i = slope_window + 1; i < samples; i++) {
      slopes[i] = 1*(reconstrOut[i]-reconstrOut[i-slope_window])/(t[i]-t[i-slope_window]);
   }
   

  
   for (int i = 0; i < (samples); i++) {
    
    //double slopes_temp[i];
    
    if(i <= window*60*Fs+1) {
      for(int k = 0; k < (i+1); k++) {
        slopes_temp[k] = slopes[k];  
      }
     
      float slopeMax  = slopes_temp[0]; 
      for(int m = 1; m < i; m++) {
        if (slopes_temp[m] > slopeMax) {
          slopeMax = slopes_temp[m];
        } 
      }
        
     //array1[i] = slopeMax;
     
    //}

      int slopeMin  = slopes_temp[0]; 
      for(int m = 1; m < i; m++) {
        if (slopes_temp[m] < slopeMin) {
          slopeMin = slopes_temp[m];
        }
      }

      array1[i] = slopeMin;
      
      range = slopeMax - slopeMin; 
      
      thres[i] = slopeMin + ratio*range;      
    }
    
   /* else {
      for(int k = 0; k < i; k++) {
        //double slopes_temp[i];
        slopes_temp[i] = slopes[i];  
      }
      
      int slopeMax  = slopes_temp[0]; 
      for(int m = 1; m < i; m++) {
        if (slopes_temp[m] > slopeMax) {
          slopeMax = slopes_temp[m];
        }
      }

      int slopeMin  = slopes_temp[0]; 
      for(int m = 1; m < i; m++) {
        if (slopes_temp[m] < slopeMin) {
          slopeMin = slopes_temp[m];
        }
      } 
      range = slopeMax - slopeMin; 
      thres[i] = slopeMin + ratio*range;           
    }    
    } */
    
   
  /* for (int i = 0; i < samples; i++) {
    Serial.print(thres[i]);
    Serial.print(" "); 
   } */
   }
   
   PrintVector(array1, samples, SCL_INDEX);
   while(1); 
   //delay(2000);
        

  }
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < samples; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
  break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
  break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
  break;
    }
    //Serial.print(abscissa, 6);
    //Serial.print(" ");
    Serial.print(vData[i], 4);
    Serial.print(" ");
    delay(1);
  }
  //Serial.println();
}

/* float calc_max(double A, int S) {
  // A is the array, S is the size of the array 
  int maxVal  = A[0]; 
  for(i = 1; i < S; i++) {
    if (A[i] > maxVal) {
      maxVal = A[i];
    }
  }
  return maxVal 
}

float calc_min(double A,int S) {
  // A is the array, S is the size of the array 
  int minVal  = A[0]; 
  for(i = 1; i < S; i++) {
    if (A[i] < minVal) {
      minVal = A[i];
    }
  }
  return minVal 
} */

 /* void ifft(double *VR, double *VI, uint16_t samples) {
  for (i = 0; i < samples; i++) {
    vReal[i] = VR[i]*-1;
    vImag[i] = VI[i]*-1; 
  }

  FFT.Windowing(vReal,samples, FFT_WIN_TYPE_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, samples,FFT_FORWARD);

  for (i = 0; i < samples; i++) {
    vReal[i] = vReal[i]*-1;
    vImag[i] = vReal[i]*-1; 
  }  
  
} */
