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
entry='1312';

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

fltr=fftSig.*0; % create a masking vector of 1s and 0s
lowBnd=1;
highBnd=85;
fltr(lowBnd:highBnd)=1; 

%figure;
%plot(abs(fftSig)); % visualization of the freq. distribution
% title('FFT of input signal');
% xlabel('Frequency');


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
plot(x,sig,'r',x,reconstrOut,'b'); 
title('FFT filtered, original signals');
xlabel('Samples');
legend('Original','Filtered');

%% Calculate slope of filtered signal

% initialize vector
slopes=zeros(size(tm));

for i=1:(L-1)
slopes(i,2)=5*(reconstrOut(i+1,1)-reconstrOut(i,1))/(time(i+1)-time(i));
% the 50 is a multiplier used to bring the slope graph to a similar
% magnitude as the signals
end


%% Define threshold

% initialize vectors
thres=zeros(1,L);
window=10; % time window in minutes

% calculate threshold via moving average 
for i=1:L 
    if i<=window*60
        slopeMax=max(slopes(1:i,2));
        slopeMin=min(slopes(1:i,2));
        range=slopeMax-slopeMin;
        thres(i)=slopeMin+0.63*range;
    else
        slopeMax=max(slopes((i-window*60):i,2));
        slopeMin=min(slopes((i-window*60):i,2));
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
hold off;
legend('Raw','Filt. Sig','Filt. Slopes','Thres Moving','Thres Orig');
title('Signal with Overlaid Thresholds');

disp(thres_orig(1));

%% Output to Excel
savepath = '/Users/Patricia/Documents/Rice/7th Semester/Senior Design/';

xlswrite(strcat(savepath,'UC_Data_',entry),[time(1:L)' reconstrOut tm(1:L,2)]);