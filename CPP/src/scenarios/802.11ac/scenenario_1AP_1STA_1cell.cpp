/*! Wireless Network Simulator WiNetSim \n
    Copyright (C) 2016  IIT Hyderabad  \n

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    
    Contact Information
	WiNetSim Admin: sreekanth@iith.ac.in
	Address      : comm-lab, IIT Hyderabad 
    
*/
/*! \file scenario_1AP_1STA_1cell.cpp
 * \brief 1Ap 1STA room layout 
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#include "common.h"
#include "AP.h"
#include "STA.h"

/*! Simple 1 AP 1 STA 1 cell \n
 *  InH model with a room layout \n
 */ 
 //global variables
extern bool stop_signal_called;
extern long long system_time;
extern long long total_tti;
extern parameters common_params;
extern long long run_ttis; // total transmissions
using namespace std;

void Scene1_1AP_1STA_1cell()
{
	
	// STA object
	STA STA_obj;
	// AP object
	AP AP_obj;
	// AP coordinates
	double coordi_AP[2] = {0,0};
	// STA coordinates
	double coordi_STA[2] = {30,30};
	
	// initialize AP
	AP_obj.AP_init(0,coordi_AP,common_params);
	// initialize STA
	STA_obj.STA_init(0,coordi_STA,common_params,(void *)&AP_obj);
	
	// initialize jump
	int jump=0,jump1=0,jump2=0;;
	
	// Associate STA to AP
	AP_obj.ADD_STA((void *) &STA_obj);
	
	// Add listeners
	// ADD STA to AP listener group
	AP_obj.add_listener((void*) &STA_obj,STA_obj.TYPE);
	// ADD AP to STA listener group
	STA_obj.add_listener((void*) &AP_obj,AP_obj.TYPE);
	
	// Start simulation
	
	
	
	while (total_tti<run_ttis && stop_signal_called==false){ // run simulation for run_ttis TTIs
	// check buffers of AP
	jump1=AP_obj.check_buffer();
	if(jump1<0)
	std::cout<<" jump AP is in error : "<<jump1<<std::endl;
	
	// check buffers of STA
	jump2=STA_obj.check_buffer();
	if(jump2<0)
	std::cout<<" jump STA in error: "<<jump2<<std::endl;
	
	// schedule and tx AP
	AP_obj.sched_tx();
	// schedule and tx STA
	STA_obj.sched_tx();
	
	// check tx time AP
	AP_obj.check_tx_time();
	// check tx time STA
	STA_obj.check_tx_time();
	
	// Print the current time
	if(system_time%100000==0) // display time every 10ms
	std::cout<<'\r'<<system_time*1e-6<<" s time elapsed";//<<std::endl;
	
	// increment system time
	system_time++;
	
	// jump if no data available
	jump = jump1>jump2?jump2:jump1;
	if(jump>0){
	// jump at AP	
	AP_obj.jump_time(jump);
	// jump at STA	
	STA_obj.jump_time(jump);
	// jump system time
	system_time+=jump;	
		} // end of jump condition
	} // end of while loop
	
	// results
	std::cout<<std::endl;
	// measured load
	cout<<"________________________________Results_________________________________________"<<endl;
	std::cout<<"Simulation Run time : "<<system_time*1e-6<<" seconds"<<std::endl;
	std::cout<<"Measured Load       : "<<((double)(AP_obj.total_files+STA_obj.total_files))*common_params.file_size/((double)system_time)<<" Mbps"<<std::endl;
	std::cout<<"Given load          : "<<(double) common_params.Network_load_bps*1e-6<<" Mbps"<<std::endl;
	double meas_dl_serv = (double) AP_obj.served_data/((double)system_time);
	double meas_ul_serv = (double) STA_obj.served_data/((double)system_time);
	std::cout<<"ToT served load     : "<<meas_dl_serv+meas_ul_serv<<" Mbps"<<std::endl;
	std::cout<<"DL served load      : "<<meas_dl_serv<<" Mbps"<<std::endl;
	std::cout<<"UL served load      : "<<meas_ul_serv<<" Mbps"<<std::endl;
	cout<<"________________________________END_________________________________________"<<endl;
	
}

void populate_params_1AP_1STA_1cell() {
	// constants
	// Temperature
	double Temp = 270;
	// boltzman constant
	double k = 1.3806488 * 1e-23;
	// Noise Figure
	double NF = std::pow(10, 0.7);
	// Bandwidth of a single subcarrier
	double B_wifi = 20e6 / 64;
	// Thermal Noise floor in Watts
	double Noise = k * Temp * B_wifi * NF;
	common_params.Noise = Noise;

	// Layout Dimensions
	// Length of the room
	double length = 100;
	common_params.dimensions[0] = length;
	// Breadth of the room
	double breadth = 100;
	common_params.dimensions[1] = breadth;
	// Height of AP from ground
	double height_AP = 0;
	common_params.height_AP = height_AP;
	// Height of STA from ground
	double height_STA = 0;
	common_params.height_STA = height_STA;
	// System Specific
	// Total cells in the layout
	int NCell = 1;
	common_params.NCell = NCell;
	// Total number of nodes per cell
	int n_STA = 1;
	common_params.n_STA = n_STA;
	// Bandwidth of the system
	double BW = 20;
	common_params.BW = BW;
	// Carrier Frequency
	double fc = 5;
	common_params.fc = fc;

	// Tx power in dBm for 20MHz for AP
	double Tx_power_AP_dbm = 18;
	common_params.Pt_AP = std::pow(10,
			0.1 * (Tx_power_AP_dbm - 30 - 10 * std::log10(56)));

	// Tx power in dBm for 20MHz for STA
	double Tx_power_STA_dbm = 18;
	common_params.Pt_STA = std::pow(10,
			0.1 * (Tx_power_STA_dbm - 30 - 10 * std::log10(56)));

	// Channel Model
	std::string channel_model = "InH";
	common_params.channel_model = channel_model;
	// Total number of MCS 
	int MAX_MCS = 8;
	common_params.MAX_MCS = MAX_MCS;

	// Data fit in 20us for each MCS
	double WIFI_data_20us[] = { 160, 320, 480, 640, 960, 1280, 1440, 1600 };
	common_params.WIFI_data_20us.insert(common_params.WIFI_data_20us.begin(),
			WIFI_data_20us, WIFI_data_20us + MAX_MCS);
	// Traffic
	// File Size at application level for FTP traffic
	long long file_size = 4 * 1e6;
	common_params.file_size = file_size;

	// Down Link to Up Link traffic ratio
	double DL_UL = 1;
	common_params.DL_UL = DL_UL;

	// Percent of Downlink Traffic
	double DL_percent = 1 / (1 + DL_UL);
	common_params.DL_percent = DL_percent;

	// Percent of Uplink Traffic
	double UL_percent = 1 / (1 + (1 / DL_UL));
	common_params.UL_percent = UL_percent;

	// Whole Network load in bps
	double Network_load_bps = 60 * 1e6;
	common_params.Network_load_bps = Network_load_bps;

	// Downlink per user load in files per micro second
	double load_per_user_dl = DL_percent
			* (Network_load_bps / (file_size * (n_STA))) * 1e-6;
	common_params.load_peruser_dl = load_per_user_dl;

	// Uplink per user load in files per micro second
	double load_per_user_ul = UL_percent
			* (Network_load_bps / (file_size * (n_STA))) * 1e-6;
	common_params.load_peruser_ul = load_per_user_ul;

	// DCF params
	// WiFi Slot duration in micro seconds
	int wifi_slot = 9;
	common_params.wifi_slot = wifi_slot;

	// DIFS duration in micro seconds
	int DIFS_time = 34;
	common_params.DIFS_time = DIFS_time;

	// Minimum Contention Window
	int CW_min = 16;
	common_params.CW_min = CW_min;

	// Maximum Contention Window
	int CW_max = 1024;
	common_params.CW_max = CW_max;

	// Maximum transmit opportunity in micro seconds
	long long MAX_TX_OP_us = 4e3;
	common_params.MAX_TXOP = MAX_TX_OP_us;

	// Wifi Energy Detect Sensing threshold in dBm for 20MHz
	double Preamble_th_dbm = -82;
	// Wifi Energy Detect Sensing threshold in Watts per subcarrier
	common_params.WIFI_TH = std::pow(10,
			0.1 * (Preamble_th_dbm - 30 - 10 * std::log10(56)));

}
