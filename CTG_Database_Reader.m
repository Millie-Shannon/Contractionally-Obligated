clear all; close all; clc;

% Modify location for where you unzipped the wfdp toolkit files, the path
% needs to be set to the 'mcode' folder
% addpath('D:\wfdb-app-toolbox-0-10-0\mcode')
addpath('C:\Users\atolp\Documents\MATLAB\mcode');
% Read the signal from the website using the rdsamp function. 
all_thresholds=[];
entries=[1001:1506 2001:2046];
for i=1:length(entries)
    str=strcat('ctu-uhb-ctgdb/',num2str(entries(i)),'.dat');
[tm,sig]=rdsamp(str); % The rdsamp function has the website 'https://physionet.org/physiobank/database/' embedded within it.  You have to provide the rest of the website information for the database and file you want.
time=[0:1/4:(length(tm)-1)/4]; % According to the website, the data was sampled at 4 Hz

% % Plot the traces.  The signal contains both FHR and UC traces.  The first is FHR, the second is UC
% figure;plot(time,tm(:,1));xlabel('Time (s)');ylabel('FHR (bpm)')
% figure;plot(time,tm(:,2));xlabel('Time (s)');ylabel('UC (arb. units)') % Since the amplitude of the toco is meaningless, I put the y-axis as 'arbitrary units'.
% 
% % Plot a shorter section of the UC trace to allow for better visibility
% figure;plot(time(1:3000),tm(1:3000,2));xlabel('Time (s)');ylabel('UC (arb. units)') 

L=size(tm,1);
sig=tm(1:L,2);

fftSig=(fft(sig));
fltr=fftSig.*0;
lowBnd=1;
highBnd=85;
fltr(lowBnd:highBnd)=1;

%figure;
%plot(abs(fftSig));
% title('FFT of input signal');
% xlabel('Frequency');

fltSig=fftSig.*fltr;
%figure;
%plot(abs(fltSig));
% title('Frequency domain filtered input signal');
% xlabel('Frequency');


reconstr=ifft((fltSig));
phase_rec=angle(reconstr);
amplitude_rec=abs(reconstr);
reconstrOut=2*amplitude_rec.*cos(phase_rec);
x=1:length(sig);

%figure;
%plot(x,sig,'r',x,reconstrOut,'b');
% title('FFT filtered, original signals');
% xlabel('Time');
% legend('Original','Filtered');


% figure;
% hold on;
slopes=zeros(size(tm));
thres=zeros(1,L);
window=10;
for i=1:(L-1)
slopes(i,2)=50*(reconstrOut(i+1,1)-reconstrOut(i,1))/(time(i+1)-time(i));
end

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
disp(thres_orig(1));
all_thresholds=[all_thresholds thres_orig(1)];
end