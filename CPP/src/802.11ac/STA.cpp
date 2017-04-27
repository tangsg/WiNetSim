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
/*! \file STA.cpp
 * \brief STA class source 
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
 extern void log();
#include "common.h"
#include "STA.h"
#include "AP.h"
// global simulation clock in us  
extern long long system_time;
// TTIs
extern long long total_tti;
extern std::mutex thread_safe_mtx;
// Constructor
STA::STA() {
}

void STA::STA_init(int ID_in, double coordinates_in[2], parameters params,
		void * BS_object) {
	// Unique number to identify the AP
	ID = ID_in;
	// X coordinate
	coordinates[0] = coordinates_in[0];
	// Y coordinate
	coordinates[1] = coordinates_in[1];
	// Height of the AP from the ground
	height = params.height_AP;
	// Associated AP object Handle
	BS_obj = BS_object;
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
	// Uplink load per user in packets per micro second
	load_peruser = params.load_peruser_ul;
	// Noise floor of the receiver in Watts measured over 1 subcarrier
	Noise = params.Noise;
	// Transmit power of the AP in Watts measured over 1 subcarrier
	Pt_W = params.Pt_STA;
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
	MCS_UL = MAX_MCS - 1;
	// bits transmitted in 20micro seconds for each MCS
	WIFI_data_20us.insert(WIFI_data_20us.begin(), &params.WIFI_data_20us[0],
			&params.WIFI_data_20us[MAX_MCS]);
	// Exponential random variable initializer
	std::exponential_distribution<double> exp_distribution(load_peruser);
	// time after which next file arrives
	Time_next_file_arrives = (long long) std::round(
			exp_distribution(generator));
	// Initialize attemp
	attempt = 0;
	// Initialize success
	success = 0;
	// Set the curtrent contention window to the minimum
	CW = CW_min;
	
	//print
	/*
	log("______________________________________________\n");
	log(" STA: ",ID," Parameters \n");
	log("______________________________________________\n");
	log(" Associated to ",((AP*)BS_obj)->TYPE," ID ",((AP*)BS_obj)->ID,'\n');
	log(" coord ",coordinates[0],',',coordinates[1],"\n");
	log(" height ",height,"\n");
	log(" CWmin: ",CW_min," CWmax: ",CW_max," CW: ",CW,"\n");
	log(" slot: ",wifi_slot," DIFS: ",DIFS_time," MAXTXOP: ",MAX_TXOP,"\n");
	log(" load_peruser: ",load_peruser," Noise: ",Noise,"\n");
	log(" Pt_W: ",Pt_W," fc: ",fc," channel_type: ",channel_type,"\n");
	log(" file_size: ",file_size," WIFI_TH: ",WIFI_TH," MAX_MCS: ",MAX_MCS,"\n");
		log(" MCS_UL:",MCS_UL," attempt:",attempt,
		" success:",success," TNFA:",Time_next_file_arrives
		,'\n');
	for (int i=0;i<MAX_MCS;i++){
		log(" WIFI_data_20us[:",i,"] ",WIFI_data_20us[i],"\n");}	
	log("______________________________________________\n");
	*/
}


void STA::jump_time(int jump) {
	// Substract jump time from the time next file arrives
	//log(" STA: ",ID," TNFA ",Time_next_file_arrives," jump ",jump," TNFA ");
	Time_next_file_arrives -= jump;
	//log(Time_next_file_arrives,'\n');
}


int STA::check_buffer() {
	// initialize jump
	int jump = 0;
	// exponential distribution for load
	std::exponential_distribution<double> exp_distribution(load_peruser);
	// Decrement time next file arrives
	Time_next_file_arrives -= 1;
	// Check if the file arrived
	if (Time_next_file_arrives <= 0) {
		// push the time the file arrived
		Time_file_arrived_UL.push_back(system_time);
		// Find the next packet arrival time
		Time_next_file_arrives = (long long) std::round(
				exp_distribution(generator));
		// Push the arrived file into the buffer
		Buffer_UL.push_back(file_size);
		// Increment the STATISTIC total files arrived 
		total_files += 1;
		// Increment buffer indicator
		BUF_Ind += 1;
		// Sum the file size to total buffer size
		TOT_BUFF += file_size;
		log('[',system_time,']'," STA: ",ID," buff_size ",Buffer_UL.size(),
		" total files ",total_files," Buf Ind ",BUF_Ind," Tot buff ",
		TOT_BUFF,'\n');
	}
	// update jump with the time next file arrives		
	jump = Time_next_file_arrives;

	if (contending == 0 && TX_ind == 0 && RX_ind == 0) {
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
			DIFS_Ind = 0;
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
				} // End of for loop
				delete PL;
			} // end of active devices empty condition
			log('[',system_time,']'," STA: ",ID," Energy_meas ",Energy_meas," Noise ",Noise,
	  " contending ",contending," TX_ind ",TX_ind," RX_ind ",RX_ind,
	  " Counter ",Counter," DIFS_Ind ",DIFS_Ind," DIFS_counter ",
	  DIFS_counter," Active ",ACTIVE_Devices.size(),'\n');	
		} // End of buff ind>0 condition
	} // End of contending tx rx condition
	else if (contending == 1) {
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
			} // End of for loop
			delete PL;
		} // end of active devices empty condition
	} // end of else if contending condition
	else {
		// might be receiving might be transmitting
		// set the jump to zero
		jump = 0;
	} // end of else
	  // return time jump value
	  	
	return jump;
} // end of function check_buffer


void STA::sched_tx() {
	// DO DCF if contending
	if (contending == 1) {
		// See if DIFS is done
		if (DIFS_Ind == 1) {
			// check if the measured enrgy is less than WIFI sensing threshold
			if (Energy_meas > WIFI_TH) {
				// If energy meas id > sens thresh then bring the counter to multiple of wifi slots
				Counter = Counter - (Counter % wifi_slot) + wifi_slot;
				// Set DIFS Ind to 0
				DIFS_Ind = 0;
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
				DIFS_Ind = 0;
				// Set DIFS counter
				DIFS_counter = DIFS_time;
				// Set TX indicater to 1
				TX_ind = 1;
				// Start Scheduling
				// Set channel occupy time to zero;
				Channel_occupy_time = 0;
				// check if the buffer is non empty (Redundant condition)
				if (TOT_BUFF > 0) {
					// increase the attempts counter 
					attempt += 1;
					// see how much chanel time required for transmission
					long long time = std::ceil(
							(double) TOT_BUFF / WIFI_data_20us[MCS_UL]);
					// check whether the time required is less than the MAX tx opportunity
					if (20 * time < MAX_TXOP) { // 20us is the minimum granularity
						// update the channel occupy time
						Channel_occupy_time = 20 * time;
						// Update Allocated data
						thread_safe_mtx.lock();
						((AP*) BS_obj)->ALLOC_data_UL[ID] = TOT_BUFF;
						thread_safe_mtx.unlock();
						// update the pumped data
						pumped_data += TOT_BUFF;
					} // end of if time condition
					else {
						// occupy the channel for max tx op
						Channel_occupy_time = MAX_TXOP;
						// update Allocated data
						thread_safe_mtx.lock();
						((AP*) BS_obj)->ALLOC_data_UL[ID] =
								WIFI_data_20us[MCS_UL] * MAX_TXOP / 20;
						thread_safe_mtx.unlock();
						// update pumped data
						pumped_data += WIFI_data_20us[MCS_UL] * MAX_TXOP / 20;
					} // end of else
				} // end of TOT buf condition
				else {
					// print error for debugging logical errors
					std::cout
							<< "###### Critical Error: contending when no data ##### "
							<< std::endl;
				}
				log('[',system_time,']'," STA: ",ID," MCS_UL ",MCS_UL, "occupy time ",
				Channel_occupy_time,'\n');
				// Notify the listening group about this transmission
				notify_listener(TRANSMITTING);
			} // end of counter zero
		} // End of DIFS ind
		else if (Energy_meas < WIFI_TH) { // if measured energy is less than the WIFI sensing threshold (1us)
			// Decrement DIFS counter by 1us
			DIFS_counter = DIFS_counter - 1;
			// Check if the DIFS counter is zero
			if (DIFS_counter <= 0) {
				// Indicate that DIFS is done
				DIFS_Ind = 1;
				// initialize the DIFS counter (Redundant but to be sure)
				DIFS_counter = DIFS_time;
			} // end of DIFS_counter
		} // end of enregy meas
	} // End of if contending
} // End of function


void STA::check_tx_time() {
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


void STA::callback_tx(void* obj, std::string type) {
	// temperoray variable
	//log('[',system_time,']'," STA: ",ID,"call back tx \n");
	bool temp = 0;
	if (type == "STA")
		// is EVENT from STA
		temp = 0;
	if (type == "AP") {
		// is EVENT from AP
		// check if the AP is the serving AP
		temp = (obj== BS_obj);
	} // end of type
	if (temp && TX_ind != 1 && RX_ind != 1
			&& ((AP*) obj)->CURRENT_STA_DL == ID) {
		// if the EVENT is from serving AP and STA not receiving not transmitting and Transmission is intended for this STA
		// Start receiving
		RX_ind = 1;
		// stop contending                
		contending = 0;
		// Mark the time the transmission started
		local_time = system_time;
		// init sig power
		Sig_power = 0;
		// init interferer power
		Int_power = 0;
		// Record the channel occupy time
		thread_safe_mtx.lock();
		Channel_occupy_time = ((AP*) obj)->Channel_occupy_time;
		thread_safe_mtx.unlock();
		log('[',system_time,']'," STA: ",ID," receiving from ",((AP*)BS_obj)->TYPE,((AP*)BS_obj)->ID,'\n');
	} // end of temp
	  // else add the event sources to interferers list
	else {
		double d = 0;
		// if the event is from other STA
		if (type == "STA")
			// find the distance from the interferer
			d = std::sqrt(
					std::pow(coordinates[0] - ((STA*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((STA*) obj)->coordinates[1], 2)
							+ std::pow(height - ((STA*) obj)->height, 2));
		if (type == "AP")
			// find the distance from the interferer
			d = std::sqrt(
					std::pow(coordinates[0] - ((AP*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((AP*) obj)->coordinates[1], 2)
							+ std::pow(height - ((AP*) obj)->height, 2));
		// check if the current STA is in receiving mode
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
			// push the interferer distance to interferers list
			ACTIVE_Devices.push_back(d);
			//log('[',system_time,']'," STA: ",ID," receiving and added to interefers list tot",ACTIVE_Devices.size(),'\n');
		} // end of RX condition
		else {
			// Add the new interferer to the interferers list
			ACTIVE_Devices.push_back(d);
			//log('[',system_time,']'," STA: ",ID," added to interefers list tot",ACTIVE_Devices.size(),'\n');
		} // end of else condition
	} // end of else not associated
} // end of the function


void STA::callback_fintx(void* obj, std::string type) {
	thread_safe_mtx.lock();
	//log('[',system_time,']'," STA: ",ID,"call back fin tx \n");
	bool temp = 0;
	if (type == "STA")
		temp = 0;
	if (type == "AP") {
		temp = (obj== BS_obj);
	}
	// if EVENT from serving AP and STA is in receiving mode and data is intended for this STA
	if (temp && RX_ind == 1 && ((AP*) obj)->CURRENT_STA_DL == ID) {
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
						std::pow(coordinates[0] - ((AP*) obj)->coordinates[0],
								2)
								+ std::pow(
										coordinates[1]
												- ((AP*) obj)->coordinates[1],
										2)
								+ std::pow(height - ((AP*) obj)->height, 2)));
		std::vector<double> *PL_sig = new std::vector<double>;
		PL_measure(d, PL_sig, fc, channel_type);
		// calculate the received signal power
		Sig_power = Channel_occupy_time * Pt_W
				* std::pow(10, PL_sig->front() / 10);
		delete PL_sig;
		// Calculate SINR and convert to dB
		double SINR_db = 10 * std::log10(Sig_power / Int_power);
		// Log the SINR for stats
		//((AP*) obj)->SINR.push_back(SINR_db);
		log('[',system_time,']'," STA: ",ID," stop rx SINR ",SINR_db," length ",
		((AP*) obj)->SINR.size(),'\n');
		// find if the transmission is in collsision based on the SINR
		bool collision = find_prb_collision(SINR_db, ((AP*) obj)->MCS_DL[ID]);
		// update global variable total_tti marking finish of 1 tti
		total_tti++;
		if (collision == 0) {
			// if packet not in collision
			// update the success 
			((AP*) obj)->success[ID] += 1;
			// Decrement the data received from the DL buffer
			((AP*) obj)->Buffer_DL[ID].front() -=
					((AP*) obj)->ALLOC_data_DL[ID];
			// Decrement the data received from the summ BUFF DL buffer
			((AP*) obj)->SUM_BUFF_DL[ID] -= ((AP*) obj)->ALLOC_data_DL[ID];
			// increment the served data STAT
			((AP*) obj)->served_data += ((AP*) obj)->ALLOC_data_DL[ID];
			// decrement the total buff
			((AP*) obj)->TOT_BUFF -= ((AP*) obj)->ALLOC_data_DL[ID];
			// if the file is finished
			if (((AP*) obj)->Buffer_DL[ID].front() <= 0) {
				// record the time taken for the file to download
				int te2 = system_time
						- ((AP*) obj)->Time_file_arrived_DL[ID].front();
				// push the time taken for STATs
				//((AP*) obj)->TTTDeliverOFile_DL.push_back(te2);
				log('[',system_time,']'," STA: ",ID," TTTDOF DL ",te2,'\n');
				// pop the time the file arrived
				((AP*) obj)->Time_file_arrived_DL[ID].pop_front();
				// see if there is another file and check if that is scheduled too
				if (((AP*) obj)->SUM_BUFF_DL[ID] > 0) {
					// check the top of the buffer
					int temp = ((AP*) obj)->Buffer_DL[ID].front();
					// pop it out of the buffer
					((AP*) obj)->Buffer_DL[ID].pop_front();
					// ADD it to the next packet
					((AP*) obj)->Buffer_DL[ID].front() += temp;
				} else {
					// due 20us granularity some zero padding will happen
					// if there is no new packet remove the zeros from counting into served data
					((AP*) obj)->served_data += ((AP*) obj)->SUM_BUFF_DL[ID];
					// do the same for tot buff
					((AP*) obj)->TOT_BUFF -= ((AP*) obj)->SUM_BUFF_DL[ID];
					// set the sum buff to zero
					((AP*) obj)->SUM_BUFF_DL[ID] = 0;
					// remove the finished packet from buffer
					((AP*) obj)->Buffer_DL[ID].pop_front();
				} // end of else
				  // decrement that file from the BUF_Ind	
				((AP*) obj)->BUF_Ind -= 1;
			} // end of buffer_dl
			  // reset the ALLOC data dl
			((AP*) obj)->ALLOC_data_DL[ID] = 0;
			// MCS Adaptation
			// if total attempts for this STA are 100	
			if (((AP*) obj)->attempt[ID] == 100) {
				// check among the 100 attempts >70 are success
				if (((AP*) obj)->success[ID] > 70) {
					// then increase the MCS
					((AP*) obj)->MCS_DL[ID] += 1;
					// if the MCS is >= MAX MCS
					if (((AP*) obj)->MCS_DL[ID] >= MAX_MCS)
						// then set it to MAX MCS
						((AP*) obj)->MCS_DL[ID] = MAX_MCS - 1;
				} // end of success
				  // check if the success are less than 30%    
				else if (((AP*) obj)->success[ID] < 30) {
					// then decrement the MCS
					((AP*) obj)->MCS_DL[ID] -= 1;
					// if the MCS is less than the minimum
					if (((AP*) obj)->MCS_DL[ID] < 0)
						// then set it to minimum
						((AP*) obj)->MCS_DL[ID] = 0;
				} // end of if else
				  // set the attempts to 0 for this STA		
				((AP*) obj)->attempt[ID] = 0;
				// set the success to 0 for this STA
				((AP*) obj)->success[ID] = 0;
			} // end of attempt condition
			  // reset the ALLCO data dl
			((AP*) obj)->ALLOC_data_DL[ID] = 0;
			// update contention window to the minimum
			((AP*) obj)->CW = CW_min;
		} // end of collision condition
		  // if it is collision
		else {
			// double the contention window
			((AP*) obj)->CW = ((AP*) obj)->CW * 2;
			// if the current window is maximum or greater
			if (((AP*) obj)->CW > CW_max)
				// set it to maximum
				((AP*) obj)->CW = CW_max;
			// MCS Adaptation
			// if total attempts for this STA are 100	
			if (((AP*) obj)->attempt[ID] == 100) {
				// check among the 100 attempts >70 are success
				if (((AP*) obj)->success[ID] > 70) {
					// then increase the MCS
					((AP*) obj)->MCS_DL[ID] += 1;
					// if the MCS is >= MAX MCS
					if (((AP*) obj)->MCS_DL[ID] >= MAX_MCS)
						// then set it to MAX MCS
						((AP*) obj)->MCS_DL[ID] = MAX_MCS - 1;
				} // end of success
				  // check if the success are less than 30%    
				else if (((AP*) obj)->success[ID] < 30) {
					// then decrement the MCS
					((AP*) obj)->MCS_DL[ID] -= 1;
					// if the MCS is less than the minimum
					if (((AP*) obj)->MCS_DL[ID] < 0)
						// then set it to minimum
						((AP*) obj)->MCS_DL[ID] = 0;
				} // end of if else
				  // set the attempts to 0 for this STA		
				((AP*) obj)->attempt[ID] = 0;
				// set the success to 0 for this STA
				((AP*) obj)->success[ID] = 0;
			} // end of attempt condition
		}
		// reset the ALLCO data dl
		((AP*) obj)->ALLOC_data_DL[ID] = 0;
	}
	// if EVENT from serving AP and STA is not in receiving mode and data is intended for this STA
	else if (temp && ((AP*) obj)->CURRENT_STA_DL == ID) {
		// double the contention window
		((AP*) obj)->CW = ((AP*) obj)->CW * 2;
		// if the current window is maximum or greater
		if (((AP*) obj)->CW > CW_max)
			// set it to maximum
			((AP*) obj)->CW = CW_max;
		// MCS Adaptation
		// if total attempts for this STA are 100	
		if (((AP*) obj)->attempt[ID] == 100) {
			// check among the 100 attempts >70 are success
			if (((AP*) obj)->success[ID] > 70) {
				// then increase the MCS
				((AP*) obj)->MCS_DL[ID] += 1;
				// if the MCS is >= MAX MCS
				if (((AP*) obj)->MCS_DL[ID] >= MAX_MCS)
					// then set it to MAX MCS
					((AP*) obj)->MCS_DL[ID] = MAX_MCS - 1;
			} // end of success
			  // check if the success are less than 30%    
			else if (((AP*) obj)->success[ID] < 30) {
				// then decrement the MCS
				((AP*) obj)->MCS_DL[ID] -= 1;
				// if the MCS is less than the minimum
				if (((AP*) obj)->MCS_DL[ID] < 0)
					// then set it to minimum
					((AP*) obj)->MCS_DL[ID] = 0;
			} // end of if else
			  // set the attempts to 0 for this STA		
			((AP*) obj)->attempt[ID] = 0;
			// set the success to 0 for this STA
			((AP*) obj)->success[ID] = 0;
		} // end of attempt condition
		  // find the distance from the AP    
		double d = std::sqrt(
				std::pow(coordinates[0] - ((AP*) obj)->coordinates[0], 2)
						+ std::pow(coordinates[1] - ((AP*) obj)->coordinates[1],
								2) + std::pow(height - ((AP*) obj)->height, 2));
		// remove the AP from the interferers list
		for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
				++it) {
			if (*it == d) {
				ACTIVE_Devices.erase(it);
				break;
			} // end of if loop
		} // end of for loop
	} // end of else if
	else {
		double d = 0;
		// if EVENT from STA
		if (type == "STA")
			// Measure the distance	
			d = std::sqrt(
					std::pow(coordinates[0] - ((STA*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((STA*) obj)->coordinates[1], 2)
							+ std::pow(height - ((STA*) obj)->height, 2));
		if (type == "AP") // if EVENT from AP
			// Measure the distance	
			d = std::sqrt(
					std::pow(coordinates[0] - ((AP*) obj)->coordinates[0], 2)
							+ std::pow(
									coordinates[1]
											- ((AP*) obj)->coordinates[1], 2)
							+ std::pow(height - ((AP*) obj)->height, 2));

		// if STA is in RX mode
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
			  // accumulate interference power
			Int_power += PL_meas * (system_time - local_time);
			// record the interference change time
			local_time = system_time;
			// remove the device from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			} // end of for loop
		} // end of RX=1
		else {
			// remove the device from the interferers list
			for (auto it = ACTIVE_Devices.begin(); it != ACTIVE_Devices.end();
					++it) {
				if (*it == d) {
					ACTIVE_Devices.erase(it);
					break;
				}
			} // end of for loop
		} // end of else
	} // end of else
	
			thread_safe_mtx.unlock();
} // end of function


void STA::add_listener(void* obj, std::string type) {
	// if the listener is AP
	if (type == "AP")
		// ADD to AP group
		listener_AP.push_back(obj);
	// if the listener is STA
	if (type == "STA")
		// ADD to STA group
		listener_STA.push_back(obj);
}


void STA::notify_listener(int EVENT) {
	switch (EVENT) {

	case TRANSMITTING:
		// Call back AP
		for (auto it = listener_AP.begin(); it != listener_AP.end(); ++it) {
			((AP*) (*it))->callback_tx(this, this->TYPE);
		}

		// call back STA
		//log('[',system_time,']'," STA: ",ID," listener_STA ",listener_STA.size(),'\n');
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
