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
%   InH documentation
%   InH 3GPP Indoor hotspot
%   InH(d,fc) computes the distance dependent Pathloss
%   parameter d can be a vector in meters
%   parameter fc is a scalar in GHz
%   retruns a vector PL in dB

function PL=InH(d,fc)

d(d<3)=3.01; 
Pr_LOS=1*(d<=18)+exp(-(d-18)/27).*(d>18).*(d<37)+0.5*(d>=37);
LOS = binornd(1,Pr_LOS);
if (d>100)
   LOS(d>100)=1; 
end
PL = LOS.*(16.9*log10(d) + 32.8 + 20*log10(fc)).*(d>3)+...
    (1-LOS).*(43.3*log10(d) + 11.5 + 20*log10(fc)).*(d>10);
Shad=LOS.*(d>3).*3^0.5.*randn(size(d))+...
    (1-LOS).*(d>10).*4^0.5.*randn(size(d));
% PL is positive so it is multiplied with -1
PL = -PL+Shad;
end
