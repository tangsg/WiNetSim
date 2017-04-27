%% WiNetSim
%     Wireless Network Simulator \n
%     Copyright (C) 2016  IIT Hyderabad  \n
% 
%     This program is free software: you can redistribute it and/or modify
%     it under the terms of the GNU General Public License as published by
%     the Free Software Foundation, either version 3 of the License, or any later version.
% 
%     This program is distributed in the hope that it will be useful, but
%     WITHOUT ANY WARRANTY; without even the implied warranty of
%     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
%     General Public License for more details.
% 
%     You should have received a copy of the GNU General Public License
%     along with this program.  If not, see <http://www.gnu.org/licenses/>.
%     
%     
%   Contact Information
% 	WiNetSim Admin : sreekanth@iith.ac.in 
%   Address        : comm-lab, IIT Hyderabad

%% Parameters

%% constants
Tem = 290;  % temperature
k = 1.3806488*10^-23; %boltzman constant
NF=10^.7; % Noise figure
B_wifi = 20e6/64;  % sub carier Bandwidth
Noise = k*Tem*B_wifi*NF; % thermal noise floor in Watts
parameter.Noise=Noise;

%% DCF Parameters
wifi_slot=9; % WiFi Slot duration in micro seconds
parameter.wifi_slot=wifi_slot;

DIFS_time=34; % DIFS duration
parameter.DIFS_time=DIFS_time;

CW_min=16; % Minimum Contention Window
parameter.CW_min=CW_min;

CW_max=1024; % Maximum Contention Window
parameter.CW_max=CW_max;

MAX_TX_OP_us = 4e3;
parameter.MAX_TXOP=MAX_TX_OP_us;


% Preamble_th_dbm = -82;
% parameter.WIFI_TH = 10^(0.1*(Preamble_th_dbm-30-10*log10(56)));


%System Specific
% NCell = 1;
% n_AP =1; % no of access points per cell
% n_STA=3; %Ap included
BW = 20; %bandwidth

fc=5; %Carier Frequcy 5 GHz
parameter.fc=fc;
Tx_power_dbm = 18;
parameter.Pt_AP=10^(0.1*(Tx_power_dbm-30-10*log10(56))); %Tx_power_dbm;
% channel model
parameter.channel_model = 'InH';
%do it for both symbol durations
% 20MHz BW
% guard = 800ns
% symbol duration 4us
% 52 subcarriers for data 4 pilots 8 nulls
% wifi rates per MCS
% MCS::modulation::code_rate::bits_in_20us::data rate
%  0    BPSK         1/2           130         6.5 Mbps
%  1    QPSK         1/2           260          13 Mbps
%  2    QPSk         3/4           390         19.5Mbps
%  3    16QAM        1/2           520          26 Mbps
%  4    16QAM        3/4           780          39 Mbps
%  5    64AQM        2/3          1040          52 Mbps
%  6    64QAM        3/4          1170         58.5Mbps
%  7    64QAM        5/6          1300          65 Mbps
%  8    256QAM       3/4          1560          78 Mbps 
%
if(BW==20)
    WIFI_data_20us = round([130,260,390,520,780,1040,1170,1300,1560]);
elseif(BW==40)
    % Add for 40MHz
    %WIFI_data_20us = round([160,320,480,640,960,1280,1440,1600]*.9);
end
parameter.WIFI_data_20us= WIFI_data_20us;


% deployment scenario : Room
% length_room = 300;
% breadth_room = 300;
height_AP = 0;% height of AP
height_STA = 0;% height of STA
parameter.height_AP=height_AP;
parameter.height_STA=height_STA;

%Traffic
% file_size = 1000000;
% parameter.file_size=file_size;
% 
% DL_UL =1; % DL to UL Ratio
DL_percent = 1/(1+DL_UL);
UL_percent = 1/(1+(1/DL_UL));

% Network_load_Mbps = 20*1e6;
% load per user in files per micro second
load_per_user_dl = DL_percent*(Network_load_Mbps/(file_size*(NCell*n_AP*n_STA)))*1e-6;
load_per_user_ul = UL_percent*(Network_load_Mbps/(file_size*(NCell*n_AP*n_STA)))*1e-6;
parameter.load_per_user_dl=load_per_user_dl;
parameter.load_per_user_ul=load_per_user_ul;
%create a figure
if(1)
    figure;
    graph=gca;
end