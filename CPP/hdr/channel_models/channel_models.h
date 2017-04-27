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
/*! \file channel_models.h 
 * \brief channel models 
 * \author sreekanth dama <sreekanth@iith.ac.in>
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */
#ifndef CHANNEL_MODELS_H_  //header gaurd
#define CHANNEL_MODELS_H_
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

void PL_measure(std::vector<double> ACTIVE_devices, std::vector<double> *PL, double fc, std::string channel_type);

void InH(std::vector<double> ACTIVE_devices, std::vector<double> *PL, double fc);
void UMa(std::vector<double> ACTIVE_devices, std::vector<double> *PL, double fc);
void UMi(std::vector<double> ACTIVE_devices, std::vector<double> *PL, double fc);
void RMa(std::vector<double> ACTIVE_devices, std::vector<double> *PL, double fc);


#endif
