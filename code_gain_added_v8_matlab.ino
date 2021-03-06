/*description of variables
   val:                       used to receive data via Serial communication
   Fs:                        sampling freq
   moving_avg_window:         number of samples to be used in smoothing signal via moving average
   BT_calculation_window:     minutes of data used to calculate basal tone (on first pass)
   thres:                     detection level for signal
   min_contr_length:          length(sec) of shortest allowable contraction
   max_contr_length:          length(sec) of longest allowable contraction
   max_iter:                  max number of times while loop for BT recalculation will iterate
   moving_window_index:       counter used in moving average calculation
   sig_moving_avg_index:      keeps track of number of samples that have been collected
   max_num_contr:             artifically-imposed limit on max number of contractions that will be collected; used to
                              restrict array sizes
   num_samples:               number of samples that are within current calculation arrays (i.e. sig_moving_avg, basal_tone, etc.)
   num_bins:                  placeholder variable that is eventually assigned number of bins in histogram from hist_calc func
   max_BT_recalc_num_samples: length(min) of max window in BT recalculation (secondary passes)
   compl_contr:               keeps track of number of contractions completed in count_new 2d array

   count:                     stores start/stop indicies (in samples) of ALL contractions that occurred during data collection
   count_prev:                array of start/stop indices of contractions in BT recalc window from previous iteration of loop;
                              this is compared with the count_new array to determine when to add entries to count array
   moving_avg:                small array with most recent raw signal values (used in moving averaging of func.)
   sig_moving_avg:            stores smoothed (via moving avg) raw signal within BT recalc window
   sig_moving_avg_no_offset:  smoothed raw signal with BT removed; this array seems corrupted and the test array
                              is used in its place
   basal_tone:                stores basal tone (adjusted multiple times, if necessary), within BT recalc window

   test:                      correct, uncorrupted version of array with smoothed raw signal w/BT removed
*/
float val = 0;
int Fs = 4;
const uint16_t moving_avg_window = 10;
int BT_calculation_window = 1;
int thres = 20;
int base_thres = 3;
int min_contr_length = 10;
int max_contr_length = 60;
int max_iter = 15;
int moving_window_index = 0;
int sig_moving_avg_index = 0;
int max_num_contr = 100;
int num_samples;
int num_bins;
const uint16_t max_BT_recalc_num_samples = 720;
int compl_contr = 0;
int tail_end = 0;
float curr_raw_val,filtered_curr_val;

int count[100][2];
int count_prev[100][2];
int gain = 10;

float dur[18]; //18 10-min segments in 3 hour block
int freq[18];
float dur_temp;
int meh;
int tot_c_10;
int dur_ind = 0;

double moving_avg[moving_avg_window];
double sig_moving_avg[max_BT_recalc_num_samples];
double sig_moving_avg_no_offset[max_BT_recalc_num_samples];
double basal_tone[max_BT_recalc_num_samples];

double test[max_BT_recalc_num_samples];

unsigned long last_time = 0L;
unsigned long start_time = 0L;
unsigned long end_time = 0L;
int PERIOD = 250;

void setup()
{
  Serial.begin(19200);
    Serial.println('a');
  char a = 'b';
    while (a != 'a')
    {
      a = Serial.read();
    }
}

void loop() {
  // check if "copy data from serial monitor" button is presssed...
//  if (analogRead(0) == 1023) {
//    delay(30000);
//    Serial.println("Starting a new dataset! Time = 0;");
//  }
//
  if (millis() - last_time > PERIOD) {
    // update last_time w/ current time
    last_time = millis();
    start_time = millis();
   if(Serial.available() > 0){
//
//    //moving average of incoming input, with averaging window of length moving_avg_window
//    val = analogRead(3);
//    //clear input Serial buffer (completes reading of any input before proceeding further in code)
    curr_raw_val = (float) analogRead(7) * gain;
    if (sig_moving_avg_index < 5){
      filtered_curr_val = curr_raw_val;
    }
    else{
      filtered_curr_val = 0.9657 * filtered_curr_val + 0.0343 * curr_raw_val;
    }
    //1/30 Hz low-pass filter (20 contractions/10 minutes)
    //filtered_curr_val = 0.949 * filtered_curr_val + 0.05101 * curr_raw_val;
    
    //1/45 Hz low-pass filter (13.3 contractions/10 minutes)
    val = filtered_curr_val;
    
    //initialize values of count array (keeps track of all contractions during collection) on 1st iteration
    if (sig_moving_avg_index == 0) {
      for (int i = 0; i < max_num_contr; i++) {
        count[i][0] = 0;
        count[i][1] = 0;
      }
    }

    //STEP 1: calculate moving avg from appropriate number of data points
    moving_avg[moving_window_index % moving_avg_window] = val;
    float total = 0;
    if (moving_window_index < moving_avg_window) {
      for (int i = 0; i <= moving_window_index; i++) {
        total = total + moving_avg[i];
      }
      total = total / (moving_window_index + 1);
    }
    else {
      for (int i = 0; i < moving_avg_window; i++) {
        total = total + moving_avg[i];
      }
      total = total / moving_avg_window;
    }
    //assume moving avg value to the sig_moving_avg array
    if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
      for (int i = 1; i < max_BT_recalc_num_samples; i++) {
        sig_moving_avg[i - 1] = sig_moving_avg[i];
      }
      sig_moving_avg[max_BT_recalc_num_samples - 1] = total;
    }
    else {
      sig_moving_avg[sig_moving_avg_index] = total;
    }


    //STEP 2: first-pass basal tone calculation and removal
    num_samples = 2.5 * Fs * 60;
    if (sig_moving_avg_index < 2.5 * Fs * 60) {
      num_samples = sig_moving_avg_index;
    }

    int num_samples_2 = BT_calculation_window * Fs * 60;
    if (sig_moving_avg_index < num_samples_2) {
      num_samples_2 = sig_moving_avg_index;
    }
    
    //temp: array of most recent data points within BT_calculation_window interval
    double temp [num_samples_2];
    if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
      for (int i = 0; i < num_samples_2; i++) {
        temp[i] = sig_moving_avg[max_BT_recalc_num_samples - i]; //flips signal vector to have most recent before old
      }
    }
    else {
      for (int i = 0; i < num_samples_2; i++) {
        temp[i] = sig_moving_avg[sig_moving_avg_index - i]; //flips signal vector to have most recent before old
      }
    }

    double bin[1000];//arbitrarily initialize bin array to something larger than the bin array would have to
    //be in histogram calculations
    num_bins = hist_calc(bin, temp, num_samples_2);//hist_calc function returns bin (see func for more explanation)
    int sum = 0;
    int min_in_window = num_bins - 1000;
    //find value of temp that is 10th percentile of all values
    while (min_in_window < num_bins && sum < round((num_samples_2 / 5) )) {
      sum = sum + bin[min_in_window + 1000 - num_bins];
      min_in_window = min_in_window + 1;
    }
    //assign appropriate values to corresponding arrays
    if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
      for (int i = 1; i < max_BT_recalc_num_samples; i++) {
        basal_tone[i - 1] = basal_tone[i];
        sig_moving_avg_no_offset[i - 1] = sig_moving_avg_no_offset[i];
        test[i - 1] = test[i];
      }
      basal_tone[max_BT_recalc_num_samples - 1] = min_in_window;
      sig_moving_avg_no_offset[max_BT_recalc_num_samples - 1] = sig_moving_avg[max_BT_recalc_num_samples - 1] - min_in_window;
      test[max_BT_recalc_num_samples - 1] = sig_moving_avg[max_BT_recalc_num_samples - 1] - min_in_window;
    }
    else {
      basal_tone[sig_moving_avg_index] = min_in_window;
      sig_moving_avg_no_offset[sig_moving_avg_index] = sig_moving_avg[sig_moving_avg_index] - min_in_window;
      test[sig_moving_avg_index] = sig_moving_avg[sig_moving_avg_index] - min_in_window;
    }

//    // STEP 2.5: Failsafe
//    if (sig_moving_avg_index > 15) {
//      bool flatline = 1;
//      for (int i = 10; i < num_samples; i++) {
//        if (temp[i] > 0) {
//          flatline = 0;
//          break;
//        }
//      }
//      if (flatline) {
//        Serial.println("DEVICE IS DISCONNECTED!");
//      }
//    }

    //STEP 3: Peak Counting
    int num_changes = 1;
    int iter = 1;

    //count_new array will keep track of contractions within BT recalc window
    int count_new [max_num_contr][2];
    for (int i = 0; i < max_num_contr; i++) {
      count_new[i][0] = 0;
      count_new[i][1] = 0;
    }

    //find indices at which contractions cross threshold
    while (num_changes != 0 && iter <= max_iter) {
      int ind = 0;
      int c = 0;
      while (c < num_samples) {
        if (test[c] > thres) {
          count_new [ind][0] = c;
          int add = 1;
          while ((c + add) < num_samples && test[c + add] > thres) {
            add = add + 1;
          }
          int time_dif;
          if (ind > 0) {
            time_dif = (c + add - count_new[ind - 1][1]) ;
          }
          else {
            time_dif = 2 * Fs * 60;
          }
          if (time_dif > 0.03 * 60 * Fs) {
            count_new[ind][1] = c + add;
            ind = ind + 1;
          }
          c = c + add;
        }
        else {
          c = c + 1;
        }
      }

      //find start, end indices of each contraction (relative to basal_tone, not threshold)
      compl_contr = 0; // # of complete contractions; i.e. # of non zero rows in count_new array
      while (count_new[compl_contr][0] != 0 && count_new[compl_contr][1] != 0) {
        compl_contr++;
      }

      for (int i = 0; i < compl_contr; i++) { // for each COMPLETED contraction...
        int k = count_new[i][0];
        while (k > 0 && test[k] > base_thres) {
          k--; //step back a sample until you reach a value of 0 but before you reach the end of samples
        }
        count_new[i][0] = k; // save that index as your new 1st crossover point
        k = count_new[i][1]; // then move on to your end threshold crossover
        int max_index = sig_moving_avg_index;
        if (sig_moving_avg_index > max_BT_recalc_num_samples) {
          max_index = max_BT_recalc_num_samples;
        }
        while (k < max_index && test[k] > base_thres) {
          k++; //step forward a sample until you reach 0 and before you reach the last known sample
        }
        count_new[i][1] = k; // save that index as your new 2nd crossover point
      }

      //remove too short contractions (less than the min_contr_length)
      int counts_filtered [max_num_contr][2];
      for (int i = 0; i < max_num_contr; i++) {
        counts_filtered[i][0] = 0;
        counts_filtered[i][1] = 0;
      }
      int tmp = 0;
      for (int i = 0; i < max_num_contr; i++) {
        if ((count_new[i][1] - count_new[i][0]) >= Fs * min_contr_length) {
          counts_filtered[tmp][0] = count_new[i][0];
          counts_filtered[tmp][1] = count_new[i][1];
          tmp = tmp + 1;
        }
      }
      for (int i = 0; i < max_num_contr; i++) {
        count_new[i][0] = counts_filtered[i][0];
        count_new[i][1] = counts_filtered[i][1];
      }

      //STEP 4: redo BT calculations as needed
      num_changes = 0;
      for (int i = 0; i < max_num_contr; i++) {
        //reset BT if original BT results in contractions that are above max_contr_length
        if ((count_new[i][1] - count_new[i][0]) >= (max_contr_length * Fs)) {
          num_changes = 5;
          double temp [count_new[i][1] - count_new[i][0]];
          for (int j = count_new[i][0]; j <= count_new[i][1]; j++) {
            temp[j - count_new[i][0]] = test[j];
          }
 
          double bin[1000];//make sure that the size of this is appropriate
          num_bins = hist_calc(bin, temp, (count_new[i][1] - count_new[i][0]));
          int sum = 0;
          int min_in_window = num_bins - 1000;
          //iterate the BT based on while loop iteration, max number of iterations assigned
          while (min_in_window < num_bins && sum < round(((count_new[i][1] - count_new[i][0]) * iter / max_iter) )) {
            sum = sum + bin[min_in_window + 1000 - num_bins];
            min_in_window = min_in_window + 1;
          }
          for (int j = count_new[i][0]; j <= count_new[i][1]; j++) {
            test[j] = test[j] - (min_in_window - basal_tone[j]);
            basal_tone[j] = basal_tone[j] + (min_in_window - basal_tone[j]);
          }
        }
      }
      iter = iter + 1;
    }

    //STEP 5: store contraction start/end indices
    //initialize the count_prev array if on the 1st iteration
    if (sig_moving_avg_index == 0) {
      for (int i = 0; i < max_num_contr; i++) {
        int curr1 = count_new[i][0];
        int curr2 = count_new[i][1];
        count_prev[i][0] = curr1;
        count_prev[i][1] = curr2;
      }
    }

    if (sig_moving_avg_index == 0) {
      for (int i = 0; i < max_num_contr; i++) {
        count_prev[i][0] = count_new[i][0];
        count_prev[i][1] = count_new[i][1];
      }
    }
    else {
      for (int i = 0; i < max_num_contr; i++) {
        for (int j = i; j < max_num_contr; j++) {
          if (((count_prev[j][0] - count_new[i][0] == 1 && count_prev[j][1] - count_new[i][1] == 1) && count_new[i][0] == 1)) {
            //when start, stop indices of contractions in count_prev and count_new are offset by 1, this loop is run.
            //you wait until count_new[i][1] equals 1 because this allows the prospective contraction to traverse, through iterations
            //over the entire BT recalc window so that all BT recalculations can occur as needed
            int curr = 0;
            //find first index of count array that doesn't contain a contraction
            while (count[curr][0] != 0 && count[curr][1] != 0) {
              curr++;
            }
            //only make a change to count if the prospective contraction has not already be added
            int make_change = 1;
            for (int k = 0; k < max_num_contr; k++) {
              if (count_new[i][0] == count[k][0] || count_new[i][1] == count[k][1]) {
                make_change = 0;
              }
            }
            //assign contraction indices to count as needed
            if (make_change == 1) {
              if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
                count[curr][0] = count_new[i][0] + sig_moving_avg_index - max_BT_recalc_num_samples - 2;
                count[curr][1] = count_new[i][1] + sig_moving_avg_index - max_BT_recalc_num_samples - 2;
              }
              else {
                count[curr][0] = count_new[i][0];
                count[curr][1] = count_new[i][1];
              }
                            //if time has moved past current 10-min segment, move on to new index in dur vector
                            int display_win = 2400;
              if (sig_moving_avg_index > (display_win*(dur_ind + 1) + max_BT_recalc_num_samples)) {
                dur_ind++;
                tail_end = curr;
                if (tail_end < 0){
                  tail_end = 0;
                }
              }
                            // determine the average duration
              dur_temp = 0;
              meh = curr;
              tot_c_10 = 0;
              while ((sig_moving_avg_index - count[meh][0]) < (display_win + max_BT_recalc_num_samples) && (sig_moving_avg_index - count[meh][0]) > max_BT_recalc_num_samples && meh >= tail_end) {
                dur_temp = dur_temp + (count[meh][1] - count[meh][0]);
                tot_c_10++;
                meh--;
              }
              dur[dur_ind] = dur_temp / tot_c_10 / 240; // change from sum to average duration in past ten minutes
              freq[dur_ind] = tot_c_10;
            }
            break;
          }
        }
      }
      //reassign the count_prev array after all appropriate additions to count have been made
      for (int i = 0; i < max_num_contr; i++) {
        int curr1 = count_new[i][0];
        int curr2 = count_new[i][1];
        count_prev[i][0] = curr1;
        count_prev[i][1] = curr2;
      }
    }
    Serial.println(curr_raw_val);
//    Serial.println(filtered_curr_val);
    //send appropriate output via Serial communication
    if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
      Serial.println(test[max_BT_recalc_num_samples - 1]);
    }
    else {
      Serial.println(test[sig_moving_avg_index]);
    }
    if (sig_moving_avg_index >= max_BT_recalc_num_samples) {
      Serial.println(basal_tone[max_BT_recalc_num_samples - 1]);
    }
    else {
      Serial.println(basal_tone[sig_moving_avg_index]);
    }
      int counts_filtered [max_num_contr][2];
      int counter = 0;

      for (int i = 0; i < max_num_contr; i++) {
        counts_filtered[i][0] = 0;
        counts_filtered[i][1] = 0;
      }
      for (int i = 0; i < max_num_contr; i++) {
        int make_change = 1;
        int loop_min = i-2;
        int loop_max = i+2;
        if (loop_min < 0){
          loop_min = 0;
        }
        if (loop_max >= max_num_contr){
          loop_max = max_num_contr - 1;
        }
        for (int j = loop_min; j < loop_max; j++){
          if (count[i][0]==counts_filtered[j][0]&&count[i][1]==counts_filtered[j][1]){
            make_change = 0;
          }
        }
        if (make_change == 1){
          counts_filtered[counter][0] = count[i][0];
          counts_filtered[counter][1] = count[i][1];
          counter = counter + 1;
        }
      }

      for (int i = 0; i < max_num_contr; i++){
        count[i][0] = counts_filtered[i][0];
        count[i][1] = counts_filtered[i][1];
      }

          int curr = 0;
    
    //find first index of count array that doesn't contain a contraction
    while (count[curr][0] != 0 && count[curr][1] != 0) {
      curr++;
    }
    if (curr > 0) {
      curr = curr - 1;
    }

    
    Serial.println(count[curr][0]);
    Serial.println(count[curr][1]);
    Serial.println(dur[dur_ind]);
    Serial.println(freq[dur_ind]);

    end_time = millis();

    //Update Counters
    sig_moving_avg_index = sig_moving_avg_index + 1;
    moving_window_index = moving_window_index + 1;
  }
  }
}




int hist_calc(double pbin[], double sig[], int n)
{
  // find max value of signal
  int maxval = 0;
  for (int i = 0; i < n; i++) {
    if (sig[i] > maxval) {
      maxval = sig[i];
    }
  }
  int nbin = maxval + 1; // our max value also helps define the # of bins we have!
  // Ensure the all values in bin count vector start at 0
  for (int i = (nbin - 1000); i < nbin; i++)
  {
    pbin[i + 1000 - nbin] = 0;
  }
  //step through signal and assign each value to a bin
  for (int i = 0; i < n; i++)
  {
    int temp = (int) round(sig[i] + 0.49);
    if (temp < nbin || temp > (nbin - 1000)){
      pbin[temp + 1000 - nbin] = pbin[temp + 1000 - nbin] + 1;
    }
  } // end of FOR loop
  return nbin;
} // END of Function
