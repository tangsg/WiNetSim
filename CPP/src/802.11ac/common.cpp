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
/*! \file common.cpp 
 * \brief common functions
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#include "common.h"
#include "BLER.h"
//! Sets the simulation parameters

extern parameters common_params;

//! Print the input simulation parameters. \n
void print_params() {
	using namespace std;
	cout
			<< "______________system parameters______________________________________"
			<< endl;
	cout << "Ncells               : " << common_params.NCell << endl;
	cout << "Ap      per Cell     : " << common_params.n_AP  << endl;
	cout << "STA     per AP       : " << common_params.n_STA << endl;
	cout << "BW                   : " << common_params.BW << " MHz" << endl;
	cout << "AP Tx Power          : " << common_params.Pt_AP << " W ("
			<< 10 * log10(common_params.Pt_AP) + 30 + 10 * log10(56) << " dBm)"
			<< endl;
	cout << "STA Tx Power         : " << common_params.Pt_STA << " W ("
			<< 10 * log10(common_params.Pt_STA) + 30 + 10 * log10(56) << " dBm)"
			<< endl;
	cout << "Noise                : " << common_params.Noise << " W ("
			<< 10 * log10(common_params.Noise) + 30 + 10 * log10(64) << " dBm)"
			<< endl;
	cout << "fc                   : " << common_params.fc << " GHz" << endl;
	cout << "Wifi TH              : " << common_params.WIFI_TH << " W ("
			<< 10 * log10(common_params.WIFI_TH) + 30 + 10 * log10(56)
			<< " dBm)" << endl;
	cout << "Channel model        : " << common_params.channel_model << endl;
	cout << endl;
	cout << "_______________DCF parameters_____________________________________"
			<< endl;
	cout << "CWmin                : " << common_params.CW_min << endl;
	cout << "CWmax                : " << common_params.CW_max << endl;
	cout << "slot duration        : " << common_params.wifi_slot << " us"
			<< endl;
	cout << "DIFS Duration        : " << common_params.DIFS_time << " us"
			<< endl;
	cout << "MAX tx opp           : " << common_params.MAX_TXOP << " us"
			<< endl;
	cout << endl;
	cout << "_______________Load/Traffic_____________________________________"
			<< endl;
	cout << "DL/UL ratio          : " << common_params.DL_UL << endl;
	cout << "Network load         : " << common_params.Network_load_bps / 1e6
			<< " Mbps" << endl;
	cout << endl;
	cout << "________Indoor model with room length: "
			<< common_params.dimensions[0] << "m breadth: "
			<< common_params.dimensions[1] << "m_________" << endl;
	cout
			<< "________________________________START_________________________________________"
			<< endl;
}

/*! 
 * checks whether the \a coord are inside a rectangle given by \a dimension[2]
 * \param dimension[2]
 * \param coord[2]
 * \return \c true if inside \c false if outside 
 */
bool is_inside_rectangle(double dimension[2], double coord[2]) {
	// returns true if the coordinates are inside the lengthxbreadth centre [0,0]
	return (std::islessequal(std::abs(coord[0]), dimension[0] / 2))
			&& (std::islessequal(std::abs(coord[1]), dimension[1] / 2));
}
// AWGN 11ac MCS 8 BLER
extern std::vector<double> AWGN_11ac_8_MCS_BLER[9];
// AWGN 11ac MCS 8 SNR
extern std::vector<double> AWGN_11ac_8_MCS_SNR[9];

/*! finds if the packet is in collision given the \a SINR_dB in dB and \a MCS_index \n
 * \param SINR  double 
 * \param MCS_index integer
 * \return \c true if collision or \c false if not collision
 */
bool find_prb_collision(double SINR_db, int MCS_index) {
	// init uniform real distibution between [0,1]
	std::uniform_real_distribution<double> uni_distribution(0.0, 1.0);
	// set random device generator
	std::random_device generator;
	// interpolate and find the BLER for the given SINR and MCS Index
	double bler = Monotone_cubic_spline_interpolation(
			&AWGN_11ac_8_MCS_SNR[MCS_index], &AWGN_11ac_8_MCS_BLER[MCS_index],
			SINR_db);
	// get a random number between [0,1]
	double rnd_number = uni_distribution(generator);
	// see if bler is less than the number generateed
	if (bler < rnd_number) // no collision
		return false;
	else
		// collision
		return true;
}
/*! Monotone_cubic_spline_interpolation  \n
 *  reference : https://en.wikipedia.org/wiki/Monotone_cubic_interpolation \n
 * The function takes \a SNR_LUT values sorted in ascending order
 * and \a BLER_LUT corresponding to \a SNR_LUT and checks the \a BLER_LUT values corresponding to \a SNR_dB \n
 * if the value is in the range of \a SNR_LUT and not present in the data then the function does a monotone cubic spline interpolation. \n
 * if the data is out of range then it gives the first or last value depending on the side the \a SNR_dB falls in.
 * \param *SNR_LUT pointer to vector
 * \param *BLER_LUT pointer to vector
 * \param SNR_dB double
 * \return BLER_est double
 */
double Monotone_cubic_spline_interpolation(std::vector<double> *SNR_LUT,
		std::vector<double> *BLER_LUT, double SNR_db) {
	int i = 0;
	// compute dx i.e SNR differences
	std::vector<double> dx;
	for (auto it = SNR_LUT->begin(); it != SNR_LUT->end() - 1; ++it) // runs from i = 1...n-1
			{
		dx.push_back(*(it + 1) - *(it));
	}
	// compute dy i.e BLER differences
	std::vector<double> dy;
	for (auto it = BLER_LUT->begin(); it != BLER_LUT->end() - 1; ++it) // runs from i = 1...n-1
			{
		dy.push_back(*(it + 1) - *(it));
	}
	// compute slopes dy/dx
	std::vector<double> ms;
	for (auto it = dx.begin(); it != dx.end(); ++it) // runs from i = 1...n-1
			{
		ms.push_back(dy[i] / (*it));
		i++;
	}
	std::vector<double> c1s;
	std::vector<double> c2s;
	std::vector<double> c3s;

	// compute c1s first order coefficients
	c1s.push_back(ms.front());
	for (i = 0; i < ms.size() - 1; i++) {
		if (ms[i] * ms[i + 1] < 0)
			c1s.push_back(0);
		else
			c1s.push_back(
					3 * (dx[i] + dx[i + 1])
							/ (((dx[i] + dx[i + 1] + dx[i + 1]) / ms[i])
									+ ((dx[i] + dx[i + 1] + dx[i]) / ms[i + 1])));
	}
	c1s.push_back(ms.back());

	// compute c2s and c3s 2nd and third order coeffs
	for (i = 0; i < c1s.size() - 1; i++) {
		c2s.push_back(
				(ms[i] - c1s[i] - c1s[i] - c1s[i + 1] + ms[i] + ms[i]) / dx[i]);
		c3s.push_back((c1s[i] + c1s[i + 1] - ms[i] - ms[i]) / (dx[i] * dx[i]));
	}
	double BLER_est = 0;
	bool do_interpolation = true;

	// see if we can avoid interpolation
	for (i = 0; i < SNR_LUT->size(); i++) {
		if (*(SNR_LUT->data() + i) == SNR_db) {
			BLER_est = *(BLER_LUT->data() + i);
			do_interpolation = false;
			break;
		}
	}

	if (do_interpolation) {
		// see if the input is not in range
		if (SNR_db < SNR_LUT->front()) {
			BLER_est = BLER_LUT->front();
		} else if (SNR_db > SNR_LUT->back()) {
			BLER_est = BLER_LUT->back();
		} else {
			// start interpolation
			for (i = 0; i < SNR_LUT->size(); i++) {
				if (SNR_db < *(SNR_LUT->data() + i)) {
					double y_low = *(BLER_LUT->data() + i - 1);
					double x_low = *(SNR_LUT->data() + i - 1);
					double diff = SNR_db - x_low;
					BLER_est = y_low + (c1s[i - 1] * diff)
							+ (c2s[i - 1] * diff * diff)
							+ (c3s[i - 1] * diff * diff * diff);
					break;
				} // end of if
			} // end of for
		} // end of else
	} // end of if
	return BLER_est;
} // end of function


void populate_common_params()
{
	
}

point ****STA_coord ;
point ***AP_coord;
void indoor_9_cell_layout(){
	// init uniform real distibution between [0,1]
	std::uniform_real_distribution<double> uni_distribution(-0.5, 0.5);
	// set random device generator
	std::random_device generator;
	bool inside = false;
		STA_coord = new point***[9];
	for (int i=0;i<9;i++){
			STA_coord[i] = new point**[common_params.n_AP];
		for (int j=0;j<common_params.n_AP;j++){
			  STA_coord[i][j] = new point*[common_params.n_STA];
			for (int k=0;k<common_params.n_STA;k++){
				point *coordi_temp = new point; 
				while(!inside){
					coordi_temp->xy[0] = uni_distribution(generator)*common_params.dimensions[0];
					coordi_temp->xy[1] = uni_distribution(generator)*common_params.dimensions[1];
					inside = is_inside_rectangle(common_params.dimensions, coordi_temp->xy);
				}
				STA_coord[i][j][k] = coordi_temp;
				inside = false;
			}
		}
	}
	
	AP_coord = new point**[9];
	for (int i=0;i<9;i++){
			AP_coord[i] = new point*[common_params.n_AP];
		for (int j=0;j<common_params.n_AP;j++){
			point *coordi_temp = new point;
				while(!inside){
					// AP will be at the centre with random offset between -2m to 2m
					coordi_temp->xy[0] = uni_distribution(generator)*4;
					coordi_temp->xy[1] = uni_distribution(generator)*4;
					inside = is_inside_rectangle(common_params.dimensions, coordi_temp->xy);
				}
				AP_coord[i][j] = coordi_temp;
				inside = false;
		}
	}
	
	// shift the coordinates to other cells
    
    for (int i=0; i<common_params.n_AP;i++){
		AP_coord[1][i]->xy[0]+= common_params.dimensions[0]; AP_coord[1][i]->xy[1]+= 0;
		AP_coord[2][i]->xy[0]-= common_params.dimensions[0]; AP_coord[2][i]->xy[1]+= 0;
		AP_coord[3][i]->xy[0]+= 0          				   ; AP_coord[3][i]->xy[1]+= common_params.dimensions[1];
		AP_coord[4][i]->xy[0]+= 0                          ; AP_coord[4][i]->xy[1]-= common_params.dimensions[1];
		AP_coord[5][i]->xy[0]+= common_params.dimensions[0]; AP_coord[5][i]->xy[1]+= common_params.dimensions[1];
		AP_coord[6][i]->xy[0]-= common_params.dimensions[0]; AP_coord[6][i]->xy[1]+= common_params.dimensions[1];
		AP_coord[7][i]->xy[0]+= common_params.dimensions[0]; AP_coord[7][i]->xy[1]-= common_params.dimensions[1];
		AP_coord[8][i]->xy[0]-= common_params.dimensions[0]; AP_coord[8][i]->xy[1]-= common_params.dimensions[1];
	}
    
    for (int i=0; i<common_params.n_AP;i++){
		for (int j=0; j<common_params.n_STA;j++){
		STA_coord[1][i][j]->xy[0]+= common_params.dimensions[0]; STA_coord[1][i][j]->xy[1]+= 0;
		STA_coord[2][i][j]->xy[0]-= common_params.dimensions[0]; STA_coord[2][i][j]->xy[1]+= 0;
		STA_coord[3][i][j]->xy[0]+= 0          				   ; STA_coord[3][i][j]->xy[1]+= common_params.dimensions[1];
		STA_coord[4][i][j]->xy[0]+= 0                          ; STA_coord[4][i][j]->xy[1]-= common_params.dimensions[1];
		STA_coord[5][i][j]->xy[0]+= common_params.dimensions[0]; STA_coord[5][i][j]->xy[1]+= common_params.dimensions[1];
		STA_coord[6][i][j]->xy[0]-= common_params.dimensions[0]; STA_coord[6][i][j]->xy[1]+= common_params.dimensions[1];
		STA_coord[7][i][j]->xy[0]+= common_params.dimensions[0]; STA_coord[7][i][j]->xy[1]-= common_params.dimensions[1];
		STA_coord[8][i][j]->xy[0]-= common_params.dimensions[0]; STA_coord[8][i][j]->xy[1]-= common_params.dimensions[1];
		}
	}
	
}
