//this program simulates contractions
//it produces contractions with duration 30-120 secs at varying frequencies, with up to 90 secs.
//b/n and no fewer than 50 seconds. between
//set randomly generated duration and frequency numbers & times as integers:
float randNumber_dur;
int randNumber_freq;
float randNumber_dur_time;
int randNumber_freq_time;
int air_in = 13; //defines PIN13 to light the LED indicating air should enter ball
int air_out = 12; //defines PIN12 to light the LED indicating air should leave ball
int motor = 7; //defines PIN7 to turn on/off the motor by controling the mosfet gate
int sol_1 = 11; //defines PIN11 to open (high) and close (low) this solenoid
int sol_2 = 2; //defines PIN2 to open (high) and close (low) this solenoid
int sol_3 = 3; //defines PIN3 to open (high) and close (low) this solenoid
int sol_4 = 4; //defines PIN4 to open (high) and close (low) this solenoid

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //setup serial monitor
  randomSeed(analogRead(0)); //initializes the random number generator
  pinMode(air_in, OUTPUT); //sets up PIN13
  pinMode(air_out, OUTPUT); //sets up PIN12
  pinMode(motor, OUTPUT); //sets up motor to be output
  pinMode(sol_1, OUTPUT); //sets up solenoid to be output
  pinMode(sol_2, OUTPUT); //sets up solenoid to be output
  pinMode(sol_3, OUTPUT); //sets up solenoid to be output
  pinMode(sol_4, OUTPUT);//sets up solenoid to be output
}

void loop() {
  // run the following continuously:
  //DURATION (contraction generation):
  randNumber_dur = random(30, 130); //generate random number from 30 to 120
  randNumber_dur = 30;
  //randNumber_dur = 20; for solenoid practice
  // randNumber_dur = random(3, 12); //generate random number from 3 to 12 for troubleshooting efficiency
  Serial.print("duration = ");  Serial.println(randNumber_dur); //display selected duration
  digitalWrite(air_in, HIGH); //turn on the LED indicating air should enter ball
  //Solenoid States - Inflate
  digitalWrite(sol_2, HIGH); //opens solenoid2 - allow air to enter ball
  digitalWrite(sol_4, HIGH); //opens solenoid4 - allow pump to pull in air from outside of system

  digitalWrite(motor, HIGH);//turns motor on
  Serial.println("LED ON");
  randNumber_dur_time = 41.5*(randNumber_dur)/100; //change delay time into milliseconds
  //break up delay time into durations short enough to be readable
  for (int i = 0; i < 1000; i ++){
    delay(randNumber_dur_time);
  }
  Serial.println("1/4");

  digitalWrite(air_in, LOW); //turn off the LED indicating air should enter ball
  digitalWrite(air_out, HIGH); //turn on the LED indicating air should leave ball
  //Solenoid States - Deflate
  digitalWrite(sol_1, HIGH); //opens solenoid1 to release air from ball
  digitalWrite(sol_3, HIGH); //opens solenoid3 to allow pump to pull air from ball
  digitalWrite(sol_2, LOW); //closes solenoid 2 - cannot put more air in ball
  digitalWrite(sol_4, LOW); //closes solenoid4 - cannot pull more air from outside
    for (int i = 0; i < 1000; i ++){
    delay(58.5*(randNumber_dur)/100);
  }
  digitalWrite(air_out, LOW); //turn off the LED indicating air should leave ball
  digitalWrite(motor, LOW); //end of contraction - turns motor off until next contraction
  //Solenoid States - Break; all closed to keep air in ball; solenoids 2 and 4 are already closed
  digitalWrite(sol_1, LOW);
  digitalWrite(sol_3, LOW);

  Serial.println("LED OFF");
  /*if (randNumber_dur > 120) { //Optoco should alert user when this serial monitor warning is displayed
    //  if (randNumber_dur < 3 || randNumber_dur > 12) {
    //Serial.print("dur danger! = "); Serial.println(randNumber_freq);
    }*/
  //FREQUENCY (break b/n contractions)
  randNumber_freq = random(50, 90); //generate random number from 50 to 90; under 60 is dangerous
  randNumber_freq = 5;
  //randNumber_freq = random(15, 30); //generate random number from 5 to 9 for troubleshooting efficiency
  randNumber_freq_time = (randNumber_freq * 1000) / 4;
  Serial.print("frequency = "); Serial.println(randNumber_freq); //change delay time into milliseconds
  //break up delay time into durations short enough to be readable
  delay(randNumber_freq_time);
  Serial.println("1/4");
  delay(randNumber_freq_time);
  Serial.println("Half way there!");
  delay(randNumber_freq_time);
  Serial.println("3/4");
  delay(randNumber_freq_time);
  //if the contraction is outside the safe physiological range, display SOMETHING
  //so we know if our device pickedup on it and alerted the user:
  if (randNumber_freq < 60 || randNumber_freq > 90) { //Optoco should alert user when this serial monitor warning is displayed
    //   if (randNumber_freq < 15 || randNumber_freq > 29) {
    Serial.print("freq danger! = "); Serial.println(randNumber_freq);
  }

}
