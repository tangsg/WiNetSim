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
#include <time.h>
#include <sys/time.h>
#include <fstream>

std::ofstream comp_log ;
void log(){}
#include "common.h"
#include "AP.h"
#include "STA.h"

std::mutex thread_safe_mtx;

bool stop_signal_called = false;

void sigint_handler(int code){
    (void)code;
    stop_signal_called = true;
}
//Function definitions
void populate_params_nAP_nSTA_ncell();
void Scene1_nAP_nSTA_ncell();
void indoor_9_cell_layout();
//global variables
long long system_time=0;
long long total_tti=0;
using namespace std;
long long run_ttis = 10000; // total transmissions
long long sim_run_time = 10*1e6; // total transmissions
parameters common_params;

int main()
{
	//open files
	comp_log.open("comp_log.txt", std::ofstream::out);
	signal(SIGINT, &sigint_handler);
	cout<<"### Press CTRL+C to end ### \n"<<endl;
	
	
	struct timeval tv0, tv1;    
	gettimeofday(&tv0, NULL);
	populate_params_nAP_nSTA_ncell();
	
	int NCell = 1;
	common_params.NCell = NCell;
	// Total number of APs percell
	int n_AP = 1;
	common_params.n_AP = n_AP;
	// Total number of nodes per AP
	int n_STA = 4;
	common_params.n_STA = n_STA;
	double length = 70;
	common_params.dimensions[0] = length;
	// Breadth of the room
	double breadth = 70;
	common_params.dimensions[1] = breadth;
	
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
	double Network_load_bps = 40 * 1e6;
	common_params.Network_load_bps = Network_load_bps;

	// Downlink per user load in files per micro second
	common_params.load_peruser_dl = DL_percent
			* (Network_load_bps / (file_size * (n_STA * NCell) * n_AP)) * 1e-6;

	// Uplink per user load in files per micro second
	common_params.load_peruser_ul = UL_percent
			* (Network_load_bps / (file_size * (n_STA * NCell) * n_AP)) * 1e-6;
	 
	print_params();
	indoor_9_cell_layout();
	//cout<<" init 9 cell \n"<<endl;
	Scene1_nAP_nSTA_ncell();
	gettimeofday(&tv1, NULL);
	//double elapsed;

	double elapsed = tv1.tv_sec - tv0.tv_sec;
	elapsed += (tv1.tv_usec - tv0.tv_usec) / 1e6;
	std::cout<<" time taken to run the simulation: "<<elapsed<<" seconds"<<std::endl;
	log(" time taken to run the simulation: ",elapsed," seconds\n");
	//close files
	comp_log.close();
	return 0;
}
