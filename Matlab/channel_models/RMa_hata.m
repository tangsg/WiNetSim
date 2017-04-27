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
% Hata rural macro model
% d in m
% fc in GHz
% h_BS in m
% h_UT in m
function PL = RMa_hata(d,fc,h_BS,h_UT)

fc = fc*1e3; %converting to MHz
%small cities
% a_hm = (1.1*log10(fc)-0.7)*h_UT-(1.56*log10(fc)-0.8);
% metro politan cities
a_hm = 3.2*(log10(11.75*h_UT)).^2 - 4.97;
A = 69.55+26.16*log10(fc)-13.82*log10(h_BS)-a_hm;
B = 44.9-6.55*log10(h_BS);

C = 4.78*(log10(fc))^2-18.33*log10(fc)+40.98;
PL = A+B*log10(d*10^-3)-C;

end

