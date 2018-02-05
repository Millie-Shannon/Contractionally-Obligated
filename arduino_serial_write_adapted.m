clc
clear all
delete(instrfindall);
%%
% Save the serial port name in comPort variable.

comPort = 'COM7';
%%
% It creates a serial element calling the function "setupSerial"
if(~exist('serialFlag','var'))
    [arduino,serialFlag] = setupSerial(comPort);
end
%%
% Time to create our plot window in order to visualize data collectoed
% from serial port readings

if (~exist('h','var') || ~ishandle(h))
    h = figure(1);
    set(h,'UserData',1);
end

if (~exist('button','var'))
    button = uicontrol('Style','togglebutton','String','Stop',...
        'Position',[0 0 50 25], 'parent',h);
end

load('one_contr_data.mat');
output = [];
tic;
for i=1:841
    tic;
    fwrite(arduino,sig_in(i));
    for j=1:12
        output(i,j) = fscanf(arduino,'%f');
    end
    toc;
end
disp(output);
hold on;
plot(1:size(output,1),output(:,1),'r');
% plot(1:size(output,1),output(:,2),'b');
plot(1:size(sig_in),sig_in,'k');
legend('Moving Avg., BT Removed','Raw');
hold off;