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

% Hata urban macro model
function PL = UMa_hata(d) % d in meters

fc = fc*1e3; %in MHz
hb=15;hm=1.5;
A = 69.55+26.16*log10(fc)-13.82*log10(hb);
B = 44.9-6.55*log10(hb);

E = 3.2*(log10(11.7554*hm))^2-4.97;
%E = (1.1*log10(fc)-0.7)*hm-(1.56*log10(fc)-0.8)

PL = A+B*log10(d*10^-3)-E;

PL=-PL;
end

