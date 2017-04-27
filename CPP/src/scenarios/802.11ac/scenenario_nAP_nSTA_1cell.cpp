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
/*! \file scenario_1AP_nSTA_1cell.cpp
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

/*! Simple 1 AP n STA 1 cell \n
 *  InH model with a room layout \n
 */ 
 //global variables
 extern bool stop_signal_called;
extern long long system_time;
extern long long total_tti;
extern parameters common_params;
extern long long run_ttis; // total transmissions
using namespace std;

void Scene1_nAP_nSTA_1cell()
{
	// init uniform real distibution between [0,1]
	std::uniform_real_distribution<double> uni_distribution(-0.5, 0.5);
	// set random device generator
	std::random_device generator;
	bool inside;
	// AP object
	AP *AP_obj = new AP[common_params.n_AP];
	// AP coordinates
	double coordi_AP[2] = {0,0};
	
	for (int i=0;i<common_params.n_AP;i++)
	{
		while(!inside){
			coordi_AP[0] = uni_distribution(generator)*common_params.dimensions[0];
			coordi_AP[1] = uni_distribution(generator)*common_params.dimensions[1];
			inside = is_inside_rectangle(common_params.dimensions, coordi_AP);
		}
		//cout<<"x: "<<coordi_STA[0]<<" y: "<<coordi_STA[1]<<endl;
		// initialize the STA objects
		AP_obj[i].AP_init(i,coordi_AP,common_params);
	}
	// STA object
	STA **STA_obj = new STA*[common_params.n_AP];
	for (int i=0;i<common_params.n_AP;i++)
	{
		STA_obj[i] = new STA[common_params.n_STA];
	}
	
	// STA coordinates
	double coordi_STA[2] = {0,0};
	for (int j=0;j<common_params.n_AP;j++){
	for (int i=0;i<common_params.n_STA;i++)
	{
		while(!inside){
			coordi_STA[0] = uni_distribution(generator)*common_params.dimensions[0];
			coordi_STA[1] = uni_distribution(generator)*common_params.dimensions[1];
			inside = is_inside_rectangle(common_params.dimensions, coordi_STA);
		}
		//cout<<"x: "<<coordi_STA[0]<<" y: "<<coordi_STA[1]<<endl;
		// initialize the STA objects
		STA_obj[j][i].STA_init(i,coordi_STA,common_params,(void *)&AP_obj[j]);
		// Associate STA to AP
		AP_obj[j].ADD_STA((void *) &STA_obj[j][i]);
		for (int k=0;k<common_params.n_AP;k++){
		// ADD all APs to STA listener group
		STA_obj[j][i].add_listener((void*) &AP_obj[k],AP_obj[k].TYPE);
		// ADD STA to all AP listener group
		AP_obj[k].add_listener((void*) &STA_obj[j][i],STA_obj[j][i].TYPE);
		}
		inside = false;
	}
	}
	
	// ADD all STAs to each STA listener group
	for (int i=0;i<common_params.n_AP;i++){
		for (int j=0;j<common_params.n_STA;j++){
			//All the STAs under this AP
			for (int k=0;k<common_params.n_STA && j!=k;k++){
			// ADD all STAs except itself
			STA_obj[i][j].add_listener((void*) &STA_obj[i][k],STA_obj[i][k].TYPE);
			}
			for (int l=0;l<common_params.n_AP && l!=i;l++){
				for (int m=0;m<common_params.n_STA;m++){
			// ADD all STAs except itself
			STA_obj[i][j].add_listener((void*) &STA_obj[l][m],STA_obj[l][m].TYPE);
				}
			}
		}
	}
	cout<<"init done \n";
	// initialize jump
	int jump=1e8,jump1=0,jump2=0;;
	//cout<<"init done  \n";
	// Start simulation
	
	while (total_tti<run_ttis && stop_signal_called==false){ // run simulation for run_ttis TTIs
	
	for (int i=0;i<common_params.n_AP;i++){
	// check buffers of AP
	jump1=AP_obj[i].check_buffer();
	if(jump1<0)
	std::cout<<" jump AP is in error : "<<jump1<<std::endl;
	jump=jump>jump1?jump1:jump;
	// check buffers of STA
	for (int j=0;j<common_params.n_STA;j++){
	jump2=STA_obj[i][j].check_buffer();
	if(jump2<0)
	std::cout<<" jump STA in error: "<<jump2<<std::endl;
	jump = jump>jump2?jump2:jump;
		}
	}
	
	for (int i=0;i<common_params.n_AP;i++){
	// schedule and tx AP
	AP_obj[i].sched_tx();
	
	// schedule and tx STA
	for (int j=0;j<common_params.n_STA;j++){
	STA_obj[i][j].sched_tx();}
	}
	
	for (int i=0;i<common_params.n_AP;i++){
	// check tx time AP
	AP_obj[i].check_tx_time();
	// check tx time STA
	for (int j=0;j<common_params.n_STA;j++){
	STA_obj[i][j].check_tx_time();}
	}
	// Print the current time
	cout<<'\r';
	if(system_time%10000==0) // display time every 1s
	std::cout<<system_time*1e-6<<" secs";//<<std::endl;
	
	// increment system time
	system_time++;
	
	
	if(jump>0){
		for (int i=0;i<common_params.n_AP;i++){
		// jump at AP	
		AP_obj[i].jump_time(jump);
		// jump at STA
		for (int j=0;j<common_params.n_STA;j++){	
		STA_obj[i][j].jump_time(jump);}
		
		} 
		// jump system time
		system_time+=jump;	
	}
	
	} // end of while loop
	
	// results
	std::cout<<std::endl;
	// measured load
	cout<<"________________________________Results_________________________________________"<<endl;
	std::cout<<"Simulation Run time : "<<system_time*1e-6<<" seconds"<<std::endl;
	int total_ul_files =0;
	for (int j=0;j<common_params.n_AP;j++){
	for (int i=0;i<common_params.n_STA;i++){
		total_ul_files += STA_obj[j][i].total_files;}}
	int total_dl_files =0;
	for (int i=0;i<common_params.n_AP;i++){
		total_dl_files += AP_obj[i].total_files;}
	std::cout<<"Measured Load       : "<<((double)(total_ul_files+total_dl_files))*common_params.file_size/((double)system_time)<<" Mbps"<<std::endl;
	std::cout<<"Given load          : "<<(double) common_params.Network_load_bps*1e-6<<" Mbps"<<std::endl;
	double meas_dl_serv = 0;
	for (int i=0;i<common_params.n_AP;i++){
	meas_dl_serv += (double) AP_obj[i].served_data/((double)system_time);}
	double meas_ul_serv = 0;
	for (int j=0;j<common_params.n_AP;j++){
	for (int i=0;i<common_params.n_STA;i++){
	meas_ul_serv += (double) STA_obj[j][i].served_data/((double)system_time);}}
	std::cout<<"ToT served load     : "<<meas_dl_serv+meas_ul_serv<<" Mbps"<<std::endl;
	std::cout<<"DL served load      : "<<meas_dl_serv<<" Mbps"<<std::endl;
	std::cout<<"UL served load      : "<<meas_ul_serv<<" Mbps"<<std::endl;
	cout<<"________________________________END_________________________________________"<<endl;
	
}

void populate_params_nAP_nSTA_1cell() {
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
	// Total number of APs percell
	int n_AP = 3;
	common_params.n_AP = n_AP;
	// Total number of nodes per AP
	int n_STA = 6;
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
			* (Network_load_bps / (file_size * (n_STA) * n_AP)) * 1e-6;
	common_params.load_peruser_dl = load_per_user_dl;

	// Uplink per user load in files per micro second
	double load_per_user_ul = UL_percent
			* (Network_load_bps / (file_size * (n_STA) * n_AP)) * 1e-6;
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
