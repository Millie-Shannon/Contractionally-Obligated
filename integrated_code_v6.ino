/*description of variables
   val: used to receive data via Serial communication
   Fs:  sampling freq
   moving_avg_window: number of samples to be used in smoothing signal via moving average
   BT_calculation_window: minutes of data used to calculate basal tone (on first pass)
   thres: detection level for signal
   min_contr_length: length(sec) of shortest allowable contraction
   max_contr_length: length(sec) of longest allowable contraction
   max_iter: max number of times while loop for BT recalculation will iterate
   moving_window_index: counter used in moving average calculation
   max_BT_recalc_window: length(min) of max window in BT recalculation (secondary passes)
   moving_avg: small array with most recent raw signal values (used in moving averaging of func.)
*/

float val = 0;
int Fs = 4;
const uint16_t moving_avg_window = 10;
int BT_calculation_window = 5;
int thres = 10;
int min_contr_length = 25;
int max_contr_length = 120;
int max_iter = 5;
int moving_window_index = 0;
int sig_moving_avg_index = 0;
int max_num_contr = 100;
int num_samples;
int num_bins;
const uint16_t max_BT_recalc_num_samples = 1200;
int contr_counted = 0;
int compl_contr = 0;
int new_contr = 0; // this keeps track of the contractions you already found where they cross 0;

double moving_avg[moving_avg_window];
double BT_recalc[max_BT_recalc_num_samples];
double sig_moving_avg[max_BT_recalc_num_samples];
double sig_moving_avg_no_offset[max_BT_recalc_num_samples];
double basal_tone[max_BT_recalc_num_samples];

double test[max_BT_recalc_num_samples];

void setup()
{
  Serial.begin(9600);
    Serial.println('a');
    char a = 'b';
    while (a != 'a')
    {
      a = Serial.read();
    }
}

void loop() {
    if (Serial.available() > 0) {
      //moving average of incoming input, with averaging window of length moving_avg_window
      val = Serial.read();
      while (Serial.available()) {
        Serial.read();
      }
//  for (int b = 0; b < 550; b++) {
//    if (b < 20) {
//      val = 0;
//    }
//    else if (b < 280) {
//      val = 100;
//    }
//    else if (b < 320) {
//      val = 50;
//    }
//    else if (b < 530) {
//      val = 100;
//    }
//    else {
//      val = 0;
//    }
    moving_avg[moving_window_index % moving_avg_window] = val;
    int total = 0;
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
    sig_moving_avg[sig_moving_avg_index % max_BT_recalc_num_samples] = total;



    //first-pass basal tone calculation and removal; maybe set this so that the refresh is only
    //every ~10 secs or so?
    num_samples = BT_calculation_window * Fs * 60;
    if (sig_moving_avg_index < BT_calculation_window * Fs * 60) {
      num_samples = sig_moving_avg_index;
    }
    double temp [num_samples];
    for (int i = 0; i < num_samples; i++) {
      temp[i] = sig_moving_avg[sig_moving_avg_index - i];
    }
    double bin[1000];//make sure that the size of this is appropriate
    num_bins = hist_calc(bin, temp, num_samples);
    int sum = 0;
    int min_in_window = 0;
    while (min_in_window < num_bins && sum < round((num_samples / 10) )) {
      sum = sum + bin[min_in_window];
      min_in_window = min_in_window + 1;
    }

    basal_tone[sig_moving_avg_index % max_BT_recalc_num_samples] = min_in_window;
    sig_moving_avg_no_offset[sig_moving_avg_index % max_BT_recalc_num_samples] = sig_moving_avg[sig_moving_avg_index % max_BT_recalc_num_samples] - min_in_window;

    test[sig_moving_avg_index % max_BT_recalc_num_samples] = sig_moving_avg[sig_moving_avg_index % max_BT_recalc_num_samples] - min_in_window;

    //Peak Counting
    int num_changes = 1;
    int iter = 1;

    int count_new [max_num_contr][2];
    for (int i = 0; i < max_num_contr; i++) {
      count_new[i][0] = 0;
      count_new[i][1] = 0;
    }


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
            time_dif = (c + add - count_new[ind - 1][1]) / Fs / 60;
          }
          else {
            time_dif = 2;
          }
          if (time_dif > .000001) {
            count_new[ind][1] = c + add;
            ind = ind + 1;
          }
          c = c + add;
        }
        else {
          c = c + 1;
        }
      }


      //remove too short contractions
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

      //find start, end indices of each contraction
      compl_contr = 0; // # of complete contractions; i.e. # of non zero rows in count_new array
      while (count_new[compl_contr][0] != 0 && count_new[compl_contr][1] != 0) {
        compl_contr++;
      }

      for (int i = 0; i < compl_contr; i++) { // for each COMPLETED contraction...
        int k = count_new[i][0];
        while (k > 0 && test[k] > 0) {
          k--; //step back a sample until you reach a value of 0 but before you reach the end of samples
        }
        if (k != 0 && test[k] <= 0) {
          count_new[i][0] = k; // save that index as your new 1st crossover point
        }
        k = count_new[i][1]; // then move on to your end threshold crossover
        while (k < sig_moving_avg_index && test[k] > 0) {
          k++; //step forward a sample until you reach 0 and before you reach the last known sample
        }

        if (k != sig_moving_avg_index && test[k] <= 0) {
          //contr_counted++;
          count_new[i][1] = k; // save that index as your new 2nd crossover point
        }
      }

      //redo BT calculations as needed
      num_changes = 0;
      for (int i = 0; i < max_num_contr; i++) {
        if ((count_new[i][1] - count_new[i][0]) >= (max_contr_length * Fs)) {
          double temp [count_new[i][1] - count_new[i][0]];
          for (int j = count_new[i][0]; j <= count_new[i][1]; j++) {
            temp[j - count_new[i][0]] = test[j];
          }
          double bin[1000];//make sure that the size of this is appropriate
          num_bins = hist_calc(bin, temp, num_samples);
          int sum = 0;
          int min_in_window = 0;
          while (min_in_window < num_bins && sum < round((num_samples * iter / max_iter) )) {
            sum = sum + bin[min_in_window];
            min_in_window = min_in_window + 1;
          }
          for (int j = count_new[i][0]; j <= count_new[i][1]; j++) {
            test[j] = test[j] - (min_in_window - basal_tone[j]);
            basal_tone[j] = basal_tone[j] + (min_in_window - basal_tone[j]);
          }
          num_changes = num_changes + 1;
          for (int d = 0; d < num_samples; d++) {
          }
        }
      }

      iter = iter + 1;
    }

    Serial.println(test[sig_moving_avg_index]);
    for (int i = 0; i < 5; i++) {
      Serial.println(count_new[i][0]);
      Serial.println(count_new[i][1]);
    }
    Serial.println(compl_contr);
//Serial.print(b);
//Serial.print("\t");
//Serial.print(test[b]);
//Serial.print("\t");
//Serial.print(count_new[0][0]);
//Serial.print("\t");
//Serial.print(count_new[0][1]);
//Serial.print("\t");
//Serial.print(count_new[1][0]);
//Serial.print("\t");
//Serial.print(count_new[1][1]);
//Serial.println();
    //Update Counters
    sig_moving_avg_index = sig_moving_avg_index + 1;
    moving_window_index = moving_window_index + 1;
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
  for (int i = 0; i < nbin; i++)
  {
    pbin[i] = 0;
  }
  //step through signal and assign each value to a bin
  for (int i = 0; i < n; i++)
  {
    int temp = (int) round(sig[i] + 0.49);
    pbin[temp] = pbin[temp] + 1;
  } // end of FOR loop
  return nbin;
} // END of Function
