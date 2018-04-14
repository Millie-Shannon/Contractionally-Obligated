clc
clear all
delete(instrfindall);
%%
% Save the serial port name in comPort variable.

comPort = 'COM15';
%%
% It creates a serial element calling the function "setupSerial"
if(~exist('serialFlag','var'))
    [arduino,serialFlag] = setupSerial(comPort);
end
%%
% Time to create our plot window in order to visualize data collectoed
% from serial port readings

if (~exist('h','var') || ~ishandle(h))
    h = figure;
    set(h,'UserData',1);
end

if (~exist('button','var'))
    button = uicontrol('Style','togglebutton','String','Stop',...
        'Position',[0 0 50 25], 'parent',h);
end

% %% Get signal from database
% 
% entry = '1018';
% % Read the signal from the website using the rdsamp function.
% tic;
% fprintf('Time to retrieve signal:\n');
% addpath('C:\Users\atolp\Documents\MATLAB\mcode'); % Aniket's path
% [tm,Fs]=rdsamp(strcat('ctu-uhb-ctgdb/',entry,'.dat')); % The rdsamp
% %function has the website 'https://physionet.org/physiobank/database/'
% %embedded within it.  You have to provide the rest of the website
% %information for the database and file you want.
% toc;
% tic;
% fprintf('Time to process signal and count contractions:\n');
% time=0:1/Fs:(length(tm)-1)/Fs; % According to the website, the data was
% % sampled at 4 Hz
% time = time./60; % convert from seconds to minutes
% 
% L=size(tm,1); % length of signal
% sig=tm(1:L,2); % set sig as only the 2nd column, the UC data
% 
% sig_in = sig;

%% feed signal to arduino
sig_in = zeros(1,19200);
output = [];
count = 1;
tic;
final_ind = length(sig_in);
serial_output_rate = 4;
contractions = [];
last = [];
num_contr = 1;
num_plots = 1;

use_waitbar = 0;
if use_waitbar == 1
    h_bar = waitbar(0,strcat('Completion of Arduino real-time calculations: 0/',...
    num2str(final_ind)));
end

tic;
for i=1:final_ind
    tic;
    fwrite(arduino,sig_in(i));
    for j=1:7
        temp = fscanf(arduino,'%f');
        output(count,j) = temp;
    end
    disp(output(count,:));
    count=count+1;
    if i==1
        last = output(i,:);
    else
        curr = output(i,:);
        if (curr(4)~=last(4)&&curr(5)~=last(5)&&curr(4)~=0&&curr(5)~=0)
            contractions(num_contr,:) = [curr(4) curr(5)];
            num_contr = num_contr + 1;
        end
        last = curr;
    end
    if use_waitbar == 1
        waitbar(i/final_ind,h_bar,strcat('Completion of Arduino real-time calculations: ',...
        num2str(i),'/',num2str(final_ind)));
    else
    if mod(i,60) == 0
        limits = [0 10];
        graph_limits = [0 i/240];
        if i > 60*4*10
            limits = [(i-60*4*10)/240 i/240];
            graph_limits = [(i-60*4*10)/240 i/240];
        end
        figure(h);
        set(gcf, 'Units', 'Normalized', 'OuterPosition', [0, 0.04, 1, 0.96]);
        hold on;
        temp = output(:,1) - output(:,3);
        a=plot((240*graph_limits(1)+1:240*graph_limits(2))./240,temp(240*graph_limits(1)+1:240*graph_limits(2)));
        b=plot((240*graph_limits(1)+1:240*graph_limits(2))./240,output(240*graph_limits(1)+1:240*graph_limits(2),2),'LineWidth',4);
        a.Color=[255 198 47]./255;
        b.Color=[79 38 131]./255;
        if size(contractions,1)>0
            c = plot(contractions(:,1)./240,zeros(size(contractions(:,1))),'x','LineWidth',20);
            d = plot(contractions(:,2)./240,zeros(size(contractions(:,2))),'x','LineWidth',20);
            c.Color = [0 79 48]./255;
            d.Color = [197 18 48]./255;
            legend('Raw','Filtered','Start','Stop');
        else
            legend('Raw','Filtered');
        end
        axis([limits(1) limits(2) -50 150]);
        xlabel('Time (minutes)');
        ylabel('UC Signal (arbitrary units)');
        dim = [.2 .5 .3 .3];
        str = {strcat('# Contractions in last 10 minutes: ',num2str(output(count-1,7))),strcat('Avg. contraction duration (min): ',num2str(output(count-1,6)))};
        if num_plots > 1
            delete(findall(gcf,'type','annotation'));
        end
        annotation('textbox',dim,'String',str,'FitBoxToText','on');
        hold off;
        num_plots = num_plots + 1;
    end
    end
    toc;
end
toc;
if use_waitbar == 1
close(h_bar);
end
%disp(output);

%% plot outputs
contractions = [];
last = output(1,:);
num_contr = 1;
for i = 2:size(output,1)
    curr = output(i,:);
    if (curr(4)~=last(4)&&curr(5)~=last(5)&&curr(4)~=0&&curr(5)~=0)
        contractions(num_contr,:) = [curr(4) curr(5)];
        num_contr = num_contr + 1;
    end
end

fig = figure;
hold on;
a = plot((1:size(output,1))./240,output(:,2));
b = plot((1:size(sig_in))./240,sig_in);
a.Color=[79 38 131]./255;
b.Color=[255 198 47]./255;
if size(contractions,1)>0
    c = plot(contractions(:,1)./240,zeros(size(contractions(:,1))),'x','LineWidth',10);
    d = plot(contractions(:,2)./240,zeros(size(contractions(:,2))),'x','LineWidth',10);
    c.Color = [0 79 48]./255;
    d.Color = [197 18 48]./255;
    legend('Moving Avg., BT Removed','Raw','Start','Stop');
else
    legend('Moving Avg., BT Removed','Raw');
end
axis([0 final_ind/(4*60) -50 150]);
xlabel('Time (min)');
ylabel('UC Signal (Arbitrary Units)');
title('Real-time Arduino contraction counting');
hold off;