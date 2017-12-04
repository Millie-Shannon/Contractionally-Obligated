%% CTG_Database_Threshold_Tester
% Purpose: 
%   This code reads ALL files from an online database of uterine
%   contractions (measured with a toco), performs filtering then calculates
%   the slopes; A threshold is defined based on a percentage of the max and
%   min of these slopes;
%   This code exists to test what the thresholds look like for each data
%   set in order to determine if for real-time data collection we would
%   need some type of moving average threshold;
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

% Read the signal from the website using the rdsamp function. 
all_thresholds=[];

% define the database entries you wish to analyze (refer to website!)
entries=[1001:1506 2001:2046];

% for each entry in your range...
for i=1:length(entries)
    
    % compile that entry's filename
    str=strcat('ctu-uhb-ctgdb/',num2str(entries(i)),'.dat');
    
    % pull signal from the site
    [tm,Fs]=rdsamp(str); % The rdsamp function has the website 'https://physionet.org/physiobank/database/' embedded within it.  You have to provide the rest of the website information for the database and file you want.
    time=0:1/Fs:(length(tm)-1)/Fs; % According to the website, the data was sampled at 4 Hz

    % % Plot the traces.  The signal contains both FHR and UC traces.  The first is FHR, the second is UC
    % figure;plot(time,tm(:,1));xlabel('Time (s)');ylabel('FHR (bpm)')
    % figure;plot(time,tm(:,2));xlabel('Time (s)');ylabel('UC (arb. units)') % Since the amplitude of the toco is meaningless, I put the y-axis as 'arbitrary units'.
    % 
    % % Plot a shorter section of the UC trace to allow for better visibility
    % figure;plot(time(1:3000),tm(1:3000,2));xlabel('Time (s)');ylabel('UC (arb. units)') 

    %% Filter Signal - Low Pass
    
    L=size(tm,1); % length of signal
    sig=tm(1:L,2); % set sig as only the 2nd column, the UC data

    fftSig=(fft(sig)); % fourier of signal

    fltr=fftSig.*0; % create a masking vector of 1s and 0s
    lowBnd=1;
    highBnd=85;
    fltr(lowBnd:highBnd)=1; 

    %figure;
    %plot(abs(fftSig));
    % title('FFT of input signal');
    % xlabel('Frequency');

    fltSig=fftSig.*fltr; % apply the filter to the signal
    %figure;
    %plot(abs(fltSig));
    % title('Frequency domain filtered input signal');
    % xlabel('Frequency');


    reconstr=ifft((fltSig)); % go back to time domain using filtered signal
    phase_rec=angle(reconstr);
    amplitude_rec=abs(reconstr);
    reconstrOut=2*amplitude_rec.*cos(phase_rec);
    x=1:length(sig);

    %figure;
    %plot(x,sig,'r',x,reconstrOut,'b');
    % title('FFT filtered, original signals');
    % xlabel('Time');
    % legend('Original','Filtered');
    
    %% Calculate slope of filtered signal

    % figure;
    % hold on;
    slopes=zeros(size(tm));
    for k=1:(L-1)
        slopes(k,2)=50*(reconstrOut(k+1,1)-reconstrOut(k,1))/(time(k+1)-time(k));
    end
    
    %% Define threshold w/ Moving Average
    
    thres=zeros(1,L);
    window=10; % time window in minutes
    for k=1:L
        if k<=window*60
            slopeMax=max(slopes(1:k,2));
            slopeMin=min(slopes(1:k,2));
            range=slopeMax-slopeMin;
            thres(k)=slopeMin+0.63*range;
        else
            slopeMax=max(slopes((k-window*60):k,2));
            slopeMin=min(slopes((k-window*60):k,2));
            range=slopeMax-slopeMin;
            thres(k)=slopeMin+0.63*range;
        end
    end
    
    %% Define threshold w/ % of Slope Range
    slopeMax=max(slopes);
    slopeMin=min(slopes);
    range=slopeMax(2)-slopeMin(2);
    thres_orig=(slopeMin(2)+0.63*range)*ones(1,length(thres));
    
    % plot(time(1:L),tm(1:L,2),'r');xlabel('Time (s)');ylabel('UC (arb. units)');
    % plot(time(1:L),slopes(1:L,2),'b');xlabel('Time (s)');ylabel('Signal Slope');
    % plot(time(1:L),reconstrOut,'k');
    % plot(time(1:L),thres,'m');
    % plot(time(1:L),thres_orig,'g');
    % hold off;
    % legend('Raw','Filt. Slopes','Filt. Sig','Thres Moving','Thres Orig');
    
    
    % add thresholds to a vector containing them for all files in database
    disp(thres_orig(1));
    all_thresholds=[all_thresholds thres_orig(1)];
end