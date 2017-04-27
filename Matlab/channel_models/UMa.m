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

% 3gpp urban macro model
function PL=UMa(fc,d) %fc in GHz and d(distance) is in meters

P=min(18./d,1).*(1-exp(-d/63))+exp(-d/63);%Probability of LOS
LOS=binornd(1,P);

h_BS=25;h_UE=1.5;%Heights of both antennas

h_bar_BS=h_BS-1;h_bar_UE=h_UE-1;

C=3.0*1e8;

h=20;%Avg height of building in m
W=20;%Street width

d_bar_BP=4*h_bar_BS*h_bar_UE*fc/C;%Breakpoint distance

PL_LOS=(10<d<d_bar_BP).*(22.*log10(d)+28+20*log10(fc))+...
    (d_bar_BP<d<5000).*(40.*log10(d)+7.8-18*log10(h_bar_BS)-18*log10(h_bar_UE)+2*log10(fc));%Pathloss for LOS

PL_LOS_shadow=-PL_LOS+(10<d<d_bar_BP).*sqrt(4).*randn(1,length(d));% Loss including shadowing effect

PL_NLOS=(10<d<5000).*(161.04-7.1*log10(W)+7.5*log10(h)+...
    -(24.37-3.7*(h/h_BS)^2)*log10(h_BS)+(43.42-3.1*log10(h_BS)).*(log10(d)-3)+...
    20*log10(fc)-(3.2*(log10(11.75*h_UE)))^2-4.97);% Pathloss for NLOS

PL_NLOS_shadow=-PL_NLOS+(10<d<5000).*sqrt(6).*randn(1,length(d));% Loss including shadowing effect

PL=LOS.*PL_LOS_shadow+(1-LOS).*PL_NLOS_shadow;% Total Path loss
end
