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


% AP_cord={[0,0],[length_room,0],...
%     [-length_room,0],...
%     [0,breadth_room],...
%     [0,-breadth_room],...
%     [length_room,breadth_room],...
%     [-length_room,breadth_room],...
%     [length_room,-breadth_room],...
%     [-length_room,-breadth_room]}; % WiFi

% maximum 9 cells
x = [-length_room/2 length_room/2 length_room/2 -length_room/2 ];
y = [-breadth_room/2 -breadth_room/2 breadth_room/2 breadth_room/2 ];
WiFi_STA=cell(9,n_AP);
WiFi_AP=cell(9,1);
for k=1:9
    % STA coordinates
    for j=1:n_AP
        i=0;
        no_coord =n_STA;
        while(i<no_coord)
            % generate coordinates
            p_x = length_room*(rand(1)-0.5);
            p_y = breadth_room*(rand(1)-0.5);
            % check if it is in the rectangle
            IN = inpolygon(p_x,p_y,x,y);
            if(IN==1)
                i=i+1;
                WiFi_STA{k,j} = [WiFi_STA{k,j}; p_x p_y];
            end
        end
    end
    % AP coordinates
    i=0;
    no_coord =n_AP;
    while(i<no_coord)
        %     p_x = length_room*(rand(1)-0.5);
        %     p_y = breadth_room*(rand(1)-0.5);
        % add APs to the centre of the room
        p_x = 2*(rand(1)-0.5);
        p_y = 2*(rand(1)-0.5);
        IN = inpolygon(p_x,p_y,x,y);
        if(IN==1)
            i=i+1;
            WiFi_AP{k} = [WiFi_AP{k}; p_x p_y];
        end
    end
    
end

% shift the coordinates to their respective rooms
for j=1:n_AP
    WiFi_STA{2,j} =[WiFi_STA{2,j}(:,1)+length_room WiFi_STA{2,j}(:,2)];
    WiFi_STA{3,j} =[WiFi_STA{3,j}(:,1)-length_room WiFi_STA{3,j}(:,2)];
    WiFi_STA{4,j} =[WiFi_STA{4,j}(:,1) WiFi_STA{4,j}(:,2)+breadth_room];
    WiFi_STA{5,j} =[WiFi_STA{5,j}(:,1) WiFi_STA{5,j}(:,2)-breadth_room];
    WiFi_STA{6,j} =[WiFi_STA{6,j}(:,1)-length_room WiFi_STA{6,j}(:,2)-breadth_room];
    WiFi_STA{7,j} =[WiFi_STA{7,j}(:,1)+length_room WiFi_STA{7,j}(:,2)-breadth_room];
    WiFi_STA{8,j} =[WiFi_STA{8,j}(:,1)-length_room WiFi_STA{8,j}(:,2)+breadth_room];
    WiFi_STA{9,j} =[WiFi_STA{9,j}(:,1)+length_room WiFi_STA{9,j}(:,2)+breadth_room];
end
WiFi_AP{2} =[WiFi_AP{2}(:,1)+length_room WiFi_AP{2}(:,2)];
WiFi_AP{3} =[WiFi_AP{3}(:,1)-length_room WiFi_AP{3}(:,2)];
WiFi_AP{4} =[WiFi_AP{4}(:,1) WiFi_AP{4}(:,2)+breadth_room];
WiFi_AP{5} =[WiFi_AP{5}(:,1) WiFi_AP{5}(:,2)-breadth_room];
WiFi_AP{6} =[WiFi_AP{6}(:,1)-length_room WiFi_AP{6}(:,2)-breadth_room];
WiFi_AP{7} =[WiFi_AP{7}(:,1)+length_room WiFi_AP{7}(:,2)-breadth_room];
WiFi_AP{8} =[WiFi_AP{8}(:,1)-length_room WiFi_AP{8}(:,2)+breadth_room];
WiFi_AP{9} =[WiFi_AP{9}(:,1)+length_room WiFi_AP{9}(:,2)+breadth_room];
% STA cordinates
% WiFi_STA={}; %contains the coordinates of the WIFI STAs
%     WiFi_STA{1}=coord;
plot(graph,[-length_room/2,length_room/2],[-breadth_room/2,-breadth_room/2],'-r',...
    [length_room/2,length_room/2],[-breadth_room/2,breadth_room/2],'-r',...
    [length_room/2,-length_room/2],[breadth_room/2,breadth_room/2],'-r',...
    [-length_room/2,-length_room/2],[breadth_room/2,-breadth_room/2],'-r');
hold on
grid on
%axis(graph,[(-length_room-20)/2 (length_room+20)/2 (-breadth_room-20)/2 (breadth_room+20)/2]);
drawnow
hold on