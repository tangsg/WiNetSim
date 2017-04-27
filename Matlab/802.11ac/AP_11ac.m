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
% AP_11ac IEEE 802.11ac access point
%  Implements 802.11 DCF \n
%  PHY is completely Abstracted
%  Access Point node contains a traffic generator + lower MAC + PHY abstraction

classdef AP_11ac<handle
    
    properties (SetAccess = public)
        ID % ID of the AP
        TYPE='AP_11ac' % Device Type
        TECH='802.11ac' % Device technology
        channel_type='InH' % channel model
        coordinates; %coordinates of the AP
        MCS_DL % DL MCS is a vector
        MAX_MCS % Total MCS available
        CURRENT_STA_DL=1; % ID of the current STA in DL
        CURRENT_STA_UL % ID of current STA in UL
        ACTIVE_Devices; %Distances of the active devices
        ALLOC_data_DL% Data allocated in downlink vector
        ALLOC_data_UL% Data allocated in uplink vector
        TX_ind=0 % Transmit indicator
        RX_ind=0 % receive indicator
        Buffer_DL % DL buffer array of cells
        SUM_BUFF_DL % vector
        BUF_Ind=0; % 0 if no data !! >0 if data
        TOT_BUFF=0; % Total sum of buffer scalar
        Channel_occupy_time=0; % channel occupy time in DL or UL
        Time_file_arrived_DL %time when packet arrived
        TTTDeliverOFile_DL % time took to deliver one file stats
        UPT; % user perceived throughput stats
        SINR; % SINR stats
        served_data; % total successfully delivered data stats
        graph_handle; % handle to current AP object inside graph
        pumped_data; % total data transmitted irrespective of success or fail
        attempt; % sttempts used for MCS adaptation
        success; % success used of MCS adaptation
        graph;  % object handle to current graph
        CW; % current contention window
    end
    
    properties (SetAccess = private)
        CW_min; % minimum contention window
        CW_max; % maximum contention window
        Counter; % counter
        contending=0; %0 if not contending 1 if contending
        wifi_slot; % wifi slot duration in us
        DIFS_time; % DIFS time in us
        DIFS_Ind=0; % DIFS indicator
        DIFS_counter=0; % DIFS counter
        MAX_TXOP; % maximum transit opportunity in us
        Energy_meas=0; % Energy detection
        Sig_power=0; % signal power
        Int_power=0; % interference power
        load_peruser; % load per user
        Time_next_file_arrives %time after which next file arrives in us
        BS_UEs=[]; % obj handles for all the connected STAs
        Connected_UEs; % total connected STAs
        BO=0; %buffer occupancy STATS
        BO_ue; % ue buffer occupancy STATS
        BO_net=0; % STATS
        total_files; % total files arrived STATS
        local_time=0; % time interference last changed
        Noise % noise floor measured in 1 subcarrier
        Pt_AP % AP transmit power in watts per subcarrier
        RX_GAIN=10^(0/10); % RX gain
        fc; %in GHz
        file_size; % in bits per second
        WIFI_TH % WIFI
        % bits transmitted in 20micro seconds for each MCS
        WIFI_data_20us
        height % height of BS
    end
    
    %events
    events
        transmitting % event for device start transmitting
        not_transmitting % event for device finished transmitting
    end
    
    methods
        
        function obj = AP_11ac(ID,coord,no_ues,parameter,graph)
            % AP_11ac IEEE 802.11ac access point
            % AP_11ac(ID,coord,no_ues,parameter,graph) where ID is the input ID must run from 1 to n_AP
            % coord is the coordinates of the AP
            % no_ues are the total number of STAs connected
            % parameter is the structure to common parameters
            % graph is the handle to graph
            obj.ID = ID;
            obj.Connected_UEs = no_ues;
            obj.pumped_data = zeros(1,no_ues);
            obj.BO_ue = zeros(1,no_ues);
            obj.coordinates=coord;
            obj.load_peruser = parameter.load_per_user_dl;
            obj.Time_next_file_arrives=round(exprnd(1/obj.load_peruser,[1,no_ues]));
            obj.Time_file_arrived_DL = cell(1,no_ues);
            obj.TTTDeliverOFile_DL = cell(1,no_ues);
            obj.ALLOC_data_DL=zeros(1,no_ues);
            obj.attempt=zeros(1,no_ues);
            obj.success=zeros(1,no_ues);
            obj.ALLOC_data_UL=zeros(1,no_ues);
            obj.Buffer_DL=cell(1,no_ues);
            obj.SUM_BUFF_DL = zeros(1,no_ues);
            obj.served_data = zeros(1,no_ues);
            obj.total_files = zeros(1,no_ues);
            obj.graph = graph;
            obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'bo');hold on
            drawnow
            obj.file_size=parameter.file_size;
            obj.Pt_AP=parameter.Pt_AP;
            obj.Noise=parameter.Noise;
            obj.height=parameter.height_AP;
            obj.WIFI_data_20us=parameter.WIFI_data_20us;
            obj.MAX_MCS = length(obj.WIFI_data_20us);
            obj.MCS_DL=obj.MAX_MCS*ones(1,no_ues);
            obj.fc=parameter.fc;
            obj.WIFI_TH=parameter.WIFI_TH;
            obj.MAX_TXOP=parameter.MAX_TXOP;
            obj.DIFS_time=parameter.DIFS_time;
            obj.CW_min=parameter.CW_min;
            obj.CW=obj.CW_min;
            obj.CW_max=parameter.CW_max;
            obj.wifi_slot=parameter.wifi_slot;
        end
        
        function ADD_UE(obj,obj1)
            %% ADD UEs
            % Add the STA object handles to the AP list
            obj.BS_UEs =[obj.BS_UEs obj1] ;
        end
        
        function jump_t(obj,jump)
            %% jump
            % jump time
            obj.Time_next_file_arrives=obj.Time_next_file_arrives-jump;
        end
        
        
        function jump=check_buffer(obj)
            %% check the buffers
            % Do Energy detect
            global system_time
            full_buffer=0;
            %ADD data to buffer
            if(full_buffer==0)
                obj.Time_next_file_arrives = obj.Time_next_file_arrives-1;
                x = find(obj.Time_next_file_arrives<=0);
                for i=x
                    obj.Time_file_arrived_DL{i}=[obj.Time_file_arrived_DL{i} system_time];
                    obj.Time_next_file_arrives(i)=round(exprnd(1/obj.load_peruser));
                    obj.Buffer_DL{i} = [obj.Buffer_DL{i} obj.file_size];
                    obj.SUM_BUFF_DL(i) = obj.SUM_BUFF_DL(i)+obj.file_size;
                    obj.BUF_Ind = obj.BUF_Ind+1;
                    obj.total_files(i) = obj.total_files(i)+1;
                    obj.TOT_BUFF = obj.TOT_BUFF+obj.file_size;
                end
                jump = min(obj.Time_next_file_arrives);
            else
                % for full buffer simulations
                jump=0;
                obj.BUF_Ind=1;
                obj.SUM_BUFF_DL = obj.file_size*ones(1,obj.Connected_UEs);
                obj.Buffer_DL = obj.file_size*ones(1,obj.Connected_UEs);
                obj.TOT_BUFF = obj.file_size*ones(1,obj.Connected_UEs);
            end
            %check the buffer
            if(obj.contending==0 && obj.TX_ind==0 && obj.RX_ind==0) %if not TX and not Contending and not RX
                if(obj.BUF_Ind>0)
                    obj.BO=obj.BO+1; %buffer occupancy
                    obj.contending=1;
                    jump=0;
                    %ADD a graph if needed
                    
                    delete(obj.graph_handle)
                    obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'co','filled');hold on
                    drawnow
                    
                    obj.Counter = (randi([0 obj.CW-1]))*obj.wifi_slot;
                    obj.DIFS_Ind=0;
                    obj.DIFS_counter=obj.DIFS_time;
                    obj.Energy_meas=obj.Noise;
                    PL=sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10)); %power measurememnt
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Energy_meas=obj.Energy_meas+(PL*obj.RX_GAIN);
                end
            elseif(obj.contending==1)
                jump=0;
                obj.BO=obj.BO+1; %buffer occupancy
                PL=sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10)); %power measurememnt
                if(isempty(PL))
                    PL=0;
                end
                obj.Energy_meas=(PL*obj.RX_GAIN)+obj.Noise;
            else
                obj.BO=obj.BO+1; %buffer occupancy
                jump=0;
            end
            
            %% BO per user average
            for i=1:obj.Connected_UEs
                if(obj.SUM_BUFF_DL(i)>0)
                    obj.BO_ue(i)=obj.BO_ue(i)+1;
                end
            end
            for i=1:obj.Connected_UEs
                if(obj.SUM_BUFF_DL(i)>0 || obj.BS_UEs(i).TOT_BUFF>0)
                    obj.BO_net=obj.BO_net+1;
                    break;
                end
            end
        end
        
        function dec_counter(obj)
            %% decrement the counters
            if(obj.contending==1) %if contending and not tx
                %% Decrement counters only after DIFS Time
                if(obj.DIFS_Ind==1)
                    if(obj.Energy_meas>obj.WIFI_TH)
                        obj.Counter =obj.Counter-mod(obj.Counter,obj.wifi_slot)+obj.wifi_slot;
                        obj.DIFS_Ind=0;
                        obj.DIFS_counter=obj.DIFS_time;
                    else
                        obj.Counter = obj.Counter-1;
                    end
                    %% If coutner zero schedulae and tx
                    if(obj.Counter<=0)
                        
                        obj.contending =0;
                        obj.DIFS_Ind=0;
                        obj.DIFS_counter=obj.DIFS_time;
                        obj.TX_ind=1;
                        %% Schedule
                        obj.Channel_occupy_time=0;
                        obj.CURRENT_STA_DL = mod(obj.CURRENT_STA_DL,obj.Connected_UEs)+1;
                        while(obj.Channel_occupy_time==0)
                            if(obj.SUM_BUFF_DL(obj.CURRENT_STA_DL)>0)
                                %used for DL MCS
                                obj.attempt(obj.CURRENT_STA_DL) = obj.attempt(obj.CURRENT_STA_DL)+1;
                                
                                time = ceil(obj.SUM_BUFF_DL(obj.CURRENT_STA_DL)/obj.WIFI_data_20us(obj.MCS_DL(obj.CURRENT_STA_DL)));
                                % add control over head time for more
                                % accurate results
                                if(20*time<obj.MAX_TXOP)
                                    obj.Channel_occupy_time = 20*time;
                                    obj.ALLOC_data_DL(obj.CURRENT_STA_DL) = obj.SUM_BUFF_DL(obj.CURRENT_STA_DL);
                                else
                                    obj.Channel_occupy_time = obj.MAX_TXOP;
                                    obj.ALLOC_data_DL(obj.CURRENT_STA_DL)=obj.WIFI_data_20us(obj.MCS_DL(obj.CURRENT_STA_DL))*obj.MAX_TXOP/20;
                                end
                            else
                                obj.CURRENT_STA_DL = mod(obj.CURRENT_STA_DL,obj.Connected_UEs)+1;
                            end
                        end
                        delete(obj.graph_handle)
                        obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'bo','filled');hold on
                        drawnow
                        notify(obj,'transmitting');
                    end %end of counter zero
                else
                    if(obj.Energy_meas<obj.WIFI_TH)
                        obj.DIFS_counter = obj.DIFS_counter-1;
                        if(obj.DIFS_counter<=0)
                            obj.DIFS_Ind=1;
                            obj.DIFS_counter=obj.DIFS_time;
                        end
                    end
                end
            end % end of contending
        end %end of function
        
        function dec_time(obj)
            %% decrement time if tx
            if(obj.TX_ind==1)
                obj.Channel_occupy_time=obj.Channel_occupy_time-1;
                if(obj.Channel_occupy_time<=0)
                    obj.TX_ind=0;
                    delete(obj.graph_handle)
                    obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'bo');hold on
                    drawnow
                    notify(obj,'not_transmitting')
                end
            end
        end
        
        % callbacks
        
        function cb_tx(obj,obj1)
            %callback if any other transmitting
            % Call back for tx
            % if an AP notifys transmitting this function checks if it is in
            % the UES list. if it is then checks if AP is tx or rx is not then
            % starts receiving from the given STA
            % select the UE only if AP can hear him
            global system_time
            temp = find(obj.BS_UEs==obj1);
            %disp(system_time)
            if(isempty(temp))
                temp=0;
            end
            if (temp && obj.TX_ind~=1 && obj.RX_ind~=1)
                %if it is intended for this AP
                obj.RX_ind=1;
                
                delete(obj.graph_handle)
                obj.graph_handle = scatter(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'go','filled');hold on
                drawnow
                
                obj.contending=0;
                obj.CURRENT_STA_UL = obj1.ID;
                obj.local_time = system_time;
                obj.Sig_power=0;
                obj.Int_power=0;
                obj.Channel_occupy_time=obj1.Channel_occupy_time;
            else
                %disp('added interference')
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Int_power = obj.Int_power+(PL*obj.RX_GAIN)*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices =[obj.ACTIVE_Devices d];
                else
                    obj.ACTIVE_Devices =[obj.ACTIVE_Devices d];
                end
            end
        end
        
       
        function cb_notx(obj,obj1)
             %% callback for finished tx
            %callback if any other node stops transmitting
            global system_time kk
            %disp(system_time)
            temp = find(obj.BS_UEs==obj1);
            if(isempty(temp))
                temp=0;
            end
            if(temp && obj.RX_ind==1 && obj.CURRENT_STA_UL==obj1.ID)
                obj.RX_ind=0;
                
                delete(obj.graph_handle)
                obj.graph_handle = plot(obj.graph,obj.coordinates(1,1),obj.coordinates(1,2),'bo');hold on
                drawnow
                
                % Calculate SINR
                PL = sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                if(isempty(PL))
                    PL=0;
                end
                obj.Int_power = obj.Int_power+(PL*obj.RX_GAIN)*(system_time-obj.local_time)+obj.Channel_occupy_time*obj.Noise;
                PL_sig = PL_measure(obj1.DIST_from_BS,obj.fc,obj.channel_type);
                obj.Sig_power =  obj.Channel_occupy_time*obj1.Pt_AP*obj.RX_GAIN*10^(PL_sig/10);
                SINR_db = 10*log10(obj.Sig_power/(obj.Int_power));
                obj.SINR = [obj.SINR SINR_db];
                collision = find_prob_coll(SINR_db,obj1.MCS_UL,obj.TECH);
                kk=kk+1;
                obj1.pumped_data = obj1.pumped_data+obj.ALLOC_data_UL(obj1.ID);
                if(collision==0)
                    %used for UL MCS
                    obj1.success = obj1.success+1;
                    obj1.Buffer_UL(1)=obj1.Buffer_UL(1)-obj.ALLOC_data_UL(obj1.ID);
                    obj1.served_data = obj1.served_data+obj.ALLOC_data_UL(obj1.ID);
                    obj1.TOT_BUFF = obj1.TOT_BUFF - obj.ALLOC_data_UL(obj1.ID);
                    if(obj1.Buffer_UL(1)<=0)
                        te2 = system_time-obj1.Time_file_arrived_UL(1);
                        obj1.TTTDeliverOFile_UL=[obj1.TTTDeliverOFile_UL te2];
                        obj1.Time_file_arrived_UL=obj1.Time_file_arrived_UL(2:end);
                        obj1.UPT = [obj1.UPT obj.file_size/te2];
                        if(obj1.TOT_BUFF>0)
                            obj1.Buffer_UL(2) =obj1.Buffer_UL(2)+obj1.Buffer_UL(1);
                        else
                            obj1.served_data = obj1.served_data+obj1.TOT_BUFF;
                            obj1.TOT_BUFF = 0;
                        end
                        obj1.Buffer_UL = obj1.Buffer_UL(2:end);
                        obj1.BUF_Ind=obj1.BUF_Ind-1;
                    end
                    if(obj1.attempt==10)
                        %above 70% increase mcs
                        if(obj1.success>7)
                            obj1.MCS_UL =obj1.MCS_UL+1;
                            if(obj1.MCS_UL>obj.MAX_MCS)
                                obj1.MCS_UL=obj.MAX_MCS;
                            end
                        elseif(obj1.success<3)
                            obj1.MCS_UL =obj1.MCS_UL-1;
                            if(obj1.MCS_UL<1)
                                obj1.MCS_UL=1;
                            end
                        end
                        obj1.attempt=0;
                        obj1.success=0;
                    end
                    % update contention window
                    obj1.CW = obj.CW_min;
                else
                    %disp(' ### collision ###')
                    obj1.CW = obj1.CW*2;
                    if(obj1.CW>obj.CW_max)
                        obj1.CW = obj.CW_max;
                    end
                    if(obj1.attempt==10)
                        %above 70% increase mcs
                        if(obj1.success>7)
                            obj1.MCS_UL =obj1.MCS_UL+1;
                            if(obj1.MCS_UL>obj.MAX_MCS)
                                obj1.MCS_UL=obj.MAX_MCS;
                            end
                        elseif(obj1.success<3)
                            obj1.MCS_UL =obj1.MCS_UL-1;
                            if(obj1.MCS_UL<1)
                                obj1.MCS_UL=1;
                            end
                        end
                        obj1.attempt=0;
                        obj1.success=0;
                    end
                end
                obj.ALLOC_data_UL(obj1.ID)=0;
            elseif(temp)
                obj1.CW = obj1.CW*2;
                if(obj1.CW>obj.CW_max)
                    obj1.CW = obj.CW_max;
                end
                if(obj1.attempt==10)
                    %above 70% increase mcs
                    if(obj1.success>7)
                        obj1.MCS_UL =obj1.MCS_UL+1;
                        if(obj1.MCS_UL>obj.MAX_MCS)
                            obj1.MCS_UL=obj.MAX_MCS;
                        end
                    elseif(obj1.success<3)
                        obj1.MCS_UL =obj1.MCS_UL-1;
                        if(obj1.MCS_UL<1)
                            obj1.MCS_UL=1;
                        end
                    end
                    obj1.attempt=0;
                    obj1.success=0;
                    
                end
                %disp(obj1.CW);
                % Remove interference
                %find the distance and u
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    %disp(PL)
                    obj.Int_power = obj.Int_power+PL*obj.RX_GAIN*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                    
                else
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                end
            else
                %find the distance and u
                %disp('added interference')
                temp1 = obj.coordinates;
                temp2 = obj1.coordinates;
                d = sqrt((temp1(1,1)-temp2(1,1))^2+(temp1(1,2)-temp2(1,2))^2+(obj.height-obj1.height)^2);
                if(obj.RX_ind==1)
                    PL =sum(obj.Pt_AP.*10.^(PL_measure(obj.ACTIVE_Devices,obj.fc,obj.channel_type)/10));
                    if(isempty(PL))
                        PL=0;
                    end
                    obj.Int_power = obj.Int_power+PL*obj.RX_GAIN*(system_time-obj.local_time);
                    obj.local_time = system_time;
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                else
                    obj.ACTIVE_Devices(obj.ACTIVE_Devices==d)=[];
                end
            end
        end
        
        function listen_to(obj,obj1)
            %% add listener
            for i=1:length(obj1)
                
                addlistener(obj1(i),'transmitting',...
                    @(src,event)obj.cb_tx(src));
                addlistener(obj1(i),'not_transmitting',...
                    @(src,event)obj.cb_notx(src));
            end
        end
        
    end %end of methods
    
    
end %end of class
