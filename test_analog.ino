int analogPin = 7;     // potentiometer wiper (middle terminal) connected to analog pin 3
int val = 0;           // variable to store the value read
float first_val = 0.0;
float curr_raw_val,filtered_curr_val;
int sig_moving_avg_index = 0;
int count [100][2];
int moving_avg[10];


unsigned long last_time = 0L;
int PERIOD = 250; // period in ms for 4Hz sampling frequency

void setup()
{
  Serial.begin(19200);              //  setup serial
}

void loop()
{
  //if (Serial.available() > 0) {
    //Serial.println("HI! serial IS available");
    if (millis() - last_time > PERIOD) {
      //Serial.println("Hello sampl. loop entered.");
      last_time = millis();
      int gain = 10;
      curr_raw_val = (float) analogRead(analogPin)*gain;
    if (sig_moving_avg_index == 0 || sig_moving_avg_index == 1){
      filtered_curr_val = curr_raw_val;
    }
    //1/45 Hz low-pass filter (13.3 contractions/10 minutes)
    filtered_curr_val = 0.9657 * filtered_curr_val + 0.0343 * curr_raw_val;

float curr = 10;
      float temp = curr / 5 / 3;

      Serial.print(curr_raw_val);
      Serial.print("\t");
      Serial.print(filtered_curr_val);    
      Serial.println();// print out value
      sig_moving_avg_index = sig_moving_avg_index + 1;
    }
  //}

}
