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
/*! \file AP.h 
 * \brief AP class 
 * \author sreekanth dama <sreekanth@iith.ac.in>
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#ifndef AP_H_   /* Include guard */
#define AP_H_
#include "common.h"

/*! \brief class AP IEEE 802.11ac access point\n 
 * Implements 802.11 DCF \n
 * PHY is completely Abstracted \n
 * Access Point node contains a traffic generator + lower MAC + PHY abstraction \n 
 */
class AP {
	//! private data
private:

	//! minimum contention window
	int CW_min;
	//! maximum contention wndow
	int CW_max;
	//! counter value
	int Counter;
	//! contention status indicator 0 if not contending 1 if contending
	bool contending = 0;
	//! Wifi Slot duratuion
	int wifi_slot;
	//! WiFi DIFS Time
	int DIFS_time;
	//! DIFS status indicator 0 if DIFS Done 1 if not
	bool DIFS_Ind = 0;
	//! DIFS counter
	int DIFS_counter = 0;
	//! Maximum transmit opportunity in micro seconds
	int MAX_TXOP;

	//! Energy measured in 1 micro second \n
	//!  This is used for Energy detection \n
	//!  TO incorporate Preamble detection SINR must be used \n
	double Energy_meas = 0;
	//! Signal power from STA to AP
	double Sig_power = 0;
	//! Interference power accumulated over the transmission duration
	double Int_power = 0;
	//! load per user in DL measured in packets per 1 micro second
	double load_peruser;
	//! Vector to hold the next packet arrival time for all the users in downlink
	std::vector<long long> Time_next_file_arrives;
	//! vector that holds the object handles of all the connected users
	std::vector<void *> BS_UEs;
	//! Total number of the connected users
	int Connected_UEs;
	//! time snapshot gives you the time last interference changed
	long long local_time = 0;
	//! Thermal noise on a subcarrier
	double Noise;
	//! Receive antenna gain
	double RX_GAIN = 1;
	//! Operating frequency
	int fc;
	//! Channel Model
	std::string channel_type;
	//! Packet Size in bits
	long long file_size;
	//! Energy detect threshold in Watts measured over 1 subcarrier
	double WIFI_TH;
	//! bits transmitted in 20micro seconds for all the MCS
	std::vector<double> WIFI_data_20us;
	//! random number generator
	//std::default_random_engine generator;
	std::random_device generator;

public:
	//! Device ID and Device TYPE combined will give a unique ID to the device. \n
	//! So there is no need for a physical MAC Address 
	int ID; /*! Device ID */
	//! cell site ID
	int cell_site_ID;
	//! Device Type
	std::string TYPE = "AP";
	//! AP height from the ground
	double height;
	//! AP coordintes [x,y]
	double coordinates[2];
	//! Vector for the DL MCS corresponding each STA
	std::vector<int> MCS_DL;
	//! Total MCS 
	int MAX_MCS;
	//! Current STA being served in DL
	int CURRENT_STA_DL = 0;
	//! Current STA doing UL
	int CURRENT_STA_UL;
	//! Vector containing the distance of all the Active devices int the network
	std::vector<double> ACTIVE_Devices;
	//! Vector containing the tx power of all the Active devices int the network
	std::vector<double> ACTIVE_Devices_power;
	/*! Data allocated in downlink for each STA
	 * This is necessary as there is no explicit signalling of control information
	 */
	std::vector<long long> ALLOC_data_DL;
	/*! Data allocated in uplink from each STA
	 * This is necessary as there is no explicit signalling of control information
	 */
	std::vector<long long> ALLOC_data_UL;
	//! Transmission status indicator
	//! 0 if not tx 1 if tx
	bool TX_ind = 0;
	//! Reception status indicator
	//! 0 if not rx 1 if rx
	bool RX_ind = 0;
	//! Transmit power of the AP in Watts measured over 1 subcarrier
	double Pt_W;
	//! vector of list of files per user: mimicks the packet que 
	std::vector<std::list<long long>> Buffer_DL;
	//! vector containing the total data per STA yet to be tranmistted in DL
	std::vector<long long> SUM_BUFF_DL;
	//! Total files yet to be transmitted 
	//! 0 if no data !! >0 if data counts currnt files
	long long BUF_Ind = 0;
	//! Total data yet to be transmitted
	long long TOT_BUFF = 0;
	//! channel occupy time for transmission
	long long Channel_occupy_time = 0;
	//! vector of list of time a file arrived per user: complements Buffer DL
	std::vector<std::list<long long>> Time_file_arrived_DL;
	//! vector containing Time in micro seconds took to delive a file in DL used for STAISTICS
	std::vector<long long> TTTDeliverOFile_DL;
	//! vector containing SINR in dB in DL for each transmission used for STAISTICS
	std::vector<double> SINR;
	//! total files received used for STATISTICS 
	long long total_files = 0;
	//! Total served data in DL used for STATISTICS
	long long served_data = 0;
	//! Total data pumped into the channel in DL used for STATISTICS
	long long pumped_data = 0;
	//! Total attempts : Used for MCS adaptation
	std::vector<long long> attempt;
	//! Total success : Used for MCS adaptation
	std::vector<long long> success;
	//! current contention window size
	int CW;
	//! Event listeners
	//! vector containing the station object handles
	std::vector<void *> listener_STA;
	//! vector containing the access point object handles
	std::vector<void *> listener_AP;

	
	//! Constructor
	AP();

	/*!initilaizer function \n
	 * Initializes the object after its creation
	 * \param ID 
	 * \param coordinates
	 * \param parameters
	 */
	void AP_init(int ID, double coordinates[2], parameters params);

	/*! Associate STA to the AP \n
	 * Adds the STA object handle to the vector BS_UEs \n
	 * \param obj STA object handle
	 */
	void ADD_STA(void* obj);

	/*!
	 * used to do a time jump is all the nodes in the network dont have data to tx \n
	 * \param jump integer 
	 */
	void jump_time(int jump);

	/*! check the buffers for packet arrival \n
	 * If there is a packet arrived then starts contending for channel \n
	 * Does energy measurement for 1 micro second \n
	 */
	int check_buffer();

	/*! sched and transmit \n
	 * Based on the energy measuremnt from previous function
	 * Clear channel assessment is done \n
	 * based on CCA DCF procedures are done. \n
	 * If AP gets the channel then it schedules a UE in round robin fashion and transmits \n
	 */
	void sched_tx();
	
	/*! check tx duration and decrement channel occupancy time \n
	 * when transmission is over notify all the listening nodes \n
	 */
	void check_tx_time();

	/*!Callback function for event TRANSMITTING \n
	 * When a device transmits and notifys the listeners \n
	 * If this device is a part of the listeners group then
	 * this call back function will be called if the event is TRANSMITTING \n
	 * This function includes: \n
	 * 1. Checking if the calling object is connected to the AP \n
	 * 2. IF so then if AP is not already in reception mode then start receiving from the STA \n
	 * 3. If the calling object is not associated with AP the it is put in the interferers list \n
	 * 4. If the AP is already in reception mode then the interference is calculated until this new interferer came \n
	 * 
	 * \param obj calling object
	 * \param type TYPE of the calling object 
	 */
	void callback_tx(void* obj, std::string type);

	/*!Callback function for finished tx \n
	 * When a device finishes transmission and notifys the listeners \n
	 * If this device is a part of the listeners then \n
	 * this call back function will be called if the event is FIN_TRANSMITTING \n
	 * This function includes \n
	 * 1. Checking if the calling object is connected to the AP \n
	 * 2. IF so then if AP is already in reception then stops receiving from the STA if it was already receiving from the same STA \n
	 * 3. If the calling object is not associated with AP the it is removed from the interferers list \n
	 * 4. If the AP is already in reception mode then the interference is calculated until this interferer left \n
	 * \param obj calling object
	 * \param type TYPE of the calling object 
	 */
	void callback_fintx(void* obj, std::string type);

	/*!framework support functions \n
	 * This function adds a STA / AP to the listeners group \n
	 * \param obj object handle
	 * \param type TYPE of the object
	 */
	void add_listener(void* obj, std::string type);

   /*! Notify the listeners
	* This function calls the callback functions of the listeners based on the event registered
	* \param EVENT TRANSMITTING or FIN_TRANSMITTING defined in common.h
	*/
	void notify_listener(int EVENT);

};

#endif
