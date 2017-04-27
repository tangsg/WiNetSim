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
%   find_prob_coll finds whether the packet is in collision given
%   SINR_DL in dB and mcs_idx and string tech
function collision = find_prob_coll(SINR_DL,mcs_idx,tech)
switch tech
    case '802.11ac'
        BLER_LUT=load('AWGN_BLER_WIFI');
    case 'LTE'
        BLER_LUT=load('AWGN_BLER_LTE');
    otherwise
        disp('tech unknown \n');
end
SNR_vector  = BLER_LUT.SNR{mcs_idx};
BLER_vector = BLER_LUT.BLER{mcs_idx};

if (SINR_DL ~= -inf)
    current_SINR_dB=SINR_DL;
    current_SNR = current_SINR_dB;
    %disp(current_SNR )
    minimum_SNR = SNR_vector(1);
    maximum_SNR = SNR_vector(end);
    current_SNR(current_SNR < minimum_SNR) = minimum_SNR;
    current_SNR(current_SNR > maximum_SNR) = maximum_SNR;
    predicted_BLER = pchip(SNR_vector,BLER_vector,current_SNR);
    %disp(predicted_BLER)
    random_num=rand(1,1);
    if predicted_BLER<random_num
        %Successful_transmisson=1;
        collision = 0;
    else
        %Successful_transmisson=0;
        collision = 1;
    end
else
    collision = 1;
end
end

