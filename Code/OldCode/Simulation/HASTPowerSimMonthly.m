%% HAST Power Simulation Monthly
clc
clear all
close all
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%% Power Block
batteryCapacity = 19 * 3; % Ah
batCapTot = (batteryCapacity)*60; % The capacity of the battery in terms of Ampere-Minutes

RSPowerOn = 20e-3; % According to documentation, in Amperes 
RSPowerOff = 20e-6; % According to documentation, in Amperes

AUPowerOn = 52e-3; % Estimated Arduino Uno current in (Measured from source to Vin)
AUPowerOff = 0; % Should be 0 with relay

% RRPowerOn = 1e-3; % Power consumption to keep regulator on
RRPowerOn = 14e-3; % Power consumption to keep relay enabled
RRPowerOff = 0;

RBPowerIdle = 20e-6; % idle current for RockBLOCK
% RBPowerIdle = 0;
RBPowerStartup = 450e-3; % max current in for RockBLOCK, from documentation 
RBPowerWait = 100e-3; % wait mode current
RBPowerOff = 0;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Time Block
NumYears = 9; % Number of years for simulated operation
NumMonths = NumYears * 12; % converts down to months, days, hrs and minutes
NumDays = NumMonths * 30.5;
NumHrs = 24 * NumDays;
NumMin = 60 * NumHrs;

NumMinInDay = 24 * 60; % constant

MinVec = 1:1:NumMin; % each minute of operation
PwrVec = zeros(NumMin,1); % current in for each minute

batCap = zeros(NumMin,1); % tracks battery capacity over time
tempCapLoss = 0.00; % estimated loss of capacity, due to temperature
currentCap = 1 - tempCapLoss;
batCap(1) = batCapTot * currentCap; % beginning capacity of battery

StartupHour = 12; % arbitrary hour of operation, in hours
StartupMin = StartupHour * 60; 

SendFlgMin = StartupMin + 3; % when RockBLOCK starts sending signal

EndHour = 12 + (5/60); % when system has finished cycle
EndMin = EndHour * 60;

RSOnTime = 5; % how long RocketScream is "on"
RSOffTime = NumMinInDay - RSOnTime; % if RocketScream isn't on, its off

AUOnTime = 5; % how long Arduino Uno is "on"

RROnTime = 5; % how long the relay/regulator is "on"

RBStartupTime = 2; % simulated length of time it takes for RockBLOCK to turn on
RBWaitTime = 1; % simulated length of time RockBLOCK is in wait mode
RBIdleTime = NumMinInDay - RBStartupTime - RBWaitTime; % simulated length of time RockBLOCK is in "Idle" mode
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Power Consumption for RocketScream
RSOn = RSPowerOn; % Ampere-Minutes
RSOff = RSPowerOff; % Ampere-Minutes

% Power Consumption for Arduino Uno
AUOn = AUPowerOn; % Ampere-Minutes

% Power Consumption for Reed Relay
RROn = RRPowerOn; % Ampere-Minutes

% Power Consumption for RockBlock
RBStartupPower = RBPowerStartup; % Ampere-Minutes
RBWaitPower = RBPowerWait; % Ampere-Minutes
RBIdlePower = RBPowerIdle; % Ampere-Minutes

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Time Vector Fill, Minutes for 1 Month
dayCount = 0; % keeps track of how many days have passed
numMinInStandby = 0; % keeps track of minutes in standby mode and on mode
numMinInOnMode = 0;
for n = 1:1:length(MinVec)
    
    StartupMinMod = StartupMin + (dayCount * NumMinInDay); % modifies start time based on how many days have passed
    EndMinMod = EndMin + (dayCount * NumMinInDay); % modifies end time based on how many days have passed
    SendFlgMinMod = SendFlgMin + (dayCount * NumMinInDay); % modifies send time based on how many days have passed
    
    if MinVec(n) < StartupMinMod % before system is in operating mode
        PwrVec(n) = RSOff + RBIdlePower;
        numMinInStandby = numMinInStandby + 1;
    end
    
    if and(MinVec(n) >= StartupMinMod,MinVec(n) < SendFlgMinMod) %% System has woken up, but RockBlock hasn't finished starting up yet
        PwrVec(n) = RSOn + AUOn + RROn + RBStartupPower;
        numMinInOnMode = numMinInOnMode + 1;
    end
    
    if(MinVec(n) > StartupMinMod & MinVec(n) >= SendFlgMinMod & MinVec(n) <= EndMinMod) % RockBLOCK has finished starting up
        PwrVec(n) = RSOn + AUOn + RROn + RBWaitPower;
        numMinInOnMode = numMinInOnMode + 1;
    end
    
    if MinVec(n) > EndMinMod % Between operation
        PwrVec(n) = RSOff + RBIdlePower;
        numMinInStandby = numMinInStandby + 1;
    end
    
    if n > 1 % calculates new battery capacity 
        batCap(n) = batCap(n - 1) - PwrVec(n);
    end
    
    if mod(MinVec(n),NumMinInDay) == 0 % increments how many days have passed
        dayCount = dayCount + 1;
        %PwrConsumed(dayCount) = sum(PwrVec);
        %disp(PwrConsumed(dayCount))
    end
end
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Plotting Block
figure(1)
subplot(2,1,1);
dailyRange = 715:1:730;
plot(MinVec(dailyRange),PwrVec(dailyRange))
title("Daily Current Usage (Amps vs. Minutes of Operation)")
xlabel("Time(min)")
ylabel("Current(A)")
ylim([-0.05,0.55])
grid on
xticks(715:1:730)
yticks(0:0.05:0.55)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
subplot(2,1,2)
yearlyRange = 1:1:8760*2;
plot(MinVec(yearlyRange),batCap(yearlyRange),'r')
title("Capacity of Battery vs. Time over a Year")
xlabel("Time(min)")
ylabel("Capacity(Am)")
ylim([min(batCap(MinVec(yearlyRange)) * currentCap),max(batCap(MinVec(yearlyRange)) * currentCap)])
grid on
%intYtick = min(batCap(MinVec(yearlyRange))*currentCap):2.5:max(batCap(MinVec(yearlyRange))*currentCap);
%yticks(intYtick)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Finds crossing point of capacity with x-axis to see how long the battery
% will last
deadFlg = false;
for n = 1:length(MinVec)
    if batCap(n) < 0 & ~deadFlg
        YearVec = (((MinVec(n) / 60) / 24) / 365);
        msg = sprintf("Estimated life time is %f years",YearVec);
        disp(msg);
        deadFlg = true;
    end
end