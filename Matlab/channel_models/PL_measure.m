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
%   PL_measure documentation
%   PL_measure(d,fc,chann_type) computes the distance dependent Pathloss
%   parameter d can be a vector in meters
%   parameter fc is a scalar in GHz
%   parameter chann_type is the type of the channel model
%   retruns a vector PL in dB
function PL = PL_measure(d,fc,chann_type )

switch chann_type
    
    case 'InH'
        PL = InH(d,fc);
    case 'RMa_hata'
        PL = RMa_hata(d,fc);
    case 'UMa'
        PL = UMa(d,fc);
    case 'UMa_hata'
        PL = UMa_hata(d,fc);
    otherwise
        disp('unknown channel \n');
end
end

