%% HAST Power Simulation Daily
clc
clear all
close all
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Power Block
RSPowerOn = 20e-3; % According to documentation 
RSPowerOff = 20e-6; % According to documentation

AUPowerOn = 50e-3;
AUPowerOff = 0;

RRPowerOn = 14e-3;
RRPowerOff = 0;

RBPowerIdle = 20e-6;
RBPowerStartup = 450e-3;
RBPowerWait = 100e-3;
RBPowerOff = 0;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Time Block (1 day)
StartupHour = 12;
EndHour = 12 + (5/60);

StartupHourMin = StartupHour * 60; % 12:00 Noon in terms of minutes, with the first hour starting at 0
EndHourMin = EndHour * 60; % 12:05, System is expected to take 5 minutes to take a scan and send

StartupHourSec = StartupHourMin * 60;
EndHourSec = EndHourMin * 60;

NumDays = 1;

NumHour = 24 * NumDays;
NumMin = 60 * NumHour;
NumSec = 60 * NumMin;


MinVec = 1:1:NumMin;
SecVec = 1:1:NumSec;

PwrVecMin = zeros(NumMin,1);
PwrVecSec = zeros(NumSec,1);

RSOnTime = 5;
RSOffTime = NumMin - RSOnTime;

AUOnTime = 5;

RROnTime = 5;

RBStartupTime = 3;
RBWaitTime = 2;
RBIdleTime = NumMin - RBStartupTime - RBWaitTime;

%% Power Consumption for RocketScream
RSOn = RSOnTime * RSPowerOn; % Ampere-Minutes
RSOff = RSOffTime * RSPowerOff; % Ampere-Minutes

% Power Consumption for Arduino Uno
AUOn = AUOnTime * AUPowerOn; % Ampere-Minutes

% Power Consumption for Reed Relay
RROn = RROnTime * RRPowerOn; % Ampere-Minutes

% Power Consumption for RockBlock
RBStartupPower = RBStartupTime * RBPowerStartup; % Ampere-Minutes
RBWaitPower = RBWaitTime * RBPowerWait; % Ampere-Minutes
RBIdlePower = RBPowerIdle * RBIdleTime; % Ampere-Minutes

%% Time Vector Fill (Minutes)

for n = 1:1:length(MinVec)
    if MinVec(n) < StartupHourMin
        PwrVecMin(n) = RSOff + RBIdlePower;
    end
    
    if and(MinVec(n) >= StartupHourMin,MinVec(n) < 723) %% System has woken up, but RockBlock hasn't finished starting up yet
        PwrVecMin(n) = RSOn + AUOn + RROn + RBStartupPower;
    end
    
    if(MinVec(n) > StartupHourMin & MinVec(n) >= 723 & MinVec(n) <= EndHourMin)
        PwrVecMin(n) = RSOn + AUOn + RROn + RBWaitPower;
    end
    
    if MinVec(n) > EndHourMin
        PwrVecMin(n) = RSOff + RBIdlePower;
    end
    
end

figure(1)
plot(MinVec,PwrVecMin);

%% Time Vector Fill (Seconds)

for n = 1:1:length(SecVec)
    if SecVec(n) < StartupHourSec
        PwrVecSec(n) = RSOff + RBIdlePower;
    end
    
    if and(SecVec(n) >= StartupHourSec,SecVec(n) < 43380) %% System has woken up, but RockBlock hasn't finished starting up yet
        PwrVecSec(n) = RSOn + AUOn + RROn + RBStartupPower;
    end
    
    if(SecVec(n) > StartupHourSec & SecVec(n) >= 43380 & SecVec(n) <= EndHourSec)
        PwrVecSec(n) = RSOn + AUOn + RROn + RBWaitPower;
        
    end
    
    if SecVec(n) > EndHourSec
        PwrVecSec(n) = RSOff + RBIdlePower;
    end
    
end
figure(2)
plot(SecVec,PwrVecSec);

        
        








