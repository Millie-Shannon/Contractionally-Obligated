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

% clear all; close all; clc;

% Modify location for where you unzipped the wfdp toolkit files, the path
% needs to be set to the 'mcode' folder
% addpath('D:\wfdb-app-toolbox-0-10-0\mcode') % Dr. Carns' path
addpath('C:\Users\atolp\Documents\MATLAB\mcode'); % Aniket's path
% addpath('/Users/Patricia/Documents/MATLAB/mcode'); % Patricia's path

%% Parameters (for easy adjustment)
entry = '1351'; %UC trace from database
% entry = '1018';
% entry = '1001';
% entry = '1488';
% entry = '1136';
% entry = '2031';
Fs = 4; %sampling frequency (4 Hz for traces on database)
moving_avg_window = 10; %number of samples to be averaged in finding moving
% avg of UC trace signal
BT_calculation_window = 5; %length (min) of window used for initial
%calculation of basal tone throughout entire trace
thres = 10; %detection level threshold for identification of potential UCs
%(if signal crosses thres, its start and end points are then found and it
%is fed through remainder of the algorithm)
min_contr_length = 25; %25 sec is min contraction duration
max_contr_length = 120; %120 sec max contraction; adjust BT if above
max_iter = 15; %max number of times BT will be adjusted in while loop


%% Get signal from database

% Read the signal from the website using the rdsamp function.
tic;
fprintf('Time to retrieve signal:\n');
[tm,Fs]=rdsamp(strcat('ctu-uhb-ctgdb/',entry,'.dat')); % The rdsamp 
%function has the website 'https://physionet.org/physiobank/database/' 
%embedded within it.  You have to provide the rest of the website 
%information for the database and file you want.
toc;
tic;
fprintf('Time to process signal and count contractions:\n');
time=0:1/Fs:(length(tm)-1)/Fs; % According to the website, the data was 
% sampled at 4 Hz
time = time./60; % convert from seconds to minutes

% Plot the traces.  The signal contains both FHR and UC traces.  The first 
%is FHR, the second is UC
%figure;plot(time,tm(:,2));xlabel('Time (min)');...
%ylabel('UC (arb. units)'); title('Raw Signal'); 
% Since the amplitude of the toco is meaningless, I put the y-axis as
%'arbitrary units'.

% Plot a shorter section of the UC trace to allow for better visibility
%figure;plot(time(1:3000),tm(1:3000,2));xlabel('Time (min)');ylabel('UC (arb. units)'); title('Raw Signal (small)');

L=size(tm,1); % length of signal
sig=tm(1:L,2); % set sig as only the 2nd column, the UC data

%% Smooth Signal with Moving Average

sig_moving_avg = zeros(size(sig));
for i=1:length(sig_moving_avg)
    if i<moving_avg_window
        sig_moving_avg(i)=mean(sig(1:i));
    else
        sig_moving_avg(i)=mean(sig((i-moving_avg_window+1):i));
    end
end

%% Remove "DC Offset" / Basal Tone
sig_moving_avg_no_offset = sig_moving_avg;
threshold = zeros(size(sig_moving_avg));
basal_tone = zeros(size(threshold));
for i = 1:size(sig_moving_avg,1)
    temp=[];
    if i<(BT_calculation_window*60*Fs+1)
        temp = sig_moving_avg(1:i);
    else
        temp = sig_moving_avg(i-BT_calculation_window*60*4:i);
    end
    [N,edges] = histcounts(temp,0:ceil(max(sig_moving_avg)));
    min_in_window = 1;
    while sum(N(1:min_in_window))<sum(N)/10
        min_in_window = min_in_window + 1;
    end
    threshold(i) = min_in_window + 10;
    basal_tone(i) = min_in_window;
    %     min_in_window = min(temp);
    sig_moving_avg_no_offset(i) = sig_moving_avg(i)-min_in_window;
end
bt_track = [];
%% Peak Counting
num_changes = 1;
iter = 1;
count = [];
while num_changes~=0 && iter<=max_iter
    bt_track(iter,:) = basal_tone;
    %% Original peak counting algorithm
    count = [];
    ind = 1;
    k = 1; 
    while k < length(sig_moving_avg_no_offset)
        % if you are above the threshold...
        if sig_moving_avg_no_offset(k) > thres
            count(ind,1) = k;  % save the index of 1st crossover point
            add = 1;
            while (k+add)<=length(sig_moving_avg_no_offset)&&...
                    sig_moving_avg_no_offset(k+add) > thres 
                %keep adding to index until you're under thresh
                add = add + 1;
            end
            % once you're below threshold, check to make sure peak isn't too
            % close to previous one...
            if ind > 1
                hi = k+add;
                hi2 = count(ind-1,1);
                time_dif = (k+add-count(ind-1,1))/Fs/60;
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
    if(size(count,1)==0)
        break;
    end
    if count(size(count,1),size(count,2))==0
        %there was a glitch in the original peak counting algorithm that 
        %often caused the "end time" of the last peak to be 0; this if
        %statement just removes that last entry so there is no such glitchy
        %entry
        count=count(1:(size(count,1)-1),:);
    end
    count=unique(count,'rows');
    %remove duplicates
    
    %% find start, end indices of each contraction with respect to basal tone
    for i=1:size(count,1)
        k=count(i,1);
        while k>0&&sig_moving_avg_no_offset(k)>0
            k=k-1;
        end
        if k~=0
            count(i,1)=k;
        end
        k=count(i,2);
        while k<length(sig_moving_avg_no_offset)&&sig_moving_avg_no_offset(k)>0
            k=k+1;
        end
        if k~=length(sig_moving_avg_no_offset)
            count(i,2)=k;
        end
    end
    
    %% remove too short contractions
    counts_filtered = [];
    tmp=1;
    for i=1:size(count,1)
        if (count(i,2)-count(i,1))>=(Fs*min_contr_length)
            counts_filtered(tmp,:)=count(i,:);
            tmp=tmp+1;
        end
    end
    count = counts_filtered;
    
    %% redo "DC Offset" / Basal Tone calculations for contractions that are
    %  longer than the max length 
    num_changes = 0;
    for i=1:size(count,1)
        if (count(i,2)-count(i,1))>=(max_contr_length*Fs)
            [N,edges] = histcounts(sig_moving_avg_no_offset(count(i,1):count(i,2)),...
                0:ceil(max(sig_moving_avg_no_offset)));
            min_in_window = 1;
            while sum(N(1:min_in_window))<(iter/max_iter*sum(N))
                min_in_window = min_in_window + 1;
            end
%             if count(i,1)<=(45*Fs*60)&&count(i,2)>=(45*Fs*60)
%                 disp([min_in_window basal_tone(count(i,1)) iter/max_iter sum(N)]);
%                 if min_in_window ==41
%                     figure;
%                     histogram(sig_moving_avg_no_offset(count(i,1):count(i,2)),...
%                 0:ceil(max(sig_moving_avg_no_offset)));
%             return;
%                 end
%             end
            sig_moving_avg_no_offset(count(i,1):count(i,2)) = ...
                sig_moving_avg_no_offset(count(i,1):count(i,2)) - ...
                (min_in_window-basal_tone(count(i,1):count(i,2)));
            basal_tone(count(i,1):count(i,2)) = ...
                basal_tone(count(i,1):count(i,2)) + ...
                (min_in_window - basal_tone(count(i,1):count(i,2)));
            num_changes = num_changes + 1;
        end
    end
    iter=iter+1;
end
count=unique(count,'rows');
toc;
tot_contr = length(count);
disp('Our contraction count:');
disp(tot_contr);

count = count./(Fs*60);
%% plotting
figure;
hold on;
a = plot(time,sig_moving_avg_no_offset);
b = plot(time,sig);
e = plot(time,basal_tone);
a.Color=[79 38 131]./255;
b.Color=[255 198 47]./255;
e.Color=[12 35 65]./255;
if size(count,1)~=0
    c = plot(count(:,1),thres*zeros(size(count(:,1))),'x','LineWidth',10);
    d = plot(count(:,2),thres*zeros(size(count(:,2))),'x','LineWidth',10);
    c.Color = [0 79 48]./255;
    d.Color = [197 18 48]./255;
    legend('Raw, no BT','Raw','BT','Start','Stop');
else
    legend('Smoothed, no BT','Raw','BT');
end
xlabel('Time (minutes)');
ylabel('Contraction Signal (UC / Arbitrary Units)');
title('Uterine contraction traces and corresponding start, stop times detected');
set(gca,'FontSize',24);
set(gca,'LineWidth',8);