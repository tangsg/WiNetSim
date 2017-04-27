# WiNetSim
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
Matlab version is very slow.    
Don't run large networks on Matlab.     
Use CPP version to run large networks like more than 100 nodes.     
There is a multi threaded version available to run on a compute cloud.      
Our experiments suggest single threaded version runs faster than a multi threaded version if your cpu has only 4 cores.
