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
/*! \file Indoor_hotspot.cpp
 * \brief Indoor hotspot Model
 * \author sreekanth dama sreekanth@iith.ac.in
 * \date 2016
 * \version 0.0
 * \note
 * \warning
 */

#include "channel_models.h"
/*! Indoor Hotspot model \n 
 * taked from 3GPP TS "36.814 Table B.1.2.1-1 Summary table of the primary module path loss models \n
 * Valid for small indoor room layouts with maximum node distance of 150m from AP \n
 * Typical cell layouts 100mX100m \n
 * \param ACTIVE_devices vector containing distance in meters \n
 * \param PL_out pointer to empty vector \n
 * \param fc frequency in GHz \n 
 */
void InH(std::vector<double> ACTIVE_devices, std::vector<double> *PL_out,
		double fc) {
	for (auto it = ACTIVE_devices.begin(); it != ACTIVE_devices.end(); ++it)

	{
		*it = *it < 3 ? 3.1 : *it; // to make sure the distance is more than 3m
		// Raise a warning if the cell size is more than 150m
		/*if (*it > 150) {
			std::cout
					<< "_#_#_#_WARNING_#_#_#: d>150 \n ### InH MODEL applicable only for cells of size < 150m ###"
					<< std::endl;
		}*/
		// probability of LOS
		double Pr_LOS = 0;
		if (*it <= 18)
			Pr_LOS = 1;
		else if (*it > 18 && *it < 37)
			Pr_LOS = std::exp(-(*it - 18) / 27);
		else if (*it >= 37)
			Pr_LOS = 0.5;
		// LOS is binomial random variable taking 0(NLOS) or 1(LOS)
		std::binomial_distribution<int> bi_distribution(1, Pr_LOS);
		// random device uses the system entropy
		std::random_device generator;
		// Calculate LOS
		int LOS = bi_distribution(generator);
		// LOS is applicable only in the range 3m-100m
		if (*it > 100) {
			LOS = 0;
		}
		// Path loss in dB	
		double PL = 0;
		if (LOS == 1) {
			PL = 16.9 * std::log10(*it) + 32.8 + 20 * std::log10(fc);
		} else {
			PL = 43.3 * std::log10(*it) + 11.5 + 20 * std::log10(fc);
		}
		// Shadowing Model is lognormal
		double PL_shd = 0;
		if (LOS == 1) {
			// Gaussian distribution with mean 0 and variance 3 for LOS
			std::normal_distribution<double> norm_distribution(0, 3.0);
			PL_shd = norm_distribution(generator);
		} else {
			// Gaussian distribution with mean 0 and variance 4 for NLOS
			std::normal_distribution<double> norm_distribution(0, 4.0);
			PL_shd = norm_distribution(generator);
		}

		PL_out->push_back(-PL + PL_shd);
	}
}
