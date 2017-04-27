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
/*! \file common.h 
 * \brief Common supporting functions 
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */



#ifndef COMMON_H_ //gaurd
#define COMMON_H_
#include <string>
#include <list>
#include <iostream>
#include <utility>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <random>
#include <algorithm>
#include <csignal>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "channel_models.h"



//EVENTS
//! EVENT transmitting
#define TRANSMITTING 0
/*!
 * EVENT finished transmitting
*/
#define FIN_TRANSMITTING 1

struct point {
	double xy[2];
};

/*! \brief struct parameters: \n
 * Contain common simulation parameters \n
 */ 

struct parameters{
	/*! Total cells in the layout. \n
	 * A cell is the coverage site for an AP. \n
	 * Multiple cells have to be manually added. \n
	 */ 
	int NCell;
	/*! Total number of APs per cell. \n
	 * This is the total number of stations associated to each AP. \n
	 * if there are multiple cells and multiple APs in each cell. \n
	 * then total nodes in the network = NCell*n_wifi* no. of APs. \n
	 */
	int n_AP;
	/*! Total number of STA per AP. \n
	 * This is the total number of stations associated to each AP. \n
	 * if there are multiple cells and multiple APs in each cell. \n
	 * then total nodes in the network = NCell*n_wifi* no. of APs. \n
	 */
	int n_STA;
	
	/*! Total number of threads. \n
	 * Each thread corresponds to one node. \n
	 */
	int N_threads;
	
	/*! System BandWidth. \n
	 * Depending on the technology the BW changes. \n
	 * Currently we use only 20MHz. \n
	 * Since PHY is just abstraction multiple 20MHz can be suppoerted. \n
	 */
	double BW;
	/*! Dimension of a single cell. \n
	 * Here A cell is a room with lenghtxbreadth size. \n
	 * Length and breadth are stored in this array. \n
	 */
	double dimensions[2];
	/*! Height of the station. \n
	 * Height is the relative distance of the STA from ground. \n
	 */
	double height_STA;
	/*! Height of the AP. \n
	 * Height is the relative distance of the AP from ground. \n
	 */
	double height_AP;
	/*! Channel Model. \n
	 * This is the pathloss model of the channel, \n
	 * supported channels are listed in channel_models.h \n
	 */
	std::string channel_model;
	/*! DCF parameters \n
	 * Minimum contention window \n
	 * Typical values 16 or 32. \n
	 */ 
	int CW_min;
	/*! DCF parameters \n
	 * Maximum contention window \n
	 * Typical values 512 or 1024. \n
	 */ 
    int CW_max;
    /*! DCF parameters \n
	 * Wifi Slot duration in micro seconds \n
	 * Typical values 9 us. \n
	 */ 
	int wifi_slot;
	/*! DCF parameters \n
	 * DIFS duration in micro seconds \n
	 * Typical values 34 us. \n
	 */ 
    int DIFS_time;
    /*! DCF parameters \n
	 * Maximum transmit opportunity duration in micro seconds \n
	 * Typical values 4000 us or 10000 us. \n
	 * Depends on the regional regulations. \n
	 */ 
	long long MAX_TXOP;
	/*! load parameters \n
	 * DL to UL Ratio \n
	 */
	double DL_UL;
	/*! load parameters \n
	 * DL perecentage \n
	 */  
	double DL_percent;
	/*! load parameters \n
	 * UL perecentage \n
	 */  
	double UL_percent;
	/*! load parameters \n
	 * Network load in bits per second. \n
	 */
	double Network_load_bps;
	/*! load parameters \n
	 *  load per user in dl in files per micro second \n
	 */
	double load_peruser_dl;
	/*! load parameters \n
	 *  load per user in ul in files per micro second
	 */
	double load_peruser_ul;
	/*! Receiver noise floor \n
	 * measured in Watts per subcarrier \n
	 */
	double Noise;
	/*! AP transmit power \n
	 * measured in Watts per subcarrier \n
	 */
    double Pt_AP;
    /*! STA transmit power \n
	 * measured in Watts per subcarrier \n
	 */
    double Pt_STA;
    /*! Operating frequency \n
	 * measured in GHz \n
	 */
    int fc;
    /*! File Size \n
	 * measured in bits \n
	 * typical value 4e6 bits \n
	 */
    long long file_size;
    /*! Sensing threshold \n
     * measured in Watts per subcarrier \n
     * Typical values -62dBm or -82dBm over 20MHz\n
     * see the conversion from dBm to Watts \n
     */
    double WIFI_TH;
    /*! bits that can be scheduled in 20us channel time \n
     * 20us is 4 symbols in wifi which chosen as the minimum granularity \n
     * no of bits per mcs for 20us is stored in the vector \n
     */
    std::vector <double> WIFI_data_20us;
    /*! Total MCS available \n
     * typical value 8 or 9 \n
     */
    int MAX_MCS;    
};

bool is_inside_rectangle(double dimension[2], double coord[2]);
void print_params();
bool find_prb_collision(double SINR_db, int MCS_index);
double Monotone_cubic_spline_interpolation(std::vector<double> *SNR_LUT,std::vector<double> *BLER_LUT, double SNR_db);
//! error logs

//! SINR logs

//! complete log

extern std::ofstream comp_log ;
template <typename First, typename ...Rest>
void log(First && first, Rest && ...rest)
{	
	if(!comp_log.is_open()){
	comp_log.open("comp_log.txt", std::ofstream::out | std::ofstream::app);
	}
    comp_log << std::forward<First>(first);
    log(std::forward<Rest>(rest)...);
    //if(comp_log.is_open()){
	//comp_log.close();
	//}
}
#endif
