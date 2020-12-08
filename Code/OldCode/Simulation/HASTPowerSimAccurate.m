%% HAST Power Simulation v3
% Used for specific intervals to calculate power consumption of HAST system
clc
clear all
close all

AUCurrent = 50e-3; % Amps
RSCurrentOn = 20e-3; % Amps
RSCurrentOff = 20e-6;
RRCurrent = 14e-3; % Amps
RockBlockBoot = 100e-3; % Amps
RockBlockSend = 400e-3; % Amps
BootTime = 4; % Mintutes
SendTime = 1; % Minutes

MeasureTime = 5; % minutes
SendPeriod = 7 * 24 * 60; % one week in minutes

TotalTime = 1; % year


cCycle = (AUCurrent + RSCurrentOn + RRCurrent); % Amp-Minutes
%5sCycle = RockBlockBoot + RockBlockSend; % Amp-Minutes

sampleVec = 0:0.25:(TotalTime * 365 * 24 * 60); % samples are in 1/4 mintues
PwrVec = 0:0.25:(TotalTime * 365 * 24 * 60); % Pwr Consumed per sample

cFlg = false;
sFlg = false;
bFlg = false;

cCount = 0;
sCount = 0;
bCount = 0;

for n = 1:length(sampleVec)
    
    if mod(sampleVec(n),60) == 0
        cFlg = true;
        if mod(sampleVec(n),SendPeriod) == 0 && n ~= 1
            sFlg = true;
            bFlg = true;
        end
    end
    
    if cFlg == true && sFlg == false
        PwrVec(n) = cCycle;
        if rem(sampleVec(n),1) == 0
            cCount = cCount + 1;
        end
    end
    
    if cFlg == true && sFlg == true
        if bFlg == true
            PwrVec(n) = cCycle + RockBlockBoot;
            if rem(sampleVec(n),1)
                bCount = bCount + 1;
            end
        end
        if bFlg == false
            PwrVec(n) = cCycle + RockBlockSend;
        end
        
        if rem(sampleVec(n),1) == 0
            sCount = sCount + 1;
        end
    end
    
    if sCount == 5
        sFlg = false;
        sCount = 0;
    end
    
    if cCount == 5
        cFlg = false;
        cCount = 0;
    end
    
    if bCount == BootTime
        bFlg = false;
        bCount = 0;
    end
    
    if cFlg == false && sFlg == false
        PwrVec(n) = RSCurrentOff;
    end
end
figure(1)
plot(sampleVec,PwrVec)

numCell = 6;
batCap = numCell * 19 * 60
batVec = 1:0.25:max(PwrVec);
batVec(1) = batCap;
for u = 1:length(PwrVec)
    if u > 1
        batVec(u) = batVec(u-1) - PwrVec(u);
    end
end

plot(sampleVec,batVec)

