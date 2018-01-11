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
entry='1002';

% Read the signal from the website using the rdsamp function. 
[tm,Fs]=rdsamp(strcat('ctu-uhb-ctgdb/',entry,'.dat')); % The rdsamp function has the website 'https://physionet.org/physiobank/database/' embedded within it.  You have to provide the rest of the website information for the database and file you want.
time=0:1/Fs:(length(tm)-1)/Fs; % According to the website, the data was sampled at 4 Hz
time = time./60; % convert from seconds to minutes

% Plot the raw signal.  The signal contains both FHR and UC traces.  The first is FHR, the second is UC
figure;plot(time,tm(:,2));xlabel('Time (min)');ylabel('UC (arb. units)'); title('Raw Signal'); % Since the amplitude of the toco is meaningless, I put the y-axis as 'arbitrary units'.

% Plot a shorter section of the UC trace to allow for better visibility
figure;plot(time(1:3000),tm(1:3000,2));xlabel('Time (min)');ylabel('UC (arb. units)'); title('Raw Signal (small)');

L=size(tm,1); % length of signal
sig=tm(1:L,2); % set sig as only the 2nd column, the UC data

%% Get signal from excel - Optoco Data

% % select folder where excel file is located
% [fname, pname] = uigetfile('*.*','MultiSelect', 'on','Choose spreadsheet for analysis');
% 
% % read in both columns 
% time = xlsread(strcat(pname,fname), 'A:A');
% time = time/60; % convert to minutes
% sig = xlsread(strcat(pname,fname), 'B:B');
% L=size(sig,1); % length of signal
% 
% % write in Optoco's sampling frequency
% Fs = 1000;
% 
% % display raw signal
% figure;
% hold on;
% plot(time,sig);xlabel('Time (min)');ylabel('UC (arb. units)'); title('Optoco Raw Signal');

%% Filter - Band pass

fftSig=(fft(sig)); % fourier of signal

% Method 1 - online
f = Fs * (0:(L/2))/L; % frequency domain captured in fft 
P2 = abs(fftSig/L); % amplitude spectrum (???)
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);

figure;
plot(f,P1);
title('Fourier of Signal')


% Method 2 - pat and aniket
f2 = Fs*(0:L-1)/(2*L); % frequency domain; same length as signal
figure;
plot(f2,abs(fftSig));
title('Fourier of Signal - Same Length')

fltr=fftSig.*0; % create a masking vector of 0s

lowF=0.0005; % low cutoff frequency (Hz);
highF = 0.02; % high cutoff frequency (Hz); 

% Pat tried finding the index of the cutoff frequencies in the f2 vector
% index_lo = find(f2 == lowF);
% index_hi = find(f2 == highF);
% fltr(index_lo:index_hi) = 1;

% Aniket mapped frequency to sample index using f = Fs * (0:(L/2))/L or f = Fs*(0:L-1)/(2*L);
array_cut_low=floor(1+(lowF/(Fs))*2*L);
array_cut_high=ceil(1+(highF/(Fs))*2*L);
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


%% Smoothing via Moving average (let's try it)


%% Calculate slope of filtered signal

% initialize vector
slopes=zeros(size(sig));
slope_window= Fs*(60 * 1); % this is an X minute window (change last number)

for i=(slope_window+1):(L)
    
    slopes(i,2)=1*(reconstrOut(i,1)-reconstrOut(i-slope_window,1))/(time(i)-time(i-slope_window));
    % the first # is a multiplier used to bring the slope graph to a similar
    % magnitude as the signals
end


%% Define threshold

% initialize vectors
thres=zeros(1,L);
window=10; % time window in minutes
ratio = 0.67;

% time=0:1/Fs:(length(tm)-1)/Fs *** REMINDER! ea. sample is spaced as such

% calculate threshold via moving average 
for i=1:L 
    if i<=window*60*Fs
        slopeMax=max(slopes(1:i,2));
        slopeMin=min(slopes(1:i,2));
        range=slopeMax-slopeMin;
        thres(i)=slopeMin+ratio*range;
    else
        slopeMax=max(slopes((i-window*60*Fs):i,2));
        slopeMin=min(slopes((i-window*60*Fs):i,2));
        range=slopeMax-slopeMin;
        thres(i)=slopeMin+ratio*range;
    end
end
    
% calculate threshold via range percentage
slopeMax=max(slopes);
slopeMin=min(slopes);
range=slopeMax(2)-slopeMin(2);
thres_orig=(slopeMin(2)+ratio*range)*ones(1,length(thres));

%% Plot signal (original+filt), slopes, and threshold (2 types)

figure;
hold on;
plot(time(1:L),sig,'r');xlabel('Time (min)');ylabel('UC (arb. units)');
plot(time(1:L),reconstrOut,'k');
plot(time(1:L),slopes(1:L,2),'b');xlabel('Time (min)');ylabel('Signal Slope');
plot(time(1:L),thres,'m');
plot(time(1:L),thres_orig,'g');
%hold off;
legend('Raw','Filt. Sig','Filt. Slopes','Thres Moving','Thres Orig');
title('Signal with Overlaid Thresholds');

%% Peak Counting
count = []; % this is a #contractions x 2 matrix with the index of the slopes vector on the up and down crossing of the threshold
ind = 1; % this index keeps track of the row in the count matrix
k = 1; % this k allows us to step through each point in slopes vector
t = 1; % this is the acceptabe time spacing between peaks in minutes 


while k < length(slopes) % perform the following on all pts in slopes vector
    
    % if you are above the threshold...
    if slopes(k,2) > thres_orig(1)
        
        % determine when you cross the threshold on the way down -- MAY NOT
        % BE NECESSARY!
        add = 1; % define an "add" term that will keep track of index
        while slopes(k+add,2) > thres_orig(1) %keep adding to index until you're under thresh
            add = add + 1;
        end
        
        % determine start TIME
        chg = 1 % index changer 
        while slopes(k-chg,2) > 0
            chg = chg + 1;
        end
        
        stime = (k - chg)/Fs/60 % start point of UC in minutes
        
        % determine end TIME
        chg = 1;
        while slopes(k+chg,2) > 0
            chg = chg + 1;
        end
        etime = (k + chg)/Fs/60 % end point of UC in minutes
        
        
        % once you're below threshold, check to make sure peak isn't too
        % close to previous one...
        if ind > 1
            time_dif = (k-count(ind-1,2))/Fs/60; 
            % index of detected peak - index of end of previous counted peak
            % converted to seconds (/Fs) then minutes (/60)
        else
            time_dif = t+1; % if this is the first detected peak, allow it to count! 
        end
        
        if time_dif > t % only if the peak is further than t min apart
            count(ind,1) = k; % save the 1st crossover point
            count(ind,2) = k + add; % save that 2nd crossover point
            count(ind,3) = stime;
            count(ind,4) = etime;
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

%% Determine duration of each contraction

%% Produce Figures (if needed)
% Use the following to produce figures for presentations/papers
% fig=figure;
% hold on;
% a=plot(time(1:L),tm(1:L,2),'r');
% b=plot(time(1:L),reconstrOut,'b');
% c=plot(time(1:L),slopes(1:L,2),'LineWidth',2);
% c.Color=[39 139 34]./255;
% d=plot(time(1:L),thres_orig,'LineWidth',3.5);
% d.Color=[0 0 0]./255;
% 
% 
% % plot the found points
% y_contr = ones(1,tot_contr).*thres_orig(1);
% e=plot(time(count(:,1)),y_contr,'x','LineWidth',18);
% e.Color=[255 140 0]./255;
% 
% legend('Raw','Filt. Sig','Filt. Sig Slopes','Threshold',...
%     'Detected Contractions','Location','southeast');
% % set(gca,'FontSize',30)
% xlabel('Time (min)','FontSize',30);
% ylabel('UC (arb. units)','FontSize',30);
% title('Uterine Contraction Original and Procesed Signals with Overlaid Thresholds','FontSize',25);
% axis([0 25 -70 110]);
% hold off;


%% Output to Excel
savepath = '/Users/Patricia/Documents/Rice/7th Semester/Senior Design/';

%xlswrite(strcat(savepath,'UC_Data_',entry),[time(1:L)' reconstrOut tm(1:L,2)]);
%xlswrite(strcat(savepath,'f1_',entry),f');
%xlswrite(strcat(savepath,'f2_',entry),f2');