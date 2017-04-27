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

clc;
%load('Inter_wifi_dcf_10_1');
points = 50;
%% delay
delay_wifi_dl_1 =[];
delay_wifi_ul_1 =[];

for i=1:NCell
    for j=1:n_AP
        for k = 1:n_STA
            t = AP_obj{1,i}(j).TTTDeliverOFile_DL{k};
            delay_wifi_dl_1 = [delay_wifi_dl_1 t];
            delay_wifi_ul_1 = [delay_wifi_ul_1 STA_obj{i,j}(k).TTTDeliverOFile_UL];
        end
    end
end
delay_wifi = [delay_wifi_dl_1 delay_wifi_ul_1];
% dl 1
% keyboard
% te1 = min(delay_wifi);
% te2 = max(delay_wifi);
% te3 = length(delay_wifi);
% x_wifi = linspace(te1,te2,te3);
% 
% for i=1:te3
%     wifi_delay_cdf(i) = length(find(delay_wifi<=x_wifi(i)));
% end
% cla(graph_delay);
% plot(graph_delay,x_wifi/1000,wifi_delay_cdf_dl_1/te3,'-b');
mean_wifi_dl_delay = mean(delay_wifi)/1000;
fprintf('Mean Delay is %f milli seconds\n',mean_wifi_dl_delay);



%% ro cdf
% ro_wifi=[];
% for i=1:NCell
%     t = AP_obj{1}(i).served_data./AP_obj{1}(i).total_files;
%     ro_wifi = [ro_wifi t];
%     t = AP_obj{2}(i).served_data./AP_obj{2}(i).total_files;
%     ro_wifi = [ro_wifi t];
%     for j=1:n_wifi-1    
%     ro_wifi = [ro_wifi STA_obj{1,i}(j).served_data/STA_obj{1,i}(j).total_files];
%     ro_wifi = [ro_wifi STA_obj{2,i}(j).served_data/STA_obj{2,i}(j).total_files];
%     end
% end
% 
% ro_wifi(isnan(ro_wifi))=[];
% te1 = min(ro_wifi);
% te2 = max(ro_wifi);
% te3 = length(ro_wifi);
% xro_wifi = linspace(te1,te2,80);
% 
% for i=1:80
%     wifi_ro_cdf(i) = length(find(ro_wifi<=xro_wifi(i)));
% end
% 
% figure,plot(xro_wifi/4e6,wifi_ro_cdf/te3,'-r');hold on
% mean_wifi_ro = mean(ro_wifi)/4e6
% legend('wifi ro');
%% network ro
net_ro_wifi_dl_1=0;data_wifi_dl_1=0;file_wifi_dl_1=0;
net_ro_wifi_ul_1=0;data_wifi_ul_1=0;file_wifi_ul_1=0;
for i=1:NCell
    for j=1:n_AP
    data_wifi_dl_1 = data_wifi_dl_1+sum(AP_obj{1,i}(j).served_data);
    file_wifi_dl_1 = file_wifi_dl_1+sum(AP_obj{1,i}(j).total_files);
    for k=1:n_STA
    data_wifi_ul_1 = data_wifi_ul_1+STA_obj{i,j}(k).served_data;
    file_wifi_ul_1 = file_wifi_ul_1+STA_obj{i,j}(k).total_files;
    end
    end
end
%ro_net_wifi_dl_1 = data_wifi_dl_1/(file_size*file_wifi_dl_1);
ro_net_wifi_dl_1 = data_wifi_dl_1/(system_time);
%ro_net_wifi_ul_1 = data_wifi_ul_1/(file_size*file_wifi_ul_1);
ro_net_wifi_ul_1 = data_wifi_ul_1/(system_time);
fprintf('RO_DL: %f RO_UL: %f \n',ro_net_wifi_dl_1,ro_net_wifi_ul_1);
%% BO
% BObs_1=[];BObs_2=[];BO_ue=[];
% for i=1:NCell
%     BObs_1 = [BObs_1 AP_obj{1}(i).BO_net];
%     for j=1:n_wifi-1
%         BO_ue = [BO_ue STA_obj{1,i}(j).BO];
%     end
% end
% 
% BO1 = (mean(BObs_1))/system_time;
%% load
% measure the given load
load_wifi1=0;load_wifi2=0;
for i=1:NCell
    for j=1:n_AP
    load_wifi1 = load_wifi1+sum(AP_obj{1,i}(j).total_files);
    for k=1:n_STA
    load_wifi1 = load_wifi1+STA_obj{i,j}(k).total_files;
    end
    end
end
load_wifi_1 = file_size*load_wifi1/system_time;
fprintf('Measured Total Network load is %f  and Given Load: %f\n',load_wifi_1,Network_load_Mbps/1e6);
fprintf('Sim Run Time %f seconds\n',system_time*1e-6);
%data_da = {mean_wifi_dl_upt;mean_wifi_ul_upt;mean_wifi_dl_delay;mean_wifi_ul_delay;...
%    ro_net_wifi_dl_1;ro_net_wifi_ul_1;BO1;load_wifi_1;round(system_time*1e-6)}

% set(handles.uitable1,'Data',data_da);