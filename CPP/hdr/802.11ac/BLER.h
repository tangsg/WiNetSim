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
/*! \file BLER.h 
 * \brief BLER Tables for 802.11ac 
 * \author sreekanth dama
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#ifndef BLER_H_  //header gaurd
#define BLER_H_
#include "common.h"

//! BLER table for 8 MCS in WiFi
std::vector<double> AWGN_11ac_8_MCS_BLER[9] = { { 1, 0.3387, 0.0221, 0.0070,
		0.004 }, { 1, 0.5580, 0.1480, 0.0241, 0.001 }, { 0.97, 0.611, 0.168,
		0.022, 0.001 }, { 0.99, 0.845, 0.359, 0.067, 0.008 }, { 0.964, 0.53,
		0.145, 0.021, 0.001 }, { 0.953, 0.482, 0.122, 0.022, 0.003 }, { 0.776,
		0.281, 0.064, 0.0090, 0.001 }, { 0.983, 0.614, 0.177, 0.0290, 0.004 },
		{ 0.983, 0.614, 0.177, 0.0290, 0.004 } };
//! SNR corresponding to the above table
std::vector<double> AWGN_11ac_8_MCS_SNR[9] = { { 0.7, 1.7, 2.7, 3.7, 4.7 }, {
		1.7, 2.7, 3.7, 4.7, 5.7 }, { 3.7, 4.7, 5.7, 6.7, 7.7 }, { 5.7, 6.7, 7.7,
		8.7, 9.7 }, { 9.7, 10.7, 11.7, 12.7, 13.7 }, { 13.7, 14.7, 15.7, 16.7,
		17.7 }, { 15.7, 16.7, 17.7, 18.7, 19.7 },
		{ 16.7, 17.7, 18.7, 19.7, 20.7 },
		{ 21.7, 22.7, 23.7, 24.7, 25.7 }  };

#endif
