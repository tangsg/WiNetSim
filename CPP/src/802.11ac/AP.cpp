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
/*! \file AP.cpp
 * \brief AP class source 
 * \author sreekanth dama sreekanth@iith.ac.in
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
 
extern void log();
#include "common.h"
#include "AP.h"
#include "STA.h"

extern std::ofstream comp_log ;
// global simulation clock in us  
extern long long system_time;
// TTIs
extern long long total_tti;
extern std::mutex thread_safe_mtx;

// Constructor
AP::AP() {
}

void AP::AP_init(int ID_in, double coordinates_in[2], parameters params) {
	
	// Unique number to identify the AP
	ID = ID_in;
	// X coordinate
	coordinates[0] = coordinates_in[0];
	// Y coordinate
	coordinates[1] = coordinates_in[1];
	
	// Height of the AP from the ground
	height = params.height_AP;
	
	// Minimum Contention window size of DCF
	CW_min = params.CW_min;
	// Maximum Contention window size of DCF
	CW_max = params.CW_max;
	
	// slot duration of DCF
	wifi_slot = params.wifi_slot;
	// DIFS duration
	DIFS_time = params.DIFS_time;
	// Maximum transmit opportunity in micro seconds
	MAX_TXOP = params.MAX_TXOP;
	
	// Downlink load per user in packets per micro second
	load_peruser = params.load_peruser_dl;
	// Total number of STAs associated to this AP
	Connected_UEs = params.n_STA;
	// Noise floor of the receiver in Watts measured over 1 subcarrier
	Noise = params.Noise;
	// Transmit power of the AP in Watts measured over 1 subcarrier
	Pt_W = params.Pt_AP;
	// Carrier frequency used for Pathloss measurements
	fc = params.fc;
	// Channel Model
	channel_type = params.channel_model;
	// Packet size at the application 
	file_size = params.file_size;
	// Wifi Energy Detect Sensing threshold
	WIFI_TH = params.WIFI_TH;
	// Maximum MCS value
	MAX_MCS = params.MAX_MCS;
	// Set the MCS (MAX) for DL transmission for all the STAs
	MCS_DL.insert(MCS_DL.begin(), Connected_UEs, MAX_MCS - 1);
	// bits transmitted in 20micro seconds for each MCS
	WIFI_data_20us.insert(WIFI_data_20us.begin(), &params.WIFI_data_20us[0],
			&params.WIFI_data_20us[MAX_MCS]);
	/* Data allocated in downlink for each STA
	 * This is necessary as there is no explicit signalling of control information
	 */
	ALLOC_data_DL.insert(ALLOC_data_DL.begin(), Connected_UEs, 0);
	/* Data allocated in uplink from each STA
	 * This is necessary as there is no explicit signalling of control information
	 */
	ALLOC_data_UL.insert(ALLOC_data_UL.begin(), Connected_UEs, 0);
	// Exponential random variable initializer
	std::exponential_distribution<double> exp_distribution(load_peruser);
	// Setting up the buffers and initializing the time first packet arrives
	for (int i = 0; i < Connected_UEs; i++) {
		// create a new linked list for Data buffering for each STA
		std::list<long long> *temp0 = new std::list<long long>;
		// push the list into the Buffer vector
		Buffer_DL.push_back(*temp0);
		// create a new linked list for time file arrived for each STA
		std::list<long long> *temp1 = new std::list<long long>;
		// push the list into the Time file arrived vector
		Time_file_arrived_DL.push_back(*temp1);
		// Find the first packet arrival for a user in DL
		int time_temp = (long long) std::round(exp_distribution(generator));
		// push the time first packet arrives to the vector
		Time_next_file_arrives.push_back(time_temp);
	}
	// Initialize attemp
	attempt.insert(attempt.begin(), Connected_UEs, 0);
	// Initialize success
	success.insert(success.begin(), Connected_UEs, 0);
	// initializ the sum of data for all STA to zero.
	SUM_BUFF_DL.insert(SUM_BUFF_DL.begin(), Connected_UEs, 0);
	// Set the curtrent contention window to the minimum
	CW = CW_min;
	
	//print
	/*
	log("______________________________________________\n");
	log(" AP: ",ID," Parameters \n");
	log("______________________________________________\n");
	log(" coord ",coordinates[0],',',coordinates[1],"\n");
	log(" height ",height,"\n");
	log(" CWmin: ",CW_min," CWmax: ",CW_max," CW: ",CW,"\n");
	log(" slot: ",wifi_slot," DIFS: ",DIFS_time," MAXTXOP: ",MAX_TXOP,"\n");
	log(" load_peruser: ",load_peruser," Connected_UEs: ",Connected_UEs," Noise: ",Noise,"\n");
	log(" Pt_W: ",Pt_W," fc: ",fc," channel_type: ",channel_type,"\n");
	log(" file_size: ",file_size," WIFI_TH: ",WIFI_TH," MAX_MCS: ",MAX_MCS,"\n");
	for (int i=0;i<Connected_UEs;i++){
		log(" STA:",i," MCS_DL:",MCS_DL[i]," attempt:",attempt[i],
		" success:",success[i]," TNFA:",Time_next_file_arrives[i]
		," sumbuff_dl:",SUM_BUFF_DL[i],'\n');}
	for (int i=0;i<MAX_MCS;i++){
		log(" WIFI_data_20us[:",i,"] ",WIFI_data_20us[i],"\n");}	
	log("______________________________________________\n");
	*/
}

/* Associate STA to the AP
 * Adds the STA object handle to the vector BS_UEs
 */
void AP::ADD_STA(void* obj) {
	// Push STA handle into the vector
	BS_UEs.push_back(obj);
}

/* 
 * jump time if no data available at all the nodes
 */
void AP::jump_time(int jump) {
	// Substract jump time from the time next file arrives
	for (auto it = Time_next_file_arrives.begin();
			it != Time_next_file_arrives.end(); ++it) {
		(*it) -= jump;
	}
}

/* check the buffers for packet arrival
 * If there is a packet arrived then starts contending
 * Does energy measurement for 1 micro second
 */
int AP::check_buffer() {
	int jump = 0, i = 0;
	// exponential distribution for load
	std::exponential_distribution<double> exp_distribution(load_peruser);
	// Decrement time next file arrives
	
	for (auto it = Time_next_file_arrives.begin();
			it != Time_next_file_arrives.end(); ++it) {
		(*it) -= 1;
		if (*it <= 0) { // Check if the file arrived
			log('[',system_time,']'," AP: ",ID);
			// push the time the file arrived
			Time_file_arrived_DL[i].push_back(system_time);
			// Find the next packet arrival time
			Time_next_file_arrives[i] = (long long) std::round(
					exp_distribution(generator));
			// Push the arrived file into the buffer
			Buffer_DL[i].push_back(file_size);
			// Sum the file size to sum buff buffer size
			SUM_BUFF_DL[i] += file_size;
			// Increment the STATISTIC total files arrived 
			total_files += 1;
			// Increment buffer indicator
			BUF_Ind += 1;
			// Sum the file size to total buffer size
			TOT_BUFF += file_size;
			log(" STA: ",i," buff_size ",Buffer_DL[i].size(),
		" total files ",total_files," Buf Ind ",BUF_Ind," Tot buff ",
		TOT_BUFF,'\n');
		}
		i++; // Next user
	}
	// update jump with the time next file arrives		
	jump = Time_next_file_arrives[0];
	// find the minimum among all the users buffer
	for (auto it = Time_next_file_arrives.begin();
			it != Time_next_file_arrives.end(); ++it) {
		jump = jump < *it ? jump : *it;
	}

	if (contending == false && TX_ind == 0 && RX_ind == 0) {
		// if not TX and not Contending and not RX
		// then check the buffer and if there is data start contending
		if (BUF_Ind > 0) {
			// As there is data start contending
			contending = true;
			// set jump to zero as there is data to tx
			jump = 0;
			// set a uniform distribution to get values between [0 CW-1]
			std::uniform_int_distribution<int> uni_distribution(0, CW - 1);
			// get a random number from uniform distribution and multiply it with wifi_slot to get time in us
			Counter = uni_distribution(generator) * wifi_slot;
			// Set DIFS_Ind to zero as this is the start of DCF procedure
			DIFS_Ind = false;
			// Set the DIFS Counter to DIFS duration
			DIFS_counter = DIFS_time;
			// Initialize energy measurement to thermal noise
			Energy_meas = Noise;
			// Measure the energy from all interferers
			if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
				// initialize a vector pointer to measure PL
				std::vector<double> *PL = new std::vector<double>;
				// Measure PL from all the interferer sources
				PL_measure(ACTIVE_Devices, PL, fc, channel_type); // PL is in dB
				for (auto it = PL->begin(); it != PL->end(); ++it) {
					// Sum all the received power from all the sources where rx power is tx power*PL
					Energy_meas = Energy_meas + Pt_W * std::pow(10, *it / 10);
				} // end of for
			} // end of if
			log('[',system_time,']'," AP: ",ID," Energy_meas ",Energy_meas," Noise ",Noise,
	  " contending ",contending," TX_ind ",TX_ind," RX_ind ",RX_ind,
	  " Counter ",Counter," DIFS_Ind ",DIFS_Ind," DIFS_counter ",
	  DIFS_counter," Active ",ACTIVE_Devices.size(),'\n');	
		} // end of if
	} // end of if
	else if (contending == true) {
		// if contending then do energy measurements
		// set jump to 0 
		jump = 0;
		// Initialize energy measurement to thermal noise
		Energy_meas = Noise;
		// Measure the energy from all interferers
		if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
			// initialize a vector pointer to measure PL
			std::vector<double> *PL = new std::vector<double>;
			// Measure PL from all the interferer sources
			PL_measure(ACTIVE_Devices, PL, fc, channel_type); // PL is in dB
			for (auto it = PL->begin(); it != PL->end(); ++it) {
				// Sum all the received power from all the sources where rx power is tx power*PL
				Energy_meas = Energy_meas + Pt_W * std::pow(10, *it / 10);
			} // end of for
			delete PL;
		} // end of if
	} // end of if else
	else {
		// might be receiving might be transmitting
		// set the jump to zero
		jump = 0;
	} // end of else
	  // return time jump value
	return jump;
}

/* sched and transmit
 * Based on the energy measuremnt from previous function
 * Clear channel assessment is done
 * based on CCA DCF procedures are done.
 * If AP gets the channel then ot schedules a UE in round robin fashion and transmits
 */
void AP::sched_tx() {
	// DO DCF if contending
	if (contending == true) {
		// See if DIFS is done
		if (DIFS_Ind == true) {
			// check if the measured enrgy is less than WIFI sensing threshold
			if (Energy_meas > WIFI_TH) {
				// If energy meas id > sens thresh then bring the counter to multiple of wifi slots
				Counter = Counter - (Counter % wifi_slot) + wifi_slot;
				// Set DIFS Ind to 0
				DIFS_Ind = false;
				// Set the DIFS counter
				DIFS_counter = DIFS_time;
			} // end of if energy condition
			else
				// Decrement the counter by 1 us If energy meas is less than sensing threshold
				Counter = Counter - 1;
			// Check whether the counter is zero
			if (Counter <= 0) {
				// If counter is zero
				// set contending to false
				contending = false;
				// Set DIFS indicator to zero
				DIFS_Ind = false;
				// Set DIFS counter
				DIFS_counter = DIFS_time;
				// Set TX indicater to 1
				TX_ind = 1;
				// Start Scheduling
				// Set channel occupy time to zero;
				Channel_occupy_time = 0;
				// Do round Robin
				if (CURRENT_STA_DL == Connected_UEs - 1) { // if current UE is last one pick the next one
					CURRENT_STA_DL = 0; // pick the first UE
				} else {
					CURRENT_STA_DL += 1; // pick the next UE
				} // end of else
				  // schedule the UE
				while (Channel_occupy_time == 0) {
					// check if the current UE has data
					if (SUM_BUFF_DL[CURRENT_STA_DL] > 0) {
						// increment the attempt
						attempt[CURRENT_STA_DL] = attempt[CURRENT_STA_DL] + 1;
						// check the time required
						long long time =
								std::ceil(
										(double) SUM_BUFF_DL[CURRENT_STA_DL]
												/ WIFI_data_20us[MCS_DL[CURRENT_STA_DL]]);
						// check whether the time required is less than the MAX tx opportunity
						if (20 * time < MAX_TXOP) { // 20us is the minimum granularity
							// update the channel occupy time
							Channel_occupy_time = 20 * time;
							// Update Allocated data
							ALLOC_data_DL[CURRENT_STA_DL] =
									SUM_BUFF_DL[CURRENT_STA_DL];
							// update the pumped data
							pumped_data += SUM_BUFF_DL[CURRENT_STA_DL];
						} else {
							// occupy the channel for max tx op
							Channel_occupy_time = MAX_TXOP;
							// update Allocated data
							ALLOC_data_DL[CURRENT_STA_DL] =
									WIFI_data_20us[MCS_DL[CURRENT_STA_DL]]
											* MAX_TXOP / 20;
							// update pumped data
							pumped_data +=
									WIFI_data_20us[MCS_DL[CURRENT_STA_DL]]
											* MAX_TXOP / 20;
						} // end of else
					} else {
						// Do round Robin
						// if current UE is last one pick the next one
						if (CURRENT_STA_DL == Connected_UEs - 1)
							CURRENT_STA_DL = 0; // pick the first UE
						else
							CURRENT_STA_DL += 1; // pick the next UE
					} // end of else
				} // end of inf while
				log('[',system_time,']'," AP: ",ID," MCS_DL "
				,MCS_DL[CURRENT_STA_DL], "CURRENT_STA_DL ",CURRENT_STA_DL, "occupy time ",
				Channel_occupy_time,'\n');
				  // Notify the listening group about this transmission
				notify_listener(TRANSMITTING);
			} // end of counter zero
		} else if (Energy_meas < WIFI_TH) { // if measured energy is less than the WIFI sensing threshold (1us)
			// Decrement DIFS counter by 1us
			DIFS_counter = DIFS_counter - 1;
			// Check if the DIFS counter is zero
			if (DIFS_counter <= 0) {
				// Indicate that DIFS is done
				DIFS_Ind = 1;
				// initialize the DIFS counter (Redundant but to be sure)
				DIFS_counter = DIFS_time;
			} // end of if
		} // end of enregy meas
	} // End of if contending
} // End of function

/* check tx duration and decrement channel occupancy time
 * when transmission is over notify all the listening nodes
 */
void AP::check_tx_time() {
	// check if STA is tranmitting
	if (TX_ind == 1) {
		// Decrement the channel occupy time
		Channel_occupy_time = Channel_occupy_time - 1;
		// check if channel occupy time is zero
		if (Channel_occupy_time <= 0) {
			// set tx ind to 0
			TX_ind = 0;
			// notify the listener group about finished transmission
			notify_listener(FIN_TRANSMITTING);
		} // end of chanel occupy cond
	} // end of tx ind condition
} // end of function

/*Callback function for tx
 * When a device transmits and notifys the listeners
 * If this device is a part of the listeners then
 * this call back function will be called if the event is TRANSMITTING
 * This function includes
 * 1. Checking if the calling object is connected to the AP
 * 2. IF so then if AP is not already in reception then starts receiving from the STA
 * 3. If the calling object is not associated with AP the it is put in the interferers list
 * 4. If the AP is already in reception mode then the interference is calculated until the new interferer came
 * 
 * INPUT: calling object (obj), type of the calling object (obj.TYPE) 
 */
void AP::callback_tx(void* obj, std::string type) {
	// temperoray variable
	bool temp = 0;

	if (type == "AP")
		// is EVENT from AP
		temp = 0;
	if (type == "STA") {
		// check if the STA is associated with AP
		for (auto it = BS_UEs.begin(); it != BS_UEs.end(); ++it) {
			if (obj == *it) {
				temp = 1;
				break;
			}
		} // end of for
	} // end if
	if (temp && TX_ind != 1 && RX_ind != 1) {
		// if the EVENT is from connected STA and AP is not receiving not transmitting
		// Start receiving
		RX_ind = 1;
		// stop contending
		contending = 0;
		// record the STA ID
		CURRENT_STA_UL = ((STA*) obj)->ID;
		// Mark the time the transmission started
		local_time = system_time;
		// init sig power
		Sig_power = 0;
		// init interferer power
		Int_power = 0;
		// Record the channel occupy time
		thread_safe_mtx.lock(); // thread safe
		Channel_occupy_time = ((STA*) obj)->Channel_occupy_time;
		log('[',system_time,']'," AP: ",ID," receiving from ",((STA*)obj)->TYPE,((STA*)obj)->ID,'\n');
		thread_safe_mtx.unlock();// thread safe
	} // end of temp
	else { // else add the event sources to interferers list
		   // if the event is from other STA
		double d = 0;
		if (type == "STA")
			// find the distance from the interferer
			d = std::sqrt(
					std::pow(coordinates[0] - ((STA*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((STA*) obj)->coordinates[1], 2)
							+ std::pow(height - ((STA*) obj)->height, 2));
		if (type == "AP")
			// measure distance from AP
			d = std::sqrt(
					std::pow(coordinates[0] - ((AP*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((AP*) obj)->coordinates[1], 2)
							+ std::pow(height - ((AP*) obj)->height, 2));

		// check if the current AP is in receiving mode
		if (RX_ind == 1) {
			// Measure the PL
			double PL_meas = 0;
			if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
				// initialize a vector pointer to measure PL
				std::vector<double> *PL = new std::vector<double>;
				// Measure PL from all the interferer sources
				PL_measure(ACTIVE_Devices, PL, fc, channel_type);
				for (auto it = PL->begin(); it != PL->end(); ++it) {
					// Sum all the received power from all the sources where rx power is tx power*PL
					PL_meas += Pt_W * std::pow(10, *it / 10);
				}
				delete PL;
			} // end of active devices
			  // Add the interferece power of previous state
			Int_power += PL_meas * (system_time - local_time);
			// record the time the interference changed
			local_time = system_time;
			// push the interferer distance to interferers list
			ACTIVE_Devices.push_back(d);
			//log('[',system_time,']'," AP: ",ID," receiving and added to interefers list tot",ACTIVE_Devices.size(),'\n');
		} // end of RX condition
		else {
			// Add the new interferer to the interferers list
			ACTIVE_Devices.push_back(d);
			//log('[',system_time,']'," AP: ",ID," added to interefers list tot",ACTIVE_Devices.size(),'\n');
		} // end of else
	} // end of else
} // end of function

/*Callback function for finished tx
 * When a device finishes transmission and notifys the listeners
 * If this device is a part of the listeners then
 * this call back function will be called if the event is FIN_TRANSMITTING
 * This function includes
 * 1. Checking if the calling object is connected to the AP
 * 2. IF so then if AP is already in reception then stops receiving from the STA if it was already receiving from the same STA
 * 3. If the calling object is not associated with AP the it is removed from the interferers list
 * 4. If the AP is already in reception mode then the interference is calculated until this interferer left
 * 
 * INPUT: calling object (obj), type of the calling object (obj.TYPE) 
 */
void AP::callback_fintx(void* obj, std::string type) {
	
	thread_safe_mtx.lock(); //thread safe
	
	bool temp = 0;
	if (type == "AP")
		temp = 0;
	if (type == "STA") {
		// check if the STA is associated with AP
		for (auto it = BS_UEs.begin(); it != BS_UEs.end(); ++it) {
			if (obj == *it) {
				temp = 1;
				break;
			}
		}
	}

	if (temp && RX_ind == 1 && CURRENT_STA_UL == ((STA*) obj)->ID) { // if STA is associated with AP and AP is receiving
	// stop receiving
		RX_ind = 0;
		// Measure the PL
		double PL_meas = 0;
		if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
			// initialize a vector pointer to measure PL
			std::vector<double> *PL = new std::vector<double>;
			// Measure PL from all the interferer sources
			PL_measure(ACTIVE_Devices, PL, fc, channel_type);
			for (auto it = PL->begin(); it != PL->end(); ++it) {
				// Sum all the received power from all the sources where rx power is tx power*PL
				PL_meas += Pt_W * std::pow(10, *it / 10);
			}
			delete PL;
		} // end of active device
		  // compute interference + noise
		Int_power += PL_meas * (system_time - local_time)
				+ Channel_occupy_time * Noise;
		std::vector<double> d;
		// calculate the distance between STA and serving AP
		d.push_back(
				std::sqrt(
						std::pow(coordinates[0] - ((STA*) obj)->coordinates[0],
								2)
								+ std::pow(
										coordinates[1]
												- ((STA*) obj)->coordinates[1],
										2)
								+ std::pow(height - ((STA*) obj)->height, 2)));
		std::vector<double> *PL_sig = new std::vector<double>;
		PL_measure(d, PL_sig, fc, channel_type);
		// calculate the received signal power
		Sig_power = Channel_occupy_time * Pt_W
				* std::pow(10, PL_sig->front() / 10);
		delete PL_sig;
		// Calculate SINR and convert to dB
		double SINR_db = 10 * std::log10(Sig_power / Int_power);
		// Log the SINR for stats
		
		//((STA*) obj)->SINR.push_back(SINR_db);
		
		log('[',system_time,']'," AP: ",ID," stop rx SINR ",SINR_db," length ",
		((STA*) obj)->SINR.size(),'\n');
		// find if the transmission is in collsision based on the SINR
		bool collision = find_prb_collision(SINR_db, ((STA*) obj)->MCS_UL);
		// update global variable total_tti marking finish of 1 tti
		total_tti++;
		if (collision == 0) {
			// if packet not in collision
			// update the success
			((STA*) obj)->success = ((STA*) obj)->success + 1;
			// Decrement the data received from the UL buffer
			((STA*) obj)->Buffer_UL.front() -= ALLOC_data_UL[((STA*) obj)->ID];
			// increment the served data STAT
			((STA*) obj)->served_data += ALLOC_data_UL[((STA*) obj)->ID];
			// decrement the total buff
			((STA*) obj)->TOT_BUFF -= ALLOC_data_UL[((STA*) obj)->ID];
			// if the file is finished
			if (((STA*) obj)->Buffer_UL.front() <= 0) {
				// record the time taken for the file to download
				int te2 = system_time
						- ((STA*) obj)->Time_file_arrived_UL.front();
				// push the time taken for STATs
				
				//((STA*) obj)->TTTDeliverOFile_UL.push_back(te2);
				log('[',system_time,']'," AP: ",ID," TTTDOF UL ",te2,'\n');
				// pop the time the file arrived
				((STA*) obj)->Time_file_arrived_UL.pop_front();
				// see if there is another file and check if that is scheduled too
				if (((STA*) obj)->TOT_BUFF > 0) {
					// check the top of the buffer
					int temp = ((STA*) obj)->Buffer_UL.front();
					// pop it out of the buffer
					((STA*) obj)->Buffer_UL.pop_front();
					// ADD it to the next packet
					((STA*) obj)->Buffer_UL.front() += temp;
				} else {
					// due 20us granularity some zero padding will happen
					// if there is no new packet remove the zeros from counting into served data
					((STA*) obj)->served_data += ((STA*) obj)->TOT_BUFF;
					// remove the zero padding
					((STA*) obj)->TOT_BUFF = 0;
					// remove the finished packet from buffer
					((STA*) obj)->Buffer_UL.pop_front();
				} // end of else
				  // decrement that file from the BUF_Ind
				((STA*) obj)->BUF_Ind = ((STA*) obj)->BUF_Ind - 1;
			} // end of buffer_ul
			  // MCS Adaptation
			  // if total attempts for this STA are 100
			if (((STA*) obj)->attempt == 100) {
				// check among the 100 attempts >70 are success
				if (((STA*) obj)->success > 70) {
					// then increase the MCS
					((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL + 1;
					// if the MCS is >= MAX MCS
					if (((STA*) obj)->MCS_UL >= MAX_MCS)
						// then set it to MAX MCS
						((STA*) obj)->MCS_UL = MAX_MCS - 1;
				} // end of success
				  // check if the success are less than 30%
				else if (((STA*) obj)->success < 30) {
					// then decrement the MCS
					((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL - 1;
					// if the MCS is less than the minimum
					if (((STA*) obj)->MCS_UL < 0)
						// then set it to minimum
						((STA*) obj)->MCS_UL = 0;
				} // end of if else
				  // set the attempts to 0 for this STA
				((STA*) obj)->attempt = 0;
				// set the success to 0 for this STA
				((STA*) obj)->success = 0;
			} // end of attempt condition
			  // update contention window to the minimum
			((STA*) obj)->CW = CW_min;
			// reset ALLOC UL
			ALLOC_data_UL[((STA*) obj)->ID] = 0;
		} // end of collision condition
		  // if it is collision
		else {
			// double the contention window
			((STA*) obj)->CW = ((STA*) obj)->CW * 2;
			// if the current window is maximum or greater
			if (((STA*) obj)->CW > CW_max)
				// set it to maximum
				((STA*) obj)->CW = CW_max;
			// MCS Adaptation
			// if total attempts for this STA are 100
			if (((STA*) obj)->attempt == 100) {
				// check among the 100 attempts >70 are success
				if (((STA*) obj)->success > 70) {
					// then increase the MCS
					((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL + 1;
					// if the MCS is >= MAX MCS
					if (((STA*) obj)->MCS_UL >= MAX_MCS)
						// then set it to MAX MCS
						((STA*) obj)->MCS_UL = MAX_MCS - 1;
				} // end of success
				  // check if the success are less than 30%
				else if (((STA*) obj)->success < 30) {
					// then decrement the MCS
					((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL - 1;
					// if the MCS is less than the minimum
					if (((STA*) obj)->MCS_UL < 0)
						// then set it to minimum
						((STA*) obj)->MCS_UL = 0;
				} // end of if else
				  // set the attempts to 0 for this STA
				((STA*) obj)->attempt = 0;
				// set the success to 0 for this STA
				((STA*) obj)->success = 0;
			} // end of attempt condition
		}
		// reset ALLOC UL
		ALLOC_data_UL[((STA*) obj)->ID] = 0;
	} // end of if temp
	  // if EVENT from associated STA and not receiving this STA
	else if (temp) {
		// double the contention window
		((STA*) obj)->CW = ((STA*) obj)->CW * 2;
		// if the current window is maximum or greater
		if (((STA*) obj)->CW > CW_max)
			// set it to maximum
			((STA*) obj)->CW = CW_max;
		// MCS Adaptation
		// if total attempts for this STA are 100
		if (((STA*) obj)->attempt == 100) {
			// check among the 100 attempts >70 are success
			if (((STA*) obj)->success > 70) {
				// then increase the MCS
				((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL + 1;
				// if the MCS is >= MAX MCS
				if (((STA*) obj)->MCS_UL >= MAX_MCS)
					// then set it to MAX MCS
					((STA*) obj)->MCS_UL = MAX_MCS - 1;
			} // end of success
			  // check if the success are less than 30%
			else if (((STA*) obj)->success < 30) {
				// then decrement the MCS
				((STA*) obj)->MCS_UL = ((STA*) obj)->MCS_UL - 1;
				// if the MCS is less than the minimum
				if (((STA*) obj)->MCS_UL < 0)
					// then set it to minimum
					((STA*) obj)->MCS_UL = 0;
			} // end of if else
			  // set the attempts to 0 for this STA
			((STA*) obj)->attempt = 0;
			// set the success to 0 for this STA
			((STA*) obj)->success = 0;
		} // end of attempt condition
		  // reset ALLOC UL
		ALLOC_data_UL[((STA*) obj)->ID] = 0;
		// find the distance from the AP
		double d = std::sqrt(
				std::pow(coordinates[0] - ((STA*) obj)->coordinates[0], 2)
						+ std::pow(
								coordinates[1] - ((STA*) obj)->coordinates[1],
								2)
						+ std::pow(height - ((STA*) obj)->height, 2));
		if (RX_ind == 1) {
			// Measure the PL
			double PL_meas = 0;
			if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
				// initialize a vector pointer to measure PL
				std::vector<double> *PL = new std::vector<double>;
				// Measure PL from all the interferer sources
				PL_measure(ACTIVE_Devices, PL, fc, channel_type);
				for (auto it = PL->begin(); it != PL->end(); ++it) {
					// Sum all the received power from all the sources where rx power is tx power*PL
					PL_meas += Pt_W * std::pow(10, *it / 10);
				}
				delete PL;
			} // end of active device
			  // Add the interferece power of previous state
			Int_power += PL_meas * (system_time - local_time);
			// record the time the interference changed
			local_time = system_time;
			// remove the STA from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			} // end of for
		} // end of if
		else {
			// remove the STA from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			} // end of for
		} // end of else
	} // end of elseif
	else {
		double d = 0;
		if (type == "STA")
			d = std::sqrt(
					std::pow(coordinates[0] - ((STA*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((STA*) obj)->coordinates[1], 2)
							+ std::pow(height - ((STA*) obj)->height, 2));
		if (type == "AP")
			d = std::sqrt(
					std::pow(coordinates[0] - ((AP*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((AP*) obj)->coordinates[1], 2)
							+ std::pow(height - ((AP*) obj)->height, 2));

		if (RX_ind == 1) {
			// Measure the PL
			double PL_meas = 0;
			if (!ACTIVE_Devices.empty()) { // compute only if there are interferers
				// initialize a vector pointer to measure PL
				std::vector<double> *PL = new std::vector<double>;
				// Measure PL from all the interferer sources
				PL_measure(ACTIVE_Devices, PL, fc, channel_type);
				for (auto it = PL->begin(); it != PL->end(); ++it) {
					// Sum all the received power from all the sources where rx power is tx power*PL
					PL_meas += Pt_W * std::pow(10, *it / 10);
				}
				delete PL;
			} // end of active device
			  // Add the interferece power of previous state
			Int_power += PL_meas * (system_time - local_time);
			// record the time the interference changed
			local_time = system_time;
			// remove the device from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			} // end of for
		} else {
			// remove the device from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			}
		}
	}
			thread_safe_mtx.unlock(); //thread safe
}

/* ADD listener type station
 * This function adds a STA/AP to the listeners group
 * INPUT: object handle (obj) , object type (obj.TYPE)
 */
void AP::add_listener(void* obj, std::string type) {
	// if the listener is AP
	if (type == "AP")
		// ADD to AP group
		listener_AP.push_back(obj);
	// if the listener is STA
	if (type == "STA")
		// ADD to STA group
		listener_STA.push_back(obj);
}

/* Notify the listeners
 * This function calls the callback functions of the listeners based on the event registered
 * \param EVENT TRANSMITTING or FIN_TRANSMITTING defined in common.h
 */
void AP::notify_listener(int EVENT) {
	switch (EVENT) {

	case TRANSMITTING:
		// Call back AP
		for (auto it = listener_AP.begin(); it != listener_AP.end(); ++it) {
			((AP*) (*it))->callback_tx(this, this->TYPE);
		}

		// call back STA
		for (auto it = listener_STA.begin(); it != listener_STA.end(); ++it) {
			((STA*) (*it))->callback_tx(this, this->TYPE);
		}

		break;

	case FIN_TRANSMITTING:
		// AP call back
		for (auto it = listener_AP.begin(); it != listener_AP.end(); ++it) {
			((AP*) (*it))->callback_fintx(this, this->TYPE);
		}

		// STA call back
		for (auto it = listener_STA.begin(); it != listener_STA.end(); ++it) {
			((STA*) (*it))->callback_fintx(this, this->TYPE);
		}

		break;
	default:
		std::cout << "unknown event occured \n";
	} // end of switch

} // end of function
