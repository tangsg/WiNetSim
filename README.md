# WiNetSim
Welcome to the Wireless Network Simulator (WiNetSim)
**Current Release**   `v0.0`    
supports Wifi 11ac 1 tx 1 rx over 20MHz channel BW   
provides both matlab, c++ and c++ multi threaded (for compute servers)   
Doxygen documentation https://commlabiithyderabad.github.io/WiNetSim/

## Future releases
Support for All 802.11 and Mm Wave specs

## Folders
+CPP  
+Matlab 


## Instructions to run CPP

`cd CPP`    
`mkdir build`   
`cd build`  
`cmake ../`     
`make`  
`./WiNetSim`


## Instructions to run Matlab
open Matlab

`cd Matlab`     
`main.m`

Refer the wiki <https://github.com/CommLabIITHyderabad/WiNetSim/wiki> for more info   
**Note:** Matlab version is very slow.    
Do not run large networks on Matlab.     
Use CPP version to run large networks like more than 100 nodes.     
There is a multi threaded version available to run on a compute cloud.      
Our experiments suggest single threaded version runs faster than a multi threaded version if your cpu has only 4 cores.
