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
%


classdef STA_11ac <handle
    %% Public Parameters
    properties (SetAccess = public)
        coordinates; % 2D coordinates of the STA
        ACTIVE_Devices; %Distances of the active devices from the current device point of view
        TX_ind=0 % Transmit Indicator
        RX_ind=0 % Receive Indicator
        MCS_UL;
        MCS_MAX
        ID % Device ID
        TYPE='STA_11ac' % Device Type
        TECH='802.11ac'
        channel_type='InH'
        Channel_occupy_time=0;
        Buffer_UL % Uplink buffer
        TOT_BUFF=0; % Total Buffer
        DISASTER=0; %if 1 the unspeakable happened
        Time_next_file_arrives %time after which next file arrives
        Time_file_arrived_UL=[] %time file arrived in UL
        TTTDeliverOFile_UL=[] % time took to deliver one file
        UPT; % User Perceived Throughput
        served_data=0; % Data Served
        
        BUF_Ind=0; % Trackes the current number of files in the buffer
        graph_handle; % Handle for graph
        pumped_data=0; % Data Pumped into the network
        attempt=0;  %Total Tx Attempts
        success=0; % Total Success
        CW;     % Current Contention Window
        
    end
    %% Private Parameters
    properties (SetAccess = private)
        Serving_BS; %obj handle for serving BS/AP
        
        % DCF Parameters
        Counter=0; % Counter
        CW_min; % Minimum Contention Window
        CW_max; % Maximum Contention Window
        DIST_from_BS; % Distance from the serving AP/BS
        Sig_power=0; % Received Signal Power
        Int_power=0; % Interference Power Received
        Energy_meas=0; % Energy detection measurement
        BO=0; %buffer occupancy
        
        total_files=0; % total files sent and received
        local_time=0; % tracks the interference stationality
        load_peruser; % load or traffic
        
        contending=0; %if 0 not contending if 1 contending
        wifi_slot; % WiFi Slot duration in micro seconds
        DIFS_Ind=0; %0 not done 1 done
        DIFS_time; % DIFS duration
        DIFS_counter; % DIFS Counter
        MAX_TXOP; % Maximum Transmit Opportunity
        Noise; % thermal noise floor
        Pt_AP; % Per Subcarrier Tx Power in Wats;
        fc; %Carier Frequcy 5 GHz
        file_size; % 4 Mega Bits
        WIFI_TH;% Detection Threshold in Watts
        WIFI_data_20us
        height;% height of UE
        graph=0; % enable or disable Graph
    end
    
    
    events
        %% events
        %transmitting in future
        transmitting %event for transmission
        not_transmitting % event for finished transmission
    end
    
    methods
        
        function obj = STA_11ac(ue_id,coord,BS_handle,parameter,graph)
            %  STA_11ac IEEE 802.11ac station
        %  Implements 802.11 DCF \n
        %  PHY is completely Abstracted
%  Access Point node contains a traffic generator + lower MAC + PHY abstraction
            obj.ID=ue_id;
            obj.Serving_BS=BS_handle;
            obj.coordinates = coord;
            obj.load_peruser = parameter.load_per_user_ul;
            obj.Time_next_file_arrives=round(exprnd(1/obj.load_peruser));
            obj.graph = graph;
            obj.graph_handle = scatter(obj.graph,coord(1,1),coord(1,2),'gs');hold on
            drawnow
            obj.file_size=parameter.file_size;
            obj.Pt_AP=parameter.Pt_AP;
            obj.Noise=parameter.Noise;
            obj.height=parameter.height_AP;
            obj.WIFI_data_20us=parameter.WIFI_data_20us;
            obj.MCS_MAX = length(obj.WIFI_data_20us);
            obj.MCS_UL=obj.MCS_MAX;
            obj.fc=parameter.fc;
            obj.WIFI_TH=parameter.WIFI_TH;
            obj.MAX_TXOP=parameter.MAX_TXOP;
            obj.DIFS_time=parameter.DIFS_time;
            obj.CW_min=parameter.CW_min;
            obj.CW=obj.CW_min;
            obj.CW_max=parameter.CW_max;
            obj.wifi_slot=parameter.wifi_slot;
            temp = BS_handle.coordinates;
            obj.DIST_from_BS = sqrt((temp(1,1)-coord(1,1))^2+(temp(1,2)-coord(1,2))^2+(obj.height-BS_handle.height)^2);
        end
        
        
        
        function jump_t(obj,jump)
            %% jump
        % Do a time jump
            obj.Time_next_file_arrives=obj.Time_next_file_arrives-jump;
        end
        
        
        function jump = check_buffer(obj)
            %% check buffers
        % Checks the buffers for files
        % If buffers are not empty then contends for the channel
        % Does an energy detect
        % Updates Buffer Occupancy metrics
        %ADD data to buffer according to FTP Model 2
            global system_time
            
            
            full_buffer=0;
            if(full_buffer==0)
                obj.Time_next_file_arrives = obj.Time_next_file_arrives-1;
                if (obj.Time_next_file_arrives<=0)
                    obj.Time_file_arrived_UL=[obj.Time_file_arrived_UL system_time];
                    obj.Buffer_UL = [obj.Buffer_UL obj.file_size];
                    obj.BUF_Ind = obj.BUF_Ind+1;
                    obj.total_files = obj.total_files+1;
                    obj.TOT_BUFF=obj.TOT_BUFF+obj.file_size;
                    obj.Time_next_file_arrives=round(exprnd(1/obj.load_peruser));
                end
                jump = obj.Time_next_file_arrives;
            else
                jump=0;
                obj.BUF_Ind=1;
                obj.TOT_BUFF = obj.file_size;
                obj.Buffer_UL = obj.file_size;
            end
            
            %check the buffer
            if(obj.contending==0 && obj.TX_ind==0 && obj.RX_ind==0) %if not TX and not Contending and not RX
                if(obj.BUF_Ind>0)
                    obj.contending=1;
                    obj.BO = obj.BO+1;
                    jump=0;
                    
                    %ADD a graph if needed
                    
                    delete(obj.graph_handle)
                    obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'cs','filled');hold on
                    drawnow
                    
                    
                    % Initialize DCF parameters
                    obj.Counter = (randi([0 obj.CW-1]))*obj.wifi_slot;
                    obj.DIFS_Ind=0;
                    obj.DIFS_counter=obj.DIFS_time;
                    
                    % Do Energy Measurement
                    % sum all the powers from all devices
                    PL=sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Energy_meas=PL+obj.Noise;
                    
                end
                
            elseif(obj.contending==1) %if contending and Not transmitting
                jump=0;
                obj.BO = obj.BO+1;
                %do ED
                PL=sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                if(isempty(PL))
                    PL=0;
                end
                obj.Energy_meas=PL+obj.Noise;
                
            else % If transimitting or receiving
                obj.BO = obj.BO+1;
                jump=0;
            end
            
        end
        
        
        
        function dec_counter(obj)
            %% decrement the counters
        % DCF is implemented here
        % Scheduling is done here
            if(obj.contending==1) %if contending and not tx
                if(obj.DIFS_Ind==1) % If channel is free for DIFS Duration
                    if(obj.Energy_meas>obj.WIFI_TH)
                        obj.Counter =obj.Counter-mod(obj.Counter,obj.wifi_slot)+obj.wifi_slot;
                        obj.DIFS_Ind=0;
                        obj.DIFS_counter=obj.DIFS_time;
                    else
                        obj.Counter = obj.Counter-1;
                    end
                    if(obj.Counter<=0) % if the counter goes to zero then we got the transmit opportunity
                        obj.contending =0; % we are no more contending
                        obj.DIFS_Ind=0; % About to occupy the channel
                        obj.DIFS_counter=obj.DIFS_time; % Reset the DIFS COunter
                        obj.TX_ind=1; % Update transmit Indicator
                        
                        %% Schedule
                        % Find the number of 20us slots required
                        time = ceil(obj.TOT_BUFF/obj.WIFI_data_20us(obj.MCS_UL));
                        %used for UL MCS
                        obj.attempt = obj.attempt+1;
                        % if the time required is less than max transmit
                        % opportunity then occupy channel for max Tx opportunity
                        if(20*time<obj.MAX_TXOP)
                            obj.Channel_occupy_time = time*20;
                            obj.Serving_BS.ALLOC_data_UL(obj.ID) = obj.TOT_BUFF;
                        else %otherwise occupy the required slots
                            obj.Channel_occupy_time = obj.MAX_TXOP;
                            obj.Serving_BS.ALLOC_data_UL(obj.ID)=obj.WIFI_data_20us(obj.MCS_UL)*obj.MAX_TXOP/20;
                        end
                        %Notify the  receivers
                        %Update graphs
                        
                        delete(obj.graph_handle)
                        obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'bs','filled');hold on
                        drawnow
                        
                        notify(obj,'transmitting');
                    end
                else % If DIFS is not over. do CCA for DIFS duration
                    if(obj.Energy_meas<obj.WIFI_TH)
                        obj.DIFS_counter = obj.DIFS_counter-1;
                        if(obj.DIFS_counter<=0)
                            obj.DIFS_Ind=1;
                            obj.DIFS_counter=obj.DIFS_time;
                        end
                    end
                end
            end
        end
        
        
        
        function dec_time(obj)
            %% dec time if tx
        % If transmitting decrement the time every micro second
            if(obj.TX_ind==1)
                obj.Channel_occupy_time=obj.Channel_occupy_time-1;
                if(obj.Channel_occupy_time<=0)
                    obj.TX_ind=0;
                    %Update graph
                    
                    delete(obj.graph_handle)
                    obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'gs');hold on
                    drawnow
                    
                    %notify finish transmitting
                    notify(obj,'not_transmitting');
                end
            end
        end
        
        
        function cb_tx(obj,obj1)
            %% transmitting callback
        % This is a callback function for the event 'transmitting'
        %
            global system_time
            % check if the transmitting device is the serving AP
            temp = (obj.Serving_BS==obj1);
            if(isempty(temp))
                temp=0;
            end
            %If the transmitting device is the serving AP and if the
            %current STA is not transmitting and if the transmission is
            %intended for current STA then start receiving
            if (temp && obj.Serving_BS==obj1 && obj.TX_ind~=1 && obj1.CURRENT_STA_DL==obj.ID)
                obj.contending =0;
                obj.RX_ind=1;
                
                delete(obj.graph_handle)
                obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'gs','filled');hold on
                drawnow
                
                obj.Int_power=0;
                obj.Sig_power=0;
                obj.local_time = system_time;
                obj.Channel_occupy_time=obj1.Channel_occupy_time;
            else
                % Otherwise consider it as interference
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                % find the distance from the interferer
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                % If current STA is receiving then update the
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Int_power = obj.Int_power+PL*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices =[obj.ACTIVE_Devices d];
                else
                    obj.ACTIVE_Devices =[obj.ACTIVE_Devices d];
                end
            end
            
        end
        
       
        function cb_notx(obj,obj1)
             %%  call back for finished tx
            global system_time kk
            
            temp = (obj.Serving_BS==obj1);
            if(isempty(temp))
                temp=0;
            end
            if(temp && obj.RX_ind==1 && obj1.CURRENT_STA_DL==obj.ID)
                obj.RX_ind=0;
                
                delete(obj.graph_handle)
                obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'gs');hold on
                drawnow
                
                PL = sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                if(isempty(PL))
                    PL=0;
                end
                obj.Int_power = obj.Int_power+PL*(system_time-obj.local_time)+obj.Channel_occupy_time*obj.Noise;
                PL_sig = PL_measure(obj.DIST_from_BS,obj.fc,obj.channel_type);
                obj.Sig_power =  obj.Channel_occupy_time*obj1.Pt_AP*10^(PL_sig/10);
                SINR = 10*log10(obj.Sig_power/(obj.Int_power));
                obj1.SINR = [obj1.SINR SINR];
                collision = find_prob_coll(SINR,obj1.MCS_DL(obj.ID),obj.TECH);
                kk=kk+1;
                obj1.pumped_data(obj.ID) = obj1.pumped_data(obj.ID) +obj1.ALLOC_data_DL(obj.ID);
                if(collision==0)
                   
                    obj1.success(obj.ID) = obj1.success(obj.ID)+1;
                    obj1.Buffer_DL{obj.ID}(1)=obj1.Buffer_DL{obj.ID}(1)-obj1.ALLOC_data_DL(obj.ID);
                    obj1.served_data(obj.ID) = obj1.served_data(obj.ID) +obj1.ALLOC_data_DL(obj.ID);
                    obj1.SUM_BUFF_DL(obj.ID) = obj1.SUM_BUFF_DL(obj.ID) - obj1.ALLOC_data_DL(obj.ID);
                    obj1.TOT_BUFF = obj1.TOT_BUFF - obj1.ALLOC_data_DL(obj.ID);
                    if(obj1.Buffer_DL{obj.ID}(1)<=0)
                        te2 = system_time-obj1.Time_file_arrived_DL{obj.ID}(1);
                        obj1.TTTDeliverOFile_DL{obj.ID}=[obj1.TTTDeliverOFile_DL{obj.ID} te2];
                        obj1.Time_file_arrived_DL{obj.ID}=obj1.Time_file_arrived_DL{obj.ID}(2:end);
                        obj1.UPT = [obj1.UPT obj.file_size/te2];
                        if( obj1.SUM_BUFF_DL(obj.ID)>0)
                            obj1.Buffer_DL{obj.ID}(2) =obj1.Buffer_DL{obj.ID}(2)+obj1.Buffer_DL{obj.ID}(1);
                        else
                            obj1.served_data(obj.ID) = obj1.served_data(obj.ID) +obj1.SUM_BUFF_DL(obj.ID);
                            obj1.TOT_BUFF = obj1.TOT_BUFF-obj1.SUM_BUFF_DL(obj.ID);
                            obj1.SUM_BUFF_DL(obj.ID)=0;
                        end
                        obj1.Buffer_DL{obj.ID} = obj1.Buffer_DL{obj.ID}(2:end);
                        obj1.BUF_Ind=obj1.BUF_Ind-1;
                    end
                    % pick a MCS
                    % MCS adaptation 
                    
                    if(obj1.attempt(obj.ID)==10)
                        %above 70% increase mcs
                        if(obj1.success(obj.ID)>7)
                            obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)+1;
                            if(obj1.MCS_DL(obj.ID)>obj.MCS_MAX)
                                obj1.MCS_DL(obj.ID)=obj.MCS_MAX;
                            end
%                             disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                        elseif(obj1.success(obj.ID)<3)
                            obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)-1;
                            if(obj1.MCS_DL(obj.ID)<1)
                                obj1.MCS_DL(obj.ID)=1;
                            end
%                             disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                        end
                        obj1.success(obj.ID)=0;
                        obj1.attempt(obj.ID)=0;
                    end
                    % MCS adaptation without feedback
                    % update contention window
                    obj1.CW = obj.CW_min;
                else
                     %disp(' ### collision ###')
                    % MCS Adaptation without feedback
                    obj1.CW = obj1.CW*2;
                    if(obj1.CW>obj.CW_max)
                        obj1.CW = obj.CW_max;
                    end
                    if(obj1.attempt(obj.ID)==10)
                        %above 70% increase mcs
                        if(obj1.success(obj.ID)>7)
                            obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)+1;
                            if(obj1.MCS_DL(obj.ID)>obj.MCS_MAX)
                                obj1.MCS_DL(obj.ID)=obj.MCS_MAX;
                            end
%                             disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                        elseif(obj1.success(obj.ID)<3)
                            obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)-1;
                            if(obj1.MCS_DL(obj.ID)<1)
                                obj1.MCS_DL(obj.ID)=1;
                            end
%                             disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                        end
                        obj1.success(obj.ID)=0;
                        obj1.attempt(obj.ID)=0;
                    end
                end
                obj1.ALLOC_data_DL(obj.ID)=0;
            elseif(temp && obj1.CURRENT_STA_DL==obj.ID)
                obj1.CW = obj1.CW*2;
                if(obj1.CW>obj.CW_max)
                    obj1.CW = obj.CW_max;
                end
                if(obj1.attempt(obj.ID)==10)
                    %above 70% increase mcs
                    if(obj1.success(obj.ID)>7)
                        obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)+1;
                        if(obj1.MCS_DL(obj.ID)>obj.MCS_MAX)
                            obj1.MCS_DL(obj.ID)=obj.MCS_MAX;
                        end
%                         disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                    elseif(obj1.success(obj.ID)<3)
                        obj1.MCS_DL(obj.ID) =obj1.MCS_DL(obj.ID)-1;
                        if(obj1.MCS_DL(obj.ID)<1)
                            obj1.MCS_DL(obj.ID)=1;
                        end
%                         disp(sprintf('%s %d: STA %d MCS DL %d \n',obj1.TYPE,obj1.ID,obj.ID,obj1.MCS_DL(obj.ID)))
                    end
                    obj1.success(obj.ID)=0;
                    obj1.attempt(obj.ID)=0;
                end
                
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_Ap.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Int_power = obj.Int_power+PL*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                    
                else
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                    
                end
            else
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Int_power = obj.Int_power+PL*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                    
                else
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                    
                end
            end
        end
        
        
        %% add listener
        % Listen to events originating from other objects
        
        function listen_to(obj,obj1)
            for i=1:length(obj1)
                addlistener(obj1(i),'transmitting',...
                    @(src,event)obj.cb_tx(src));
                addlistener(obj1(i),'not_transmitting',...
                    @(src,event)obj.cb_notx(src));
            end
        end
    end
    
end

