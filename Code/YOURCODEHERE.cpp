#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <math.h>
#include <fcntl.h>
#include <vector>
#include <iterator>

#include "431project.h"

using namespace std;



/*
 * Enter your PSU IDs here to select the appropriate scanning order.
 */
#define PSU_ID_SUM (906087277+961603255)
// (906087277+961603255) mod 24 = 20 -> FPU Cache BP Core

/*
 * Some global variables to track heuristic progress.
 * 
 * Feel free to create more global variables to track progress of your
 * heuristic.
 */

// Exploration order based on the types from PDF
int EXPLORE[NUM_DIMS-NUM_DIMS_DEPENDENT] = {11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 0, 1};
int currentDimIndex = 0;
unsigned int currentlyExploringDim = EXPLORE[currentDimIndex];
string bestConfig;
int doneDim[NUM_DIMS-NUM_DIMS_DEPENDENT] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
// int bestIndex[NUM_DIMS-NUM_DIMS_DEPENDENT] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
bool currentDimDone = false;
bool isDSEComplete = false;
bool firstBest = false;

/*
 * Given a half-baked configuration containing cache properties, generate
 * latency parameters in configuration string. You will need information about
 * how different cache paramters affect access latency.
 * 
 * Returns a string similar to "1 1 1"
 */
std::string generateCacheLatencyParams(string halfBackedConfig) {

	string latencySettings;

	//
	//YOUR CODE BEGINS HERE
	//
	//cacheSize = num_Sets*assoc*blockSize;

	int L1dSize, L1iSize, L2Size;
	int L1d_lat_dm,L1i_lat_dm,L2_lat_dm;

	vector <int> token;
	

	stringstream check1(halfBackedConfig);

	string temp;

	while (getline(check1,temp,' '))
	{
		token.push_back(atoi(temp.c_str()));
	}
	

	L1dSize=getdl1size(halfBackedConfig)/1024;
	L1iSize=getil1size(halfBackedConfig)/1024;
	L2Size=getl2size(halfBackedConfig)/1024;

	L1d_lat_dm = log2(L1dSize);
	L1i_lat_dm = log2(L1iSize);
	L2_lat_dm = log2(L2Size);

	if (token[4] ==1) {

		L1d_lat_dm +=1;
	}

	if (token[4] ==2) {

		L1d_lat_dm += 2;
	}

	if (token[6] ==1){

		L1i_lat_dm +=1;
	}

	if (token[6] ==2){

		L1i_lat_dm +=2;
	}

	if(token[9]==1){
		L2_lat_dm +=1;
	}

	if (token[9] ==2){

		L2_lat_dm +=2;
	}

	

	

	 




	// This is a dumb implementation.
	latencySettings = "1 1 1";

	latencySettings[0] = L1d_lat_dm -1 +'0';
	latencySettings[2] = L1i_lat_dm -1 + '0';
	latencySettings[4] = L2_lat_dm - 5 + '0';




	//
	//YOUR CODE ENDS HERE
	//

	return latencySettings;
}

/*
 * Returns 1 if configuration is valid, else 0
 */
int validateConfiguration(std::string configuration) {

	// FIXME - YOUR CODE HERE

	int L1dblockSize, L1iblockSize, L2blocksize, L1dSize, L1iSize, L2Size, width;

	int valid =0;

	//get design parameter


	width = 8 * pow(2, extractConfigPararm(configuration,0));
	L1dblockSize = 8 * (1 << extractConfigPararm(configuration, 2));
    L1iblockSize = 8 * (1 << extractConfigPararm(configuration, 2));
    L2blocksize = 16 << extractConfigPararm(configuration, 8);

    L1dSize = getdl1size(configuration) / 1024;
    L1iSize = getil1size(configuration) / 1024;
    L2Size = getl2size(configuration) / 1024;

	//check conditions


	// if(isNumDimConfiguration(configuration) == 1){
    //     valid++;
    // } else {
	// 	return 0;
	// }
    if(L1dblockSize % width == 0){
        valid++;
		cout << "valid 1 ";
    }
    if(L1dblockSize == L1iblockSize){
        valid++;
		cout << "valid 2 " ;
    }
    if(L2blocksize >= 2 * L1iblockSize && L2blocksize <= 128){
        valid++;
		cout << "valid 3 " ;
    }
    if(L1iSize >= 2 && L1iSize <= 64){
        valid++;
		cout << "valid 4 " ;
    }
    if(L2Size >= 32 && L2Size <= 1024){
        valid++;
		cout << "valid 5 " <<endl;
	}

	//valid return 1 invalid return 0

	if (valid != 5){

		return 0;
	}






	// The below is a necessary, but insufficient condition for validating a
	// configuration.
	return isNumDimConfiguration(configuration);
}

/*
 * Given the current best known configuration, the current configuration,
 * and the globally visible map of all previously investigated configurations,
 * suggest a previously unexplored design point. You will only be allowed to
 * investigate 1000 design points in a particular run, so choose wisely.
 *
 * In the current implementation, we start from the leftmost dimension and
 * explore all possible options for this dimension and then go to the next
 * dimension until the rightmost dimension.
 */
std::string generateNextConfigurationProposal(std::string currentconfiguration,
		std::string bestEXECconfiguration, std::string bestEDPconfiguration,
		int optimizeforEXEC, int optimizeforEDP) {

	//
	// Some interesting variables in 431project.h include:
	//
	// 1. GLOB_dimensioncardinality
	// 2. GLOB_baseline = "0 0 0 5 0 5 0 2 2 2 0 1 0 1 2 2 2 5"
	// 3. NUM_DIMS = 18
	// 4. NUM_DIMS_DEPENDENT = 3
	// 5. GLOB_seen_configurations 

	// Exploration order: FPU Cache BP Core
	// Cache is 2-10, fpu is 11, BP is 12 -14, and core is 0 to 1. 
	std::string nextconfiguration = currentconfiguration;


	// Continue if proposed configuration is invalid or has been seen/checked before.
	while (!validateConfiguration(nextconfiguration) ||
		GLOB_seen_configurations[nextconfiguration]) {

		// Check if DSE has been completed before and return current
		// configuration.
		if(isDSEComplete) {
			return currentconfiguration;
		}

		
		// Exploring different parameters and continue when there is a change. 
		// for (int i = 0; i < NUM_DIMS - NUM_DIMS_DEPENDENT; i++){
		// 	if (nextconfiguration[EXPLORE[i]] != GLOB_dimensioncardinality[EXPLORE[i]] - 1){
		// 		nextconfiguration[EXPLORE[i]] += 1;
		// 		GLOB_seen_configurations[nextconfiguration] = 1;
		// 		break;
		// 	}
		// }

		std::stringstream ss;

		// bool best = false;
		if (optimizeforEXEC == 1){
			bestConfig = bestEXECconfiguration;
		}
			

		if (optimizeforEDP == 1){
			bestConfig = bestEDPconfiguration;
		}
		
		// if (best){
		// 	bestIndex[currentlyExploringDim] = extractConfigPararm(bestConfig, currentlyExploringDim);
		// } 
			

		// // Fill in the dimensions already-scanned with the already-selected best
		// // value.
		// for (int dim = 0; dim < currentDimIndex; ++dim) {
		// 	ss << extractConfigPararm(bestConfig, EXPLORE[dim]) << " ";
		// }

		// // Handling for currently exploring dimension. This is a very dumb
		// // implementation.
		// int nextValue = extractConfigPararm(nextconfiguration,
		// 		currentlyExploringDim) + 1;

		// if (nextValue >= GLOB_dimensioncardinality[currentlyExploringDim]) {
		// 	nextValue = GLOB_dimensioncardinality[currentlyExploringDim] - 1;
		// 	currentDimDone = true;
		// } else {
		// 	ss << nextValue << " ";
		// }

		// // Fill in remaining independent params with 0.
		// for (int dim = (currentlyExploringDim + 1);
		// 		dim < (NUM_DIMS - NUM_DIMS_DEPENDENT); ++dim) {
		// 	ss << "0 ";
		// }

		for (int i = 0;
				i < (NUM_DIMS - NUM_DIMS_DEPENDENT); ++i) {
					if (doneDim[i] == 1){
						// Explored and have best dim. 
						ss << extractConfigPararm(bestConfig, i) <<" ";
						// cout << "Adding best dim " << " " <<ss.str()<<endl;
					} else {
						if (i == currentlyExploringDim){
							// check start from 0 since baseline does not
							if (doneDim[i] == -1 && extractConfigPararm(currentconfiguration, i) != 0){
							ss << "0 ";
							doneDim[i] = 0;
							} else {
								// increment value by 1
								int nextValue = extractConfigPararm(nextconfiguration, i) + 1;
								if (nextValue >= GLOB_dimensioncardinality[i]){
									// if exccedds, reset the dim
									nextValue = GLOB_dimensioncardinality[i] - 1;
									currentDimDone = true;
								}
								ss << nextValue << " "; 
								// cout << "Adding current dim" << i <<endl;
							}
						} else {
							// adding rest dims from previous
							ss << extractConfigPararm(currentconfiguration, i) << " ";
							// cout << "Adding rest dim " << i <<endl;
						}
					}
		}
		
		//
		// Last NUM_DIMS_DEPENDENT3 configuration parameters are not independent.
		// They depend on one or more parameters already set. Determine the
		// remaining parameters based on already decided independent ones.
		//
		string configSoFar = ss.str();

		// Populate this object using corresponding parameters from config.
		ss << generateCacheLatencyParams(configSoFar);

		// Configuration is ready now.
		nextconfiguration = ss.str();

		// Make sure we start exploring next dimension in next iteration.
		if (currentDimDone) {
			doneDim[currentlyExploringDim] = 1;
			currentDimIndex += 1;
			currentlyExploringDim = EXPLORE[currentDimIndex];
			currentDimDone = false;
		}

		// Signal that DSE is complete after this configuration.
		if (currentDimIndex == (NUM_DIMS - NUM_DIMS_DEPENDENT)){
			if (firstBest)
				isDSEComplete = true;
			else{
				for (int i=0; i<NUM_DIMS-NUM_DIMS_DEPENDENT; i++)
					doneDim[i] = -1;
				currentDimIndex = 0;
				firstBest = true;
			}
		}
		cout << nextconfiguration <<endl;
		cout << !validateConfiguration(nextconfiguration) <<" " <<GLOB_seen_configurations[nextconfiguration] <<endl;
	}
	
	return nextconfiguration;
}


