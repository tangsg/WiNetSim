%% WiNetSim
%     Wireless Network Simulator \n
%     Copyright (C) 2016  IIT Hyderabad  \n
% 
%     This program is free software: you can redistribute it and/or modify
%     it under the terms of the GNU General Public License as published by
%     the Free Software Foundation, either version 3 of the License, or (at
%     your option) any later version.
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

%% test RMA_3gpp
clear all
% distnaces
d = 5:500:130000;
% carrier frequecny in GHz
fc = 0.7;
% BS height in m
h_BS = 30;
% UE height in m
h_UT = 10;
% building height in m
h=5;
% street Width in m
W=20;
% transmit power in dBm
P_tx = 30;
% BW in Hz
BW = 1e6; %Hz
% Reciver Noise figure in dB
NF = 7;
% Thermal noise in dBm
Noise = -174+10*log10(BW)+NF;
% Receive Antenna Gain 
G_rx = 12;
% Path loss in dB
PL_3gpp = RMa_3gpp(d,fc,h_BS,h_UT,h,W);
PL_hata = RMa_hata(d,fc,h_BS,h_UT);
% Received power in dBm
RX_3gpp = G_rx+P_tx-PL_3gpp;
RX_hata = G_rx+P_tx-PL_hata;
% SNR in dB
SNR_3gpp = RX_3gpp-Noise;
SNR_hata = RX_hata-Noise;
% plot
plot(d,SNR_3gpp,d,SNR_hata)
legend ('3gpp','hata')
xlabel('distance in m');
ylabel('SNR in dB');