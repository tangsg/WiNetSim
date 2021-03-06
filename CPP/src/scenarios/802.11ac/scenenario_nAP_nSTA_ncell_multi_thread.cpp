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
/*! \file scenario_nAP_nSTA_ncell_multi_threaded.cpp
 * \brief ncell nAp nSTA room layout 
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#include "common.h"
#include "AP.h"
#include "STA.h"

/*! Simple n AP n STA n cell multi threaded\n
 *  InH model with a room layout \n
 */ 
 //global variables
extern bool stop_signal_called;
extern long long system_time;
extern long long total_tti;
extern long long sim_run_time;
extern parameters common_params;
extern long long run_ttis; // total transmissions

extern point ****STA_coord;
extern point ***AP_coord;

// threading variables
int jump=0;
int *jump_thread;
bool *start;
int sync_total=0;

std::mutex sync_mtx;
std::condition_variable cv;
std::condition_variable av;

void thread_function_AP(int i,int j,int tid);
void thread_function_STA(int i,int j, int k,int tid);

AP **AP_obj;// = new AP*[common_params.NCell];
STA ***STA_obj;// = new STA**[common_params.NCell];

using namespace std;

void Scene1_nAP_nSTA_ncell_multi_thread()
{
	// init uniform real distibution between [0,1]
	std::uniform_real_distribution<double> uni_distribution(-0.5, 0.5);
	// set random device generator
	std::random_device generator;
	bool inside;
	// AP object
	AP_obj = new AP*[common_params.NCell];
	for (int i=0;i<common_params.NCell;i++){
		AP_obj[i] = new AP[common_params.n_AP];
	}
	// AP coordinate
	
	for (int i=0;i<common_params.NCell;i++)
	{
		for (int j=0; j<common_params.n_AP;j++){
		// initialize the AP objects
		AP_obj[i][j].AP_init(j,AP_coord[i][j]->xy,common_params);
		//ADD all the APs to the current AP listener group
		for (int k=0;k<common_params.NCell;k++){
			for (int l=0;l<common_params.n_AP;l++){
						if(k==i && l==j){}
						// Dont add it
						else
						AP_obj[i][j].add_listener((void*) &AP_obj[k][l],AP_obj[k][l].TYPE);
					}
			}
		}
	}
	
	// STA object
	STA_obj = new STA**[common_params.NCell];
	for (int i=0;i<common_params.NCell;i++)
	{
		STA_obj[i] = new STA*[common_params.n_AP];
		for (int j=0;j<common_params.n_AP;j++){
			STA_obj[i][j] = new STA[common_params.n_STA];
		}
	}
	
	for (int i=0;i<common_params.NCell;i++){
	for (int j=0;j<common_params.n_AP;j++){
	for (int k=0;k<common_params.n_STA;k++)
	{
		// initialize the STA objects
		STA_obj[i][j][k].STA_init(k,STA_coord[i][j][k]->xy,common_params,(void *)&AP_obj[i][j]);
		// Associate STA to AP
		AP_obj[i][j].ADD_STA((void *) &STA_obj[i][j][k]);
		// ADD all APs to current STA listener group
		for (int l=0;l<common_params.NCell;l++){
			for (int m=0;m<common_params.n_AP;m++){
				STA_obj[i][j][k].add_listener((void*) &AP_obj[l][m],AP_obj[l][m].TYPE);
				// ADD current STA to all AP listener group
				AP_obj[l][m].add_listener((void*) &STA_obj[i][j][k],STA_obj[i][j][k].TYPE);
					}
				}
			}
		
		}
	}
	
	// ADD all STAs to each STA listener group
	for (int i=0;i<common_params.NCell;i++){
		for (int j=0;j<common_params.n_AP;j++){
			for (int k=0;k<common_params.n_STA;k++){
			//All the STAs to this STA listener group
			
			for (int l=0;l<common_params.NCell;l++){
				for (int m=0;m<common_params.n_AP;m++){
					for (int n=0;n<common_params.n_STA;n++){
						//ADD all stas except itself
						
						if(i==l && j==m && k==n){}
						// do nothing
						else
						STA_obj[i][j][k].add_listener((void*) &STA_obj[l][m][n],STA_obj[l][m][n].TYPE);
						}
					}
				}
			}
		}
	}
	int ttis_sofar =0;
	int temp_var [common_params.N_threads];
	jump_thread = temp_var; 
	bool start_temp[common_params.N_threads];
	start = start_temp;
	for (int i=0; i<common_params.N_threads; i++)
	start[i] = false;
	
	int threads_AP_tot = common_params.NCell* common_params.n_AP;
	int threads_STA_tot = common_params.NCell* common_params.n_AP* common_params.n_STA; 
	std::thread threads_AP[common_params.NCell][common_params.n_AP];
	std::thread threads_STA[common_params.NCell][common_params.n_AP][common_params.n_STA];
	int t_id=0;
	for (int i=0; i<common_params.NCell; i++){
		for (int j=0; j<common_params.n_AP; j++){
			threads_AP[i][j] = std::thread(thread_function_AP,i,j,t_id++);
			for (int k=0; k<common_params.n_STA; k++){
				threads_STA[i][j][k] = std::thread(thread_function_STA,i,j,k,t_id++);
			}
		}
	}
	
	//while (ttis_sofar<run_ttis && stop_signal_called==false){
	while (system_time<sim_run_time && stop_signal_called==false){
	std::unique_lock<std::mutex> lck(sync_mtx);	
	while (sync_total!=(threads_AP_tot+threads_STA_tot))
		av.wait(lck);
		lck.unlock();
	sync_mtx.lock();
	system_time++;
	jump = jump_thread[0]; 
	for (int i=1;i<t_id;i++){
		jump = jump_thread[i]<jump?jump_thread[i]:jump;
	}
	if(jump>0){
	system_time+=jump;}	
	if(system_time%100==0) // display time every 1s
	{std::cout<<'\r';
	std::cout<<std::fixed<<std::setprecision(4)<<system_time*1e-6<<" sec time elapsed TTIs: "<<std::setprecision(4)<<total_tti;}//<<std::endl;
	ttis_sofar = total_tti;
	sync_total=0;
	for (int i=0; i<common_params.N_threads; i++)
	start[i] = true;
	cv.notify_all();
	sync_mtx.unlock();
	
	
	}
	// epilogue
	std::cout<<" \n epilogue \n"<<std::endl;
	stop_signal_called=true;
	sync_mtx.lock();
	for (int i=0; i<common_params.N_threads; i++)
	start[i] = true;
	cv.notify_all();
	sync_mtx.unlock();
	
	std::cout<<" joining threads \n"<<std::endl;
	// end of the simulation
	for (int i=0; i<common_params.NCell; i++){
		for (int j=0; j<common_params.n_AP; j++){
			threads_AP[i][j].join();
			for (int k=0; k<common_params.n_STA; k++){
				threads_STA[i][j][k].join();
			}
		}
	}
	std::cout<<" ALL threads joined \n"<<std::endl;
	// results
	std::cout<<std::endl;
	// measured load
	cout<<"________________________________Results_________________________________________"<<endl;
	std::cout<<"Simulation Run time : "<<system_time*1e-6<<" seconds"<<std::endl;
	int total_ul_files =0;
	for (int i=0;i<common_params.NCell;i++){
	for (int j=0;j<common_params.n_AP;j++){
	for (int k=0;k<common_params.n_STA;k++){
		total_ul_files += STA_obj[i][j][k].total_files;}}}
	int total_dl_files =0;
	for (int i=0;i<common_params.NCell;i++){
	for (int j=0;j<common_params.n_AP;j++){
		total_dl_files += AP_obj[i][j].total_files;}}
	std::cout<<"Measured Load       : "<<((double)(total_ul_files+total_dl_files))*common_params.file_size/((double)system_time)<<" Mbps"<<std::endl;
	std::cout<<"Given load          : "<<(double) common_params.Network_load_bps*1e-6<<" Mbps"<<std::endl;
	double meas_dl_serv = 0;
	for (int i=0;i<common_params.NCell;i++){
	for (int j=0;j<common_params.n_AP;j++){
	meas_dl_serv += (double) AP_obj[i][j].served_data/((double)system_time);}}
	double meas_ul_serv = 0;
	for (int i=0;i<common_params.NCell;i++){
	for (int j=0;j<common_params.n_AP;j++){
	for (int k=0;k<common_params.n_STA;k++){
	meas_ul_serv += (double) STA_obj[i][j][k].served_data/((double)system_time);}}}
	std::cout<<"ToT served load     : "<<meas_dl_serv+meas_ul_serv<<" Mbps"<<std::endl;
	std::cout<<"DL served load      : "<<meas_dl_serv<<" Mbps"<<std::endl;
	std::cout<<"UL served load      : "<<meas_ul_serv<<" Mbps"<<std::endl;
	cout<<"________________________________END_________________________________________"<<endl;
	
}

void thread_function_AP(int i,int j, int tid){
	
	
	while(stop_signal_called==false){
	
	// start the operations
	jump_thread[tid]=AP_obj[i][j].check_buffer();
	AP_obj[i][j].sched_tx();
	AP_obj[i][j].check_tx_time();
	// sync mechanism	
	sync_mtx.lock();
	sync_total++;
	if(sync_total==common_params.N_threads){
	av.notify_one();}
	
	sync_mtx.unlock();
	
	//wait for sync
	std::unique_lock<std::mutex> lck(sync_mtx);
	while(start[tid]==false)
	cv.wait(lck);
	lck.unlock();
	
	start[tid] = false;	
	if(jump>0)
	AP_obj[i][j].jump_time(jump);	
	}
	sync_mtx.lock();
	sync_total=common_params.N_threads;
	av.notify_one();
	sync_mtx.unlock();
}

void thread_function_STA(int i,int j, int k,int tid){
	
	while(stop_signal_called==false){
	
	// start the operations
	jump_thread[tid]=STA_obj[i][j][k].check_buffer();
	STA_obj[i][j][k].sched_tx();
	STA_obj[i][j][k].check_tx_time();
	// sync mechanism	
	sync_mtx.lock();
	sync_total++;
	if(sync_total==common_params.N_threads){
	av.notify_one();}
	
	sync_mtx.unlock();
	
	//wait for sync
	std::unique_lock<std::mutex> lck(sync_mtx);
	while(start[tid]==false)
	cv.wait(lck);
	lck.unlock();
	
	start[tid] = false;	
	if(jump>0)
	STA_obj[i][j][k].jump_time(jump);
	}
	sync_mtx.lock();
	sync_total=common_params.N_threads;
	av.notify_one();
	sync_mtx.unlock();
}

void populate_params_nAP_nSTA_ncell_multi_thread() {
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
	int n_AP = 2;
	common_params.n_AP = n_AP;
	// Total number of nodes per AP
	int n_STA = 3;
	common_params.n_STA = n_STA;
	// N threads
	int N_threads = (NCell*n_AP*n_STA)+(NCell*n_AP);
	common_params.N_threads = N_threads;
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
	long long file_size = 1 * 1e6;
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
			* (Network_load_bps / (file_size * (n_STA * NCell) * n_AP)) * 1e-6;
	common_params.load_peruser_dl = load_per_user_dl;

	// Uplink per user load in files per micro second
	double load_per_user_ul = UL_percent
			* (Network_load_bps / (file_size * (n_STA * NCell) * n_AP)) * 1e-6;
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
