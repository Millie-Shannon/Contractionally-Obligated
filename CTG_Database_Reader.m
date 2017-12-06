%% CTG_Database_Reader 
% Purpose: 
%   This code reads a specific file from an online database of uterine
%   contractions (measured with a toco), performs filtering then uses a
%   thresholding system to detect contraction peaks
%
% Notes: 
%   rdsamp outputs sig & Fs
%       sig = N x M matrix where there are M signals (in our case 2, one is FHR
%       one is UC) w/ ea. signal comprising one column
%       Fs = 1 x M vector that contains the sampling frequency of the data; in
%       our case this is 4Hz

%% Setup

clear all; close all; clc;

% Modify location for where you unzipped the wfdp toolkit files, the path
% needs to be set to the 'mcode' folder
% addpath('D:\wfdb-app-toolbox-0-10-0\mcode') % Dr. Carns' path
%addpath('C:\Users\atolp\Documents\MATLAB\mcode'); % Aniket's path
addpath('/Users/Patricia/Documents/MATLAB/mcode'); % Patricia's path

%% Get signal from database

% define entry to analyze
entry='1001';

% Read the signal from the website using the rdsamp function. 
[tm,Fs]=rdsamp(strcat('ctu-uhb-ctgdb/',entry,'.dat')); % The rdsamp function has the website 'https://physionet.org/physiobank/database/' embedded within it.  You have to provide the rest of the website information for the database and file you want.
time=0:1/Fs:(length(tm)-1)/Fs; % According to the website, the data was sampled at 4 Hz
time = time./60; % convert from seconds to minutes

% Plot the traces.  The signal contains both FHR and UC traces.  The first is FHR, the second is UC
figure;plot(time,tm(:,2));xlabel('Time (min)');ylabel('UC (arb. units)'); title('Raw Signal'); % Since the amplitude of the toco is meaningless, I put the y-axis as 'arbitrary units'.

% Plot a shorter section of the UC trace to allow for better visibility
figure;plot(time(1:3000),tm(1:3000,2));xlabel('Time (min)');ylabel('UC (arb. units)'); title('Raw Signal (small)');


%% Filter - Low pass

L=size(tm,1); % length of signal
sig=tm(1:L,2); % set sig as only the 2nd column, the UC data

fftSig=(fft(sig)); % fourier of signal

% Method 1 - online
f = Fs * (0:(L/2))/L; % frequency domain captured in fft 
P2 = abs(fftSig/L); % amplitude spectrum (???)
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

figure;
plot(f,P1);
title('Fourier of Signal - Online Method')


% Method 2 - pat and aniket
f2 = Fs*(0:L-1)/(2*L); % frequency domain; same length as signal
figure;
plot(f2,abs(fftSig));
title('Fourier of Signal - Same Length')

fltr=fftSig.*0; % create a masking vector of 1s and 0s

lowF=0.0005; % low cutoff frequency (Hz);
highF = 0.025; % high cutoff frequency (Hz); 

% Pat tried finding the index of the cutoff frequencies in the f2 vector
% index_lo = find(f2 == lowF);
% index_hi = find(f2 == highF);
% fltr(index_lo:index_hi) = 1;

% Aniket mapped frequency to sample index using f = Fs * (0:(L/2))/L or f = Fs*(0:L-1)/(2*L);
array_cut_low=floor(1+(lowF/(Fs))*2*L)
array_cut_high=ceil(1+(highF/(Fs))*2*L)
fltr(array_cut_low:array_cut_high)=1;


fltSig=fftSig.*fltr; % apply the filter to the signal
% figure;
% plot(abs(fltSig));
% title('Frequency domain filtered input signal');
% xlabel('Frequency');


reconstr=ifft((fltSig)); % go back to time domain using filtered signal
phase_rec=angle(reconstr);
amplitude_rec=abs(reconstr);
reconstrOut=2*amplitude_rec.*cos(phase_rec);
x=1:length(sig);

%% Plot filtered and original signal

figure;
plot(time(1:L),sig,'r',time(1:L),reconstrOut,'b'); 
title('FFT filtered, original signals');
xlabel('Time (min)');
legend('Original','Filtered');

%% Calculate slope of filtered signal

% initialize vector
slopes=zeros(size(tm));

for i=1:(L-1)
slopes(i,2)=1*(reconstrOut(i+1,1)-reconstrOut(i,1))/(time(i+1)-time(i));
% the 50 is a multiplier used to bring the slope graph to a similar
% magnitude as the signals
end


%% Define threshold

% initialize vectors
thres=zeros(1,L);
window=10; % time window in minutes

% time=0:1/Fs:(length(tm)-1)/Fs *** REMINDER! ea. sample is spaced as such

% calculate threshold via moving average 
for i=1:L 
    if i<=window*60*Fs
        slopeMax=max(slopes(1:i,2));
        slopeMin=min(slopes(1:i,2));
        range=slopeMax-slopeMin;
        thres(i)=slopeMin+0.63*range;
    else
        slopeMax=max(slopes((i-window*60*Fs):i,2));
        slopeMin=min(slopes((i-window*60*Fs):i,2));
        range=slopeMax-slopeMin;
        thres(i)=slopeMin+0.63*range;
    end
end
    
% calculate threshold via range percentage
slopeMax=max(slopes);
slopeMin=min(slopes);
range=slopeMax(2)-slopeMin(2);
thres_orig=(slopeMin(2)+0.63*range)*ones(1,length(thres));

%% Plot signal (original+filt), slopes, and threshold (2 types)

figure;
hold on;
plot(time(1:L),tm(1:L,2),'r');xlabel('Time (min)');ylabel('UC (arb. units)');
plot(time(1:L),reconstrOut,'k');
plot(time(1:L),slopes(1:L,2),'b');xlabel('Time (min)');ylabel('Signal Slope');
plot(time(1:L),thres,'m');
plot(time(1:L),thres_orig,'g');
%hold off;
legend('Raw','Filt. Sig','Filt. Slopes','Thres Moving','Thres Orig');
title('Signal with Overlaid Thresholds');

%% Peak Counting
count = [];
ind = 1;
k = 1;

while k < length(slopes)
    % if you are above the threshold...
    if slopes(k,2) > thres_orig(1)
        count(ind,1) = k;  % save the index of 1st crossover point
        add = 1;
        while slopes(k+add,2) > thres_orig(1) %keep adding to index until you're under thresh
            add = add + 1;
        end
        
        % once you're below threshold, check to make sure peak isn't too
        % close to previous one...
        if ind > 1
            hi = k+add
            hi2 = count(ind-1,1)
            time_dif = (k+add-count(ind-1,1))/Fs/60
        else
            time_dif = 2;
        end
        
        if time_dif > 1 % only if the peak is further than 1 min apart
            count(ind,2) = k + add; % save that 2nd crossover point
            ind = ind + 1; % advance the index of your count matrix
        end
        
        k = k+ add; % make sure to start from the (k+add)th sample next loop

    % if you aren't above the threshold...
    else
        % just keep advancing your index
        k = k + 1;
    end
    
end
count
tot_contr = length(count);
disp('Our contraction count:');
disp(tot_contr);

% plot the found points
y_contr = ones(1,tot_contr).*thres_orig(1);
plot(time(count(:,1)),y_contr,'mx','LineWidth',12);
hold off;


%% Output to Excel
savepath = '/Users/Patricia/Documents/Rice/7th Semester/Senior Design/';

%xlswrite(strcat(savepath,'UC_Data_',entry),[time(1:L)' reconstrOut tm(1:L,2)]);
%xlswrite(strcat(savepath,'f1_',entry),f');
%xlswrite(strcat(savepath,'f2_',entry),f2');