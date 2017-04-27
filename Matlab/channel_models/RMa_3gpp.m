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

% 3gpp rural macro model
% Refer Table B.1.2.1-1 36.814v9.0
% d_BP = 2π hBS hUT fc /c, where fc is the centre frequency in Hz, c = 3.0 10^8 m/s
% 450MHz <fc<6GHz
% 5 m < h < 50 m
% 5 m < W < 50 m
% 10 m < h BS < 150 m
% 1 m < h UT < 10 m
% h BS = 35 m, h UT = 1.5 m,
% W = 20 m, h = 5 m
% 1 for d<10
% plot(d,exp(-(d-10)/1000))
function PL = RMa_3gpp(d,fc,h_BS,h_UT,h,W)

    switch nargin
        
        case 6
            % all inputs are given
        case 5
            W = 20;
        case 4
            W=20;
            h=5;
        case 3
            W=20;
            h=5;
            h_UT = 1.5;
        case 2
            W=20;
            h=5;
            h_UT = 1.5;
            h_BS = 35;
        case 1
            W=20; %m
            h=5; %m
            h_UT = 1.5; %m
            h_BS = 35; %m
            fc = 1; % 1GHz
        otherwise
            fprintf('unknown arguments')
    end
    %speed of light
    c = 3*10^8; %m/sec
    % breakpoint distance
    d_BP = 2*pi*h_BS*h_UT*(fc*1e9)/c; %m
    
    % find the probability of LOS
    Pr_LOS=1*(d<=10)+exp(-(d-10)/1000).*(d>10);
    LOS = binornd(1,Pr_LOS);
    LOS = ones(size(d));
   PL =(LOS).*((d<d_BP).*( 20*log10(40*pi*d*fc/3)+min(0.03*h^1.72,10)*log10(d)...
           -min(0.044*h^1.72 ,14.77) + 0.002*log10(h)*d)...
           +(d>=d_BP).*(20*log10(40*pi*d_BP*fc/3)+min(0.03*h^1.72,10)*log10(d_BP)...
           -min(0.044*h^1.72 ,14.77) + 0.002*log10(h)*d_BP+40*log10(d/d_BP)))+...
           (1-LOS).*(161.04 - 7.1*log10(W) + 7.5*log10(h)-(24.37-3.7*(h/h_BS)^2)*log10(h_BS)...
            +(43.42-3.1*log10(h_BS))*(log10(d)-3)+20*log10(fc)-(3.2*(log10(11.75*h_UT))^2-4.97)); 
  Shad = 0;%LOS.*((d<d_BP).*(4^0.5.*randn(size(d)))+(d>=d_BP).*(6^0.5.*randn(size(d))))+(1-LOS).*(8^0.5.*randn(size(d)));      

  PL = PL+Shad;
end

