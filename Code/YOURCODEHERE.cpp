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
int bestIndex[NUM_DIMS-NUM_DIMS_DEPENDENT] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool currentDimDone = false;
bool isDSEComplete = false;

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

	/*
	0 = width=( "1" "2" "4" "8" ) 
	1 = fetchspeed=( "1" "2" ) 
	2 = scheduling=( "-issue:inorder true -issue:wrongpath false" "-issue:inorder false -issue:wrongpath true" ) 
	3 = ruusize=( "4" "8" "16" "32" "64" "128" ) 
	4 = lsqsize=( "4" "8" "16" "32" )
	5 = memport=( "1" "2" )
	6 = dl1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
	7 = dl1assoc=( "1" "2" "4" )
	8 = il1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
	9 = il1assoc=( "1" "2" "4" )
	10 = ul2sets=( "256" "512" "1024" "2048" "4096" "8192" "16384" "32768" "65536" "131072" )
	11 = ul2block=( "16" "32" "64" "128" )
	12 = ul2assoc=( "1" "2" "4" "8" "16" ) 
	13 = tlbsets=( "4" "8" "16" "32" "64" )
	14 = dl1lat=( "1" "2" "3" "4" "5" "6" "7" )
	15 = il1lat=( "1" "2" "3" "4" "5" "6" "7" )
	16 = ul2lat=( "5" "6" "7" "8" "9" "10" "11" "12" "13" )
	17 = branchsettings=( "-bpred perfect" "-bpred nottaken" "-bpred bimod -bpred:bimod 2048" "-bpred 2lev -bpred:2lev 1 1024 8 0" "-bpred 2lev -bpred:2lev 4 256 8 0" "-bpred comb -bpred:comb 1024" )
	*/


	width = 8 * pow(2, extractConfigPararm(configuration,0));
	L1dblockSize = 8 * (1 << extractConfigPararm(configuration, 2));
    L1iblockSize = 8 * (1 << extractConfigPararm(configuration, 2));
    L2blocksize = 16 << extractConfigPararm(configuration, 8);

    L1dSize = getdl1size(configuration) / 1024;
    L1iSize = getil1size(configuration) / 1024;
    L2Size = getl2size(configuration) / 1024;

	//check conditions


	 if(isNumDimConfiguration(configuration) == 1){
        valid++;
    }
    if(width == L1dblockSize){
        valid++;
    }
    if(L1dblockSize == L1iblockSize){
        valid++;
    }
    if(L2blocksize >= 2 * L1iblockSize && L2blocksize <= 128){
        valid++;
    }
    if(L1iSize >= 2 && L1iSize <= 64){
        valid++;
    }
    if(L2Size >= 32 && L2Size <= 1024){
        valid++;
	}

	//valid return 1 invalid return 0

	if (valid == 6){

		return 1;
	}

	else {

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

	/*
	0 = width=( "1" "2" "4" "8" ) 
	1 = fetchspeed=( "1" "2" ) 
	2 = scheduling=( "-issue:inorder true -issue:wrongpath false" "-issue:inorder false -issue:wrongpath true" ) 
	3 = ruusize=( "4" "8" "16" "32" "64" "128" ) 
	4 = lsqsize=( "4" "8" "16" "32" )
	5 = memport=( "1" "2" )
	6 = dl1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
	7 = dl1assoc=( "1" "2" "4" )
	8 = il1sets=( "32" "64" "128" "256" "512" "1024" "2048" "4096" "8192" )
	9 = il1assoc=( "1" "2" "4" )
	10 = ul2sets=( "256" "512" "1024" "2048" "4096" "8192" "16384" "32768" "65536" "131072" )
	11 = ul2block=( "16" "32" "64" "128" )
	12 = ul2assoc=( "1" "2" "4" "8" "16" ) 
	13 = tlbsets=( "4" "8" "16" "32" "64" )
	14 = dl1lat=( "1" "2" "3" "4" "5" "6" "7" )

	Dependents:
	15 = il1lat=( "1" "2" "3" "4" "5" "6" "7" )
	16 = ul2lat=( "5" "6" "7" "8" "9" "10" "11" "12" "13" )
	17 = branchsettings=( "-bpred perfect" "-bpred nottaken" "-bpred bimod -bpred:bimod 2048" "-bpred 2lev -bpred:2lev 1 1024 8 0" "-bpred 2lev -bpred:2lev 4 256 8 0" "-bpred comb -bpred:comb 1024" )
	*/
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

		bool best = false;
		if (optimizeforEXEC == 1){
			bestConfig = bestEXECconfiguration;
			best = true;
		}
			

		if (optimizeforEDP == 1){
			bestConfig = bestEDPconfiguration;
			best = true;
		}
		
		if (best){
			bestIndex[currentlyExploringDim] = 1;
			currentDimIndex +=1;
		} 
			

		// // Fill in the dimensions already-scanned with the already-selected best
		// // value.
		// for (int dim = 0; dim < currentlyExploringDim; ++dim) {
		// 	ss << extractConfigPararm(bestConfig, dim) << " ";
		// }

		// // Handling for currently exploring dimension. This is a very dumb
		// // implementation.
		// int nextValue = extractConfigPararm(nextconfiguration,
		// 		currentlyExploringDim) + 1;

		// if (nextValue >= GLOB_dimensioncardinality[currentlyExploringDim]) {
		// 	nextValue = GLOB_dimensioncardinality[currentlyExploringDim] - 1;
		// 	currentDimDone = true;
		// }

		// ss << nextValue << " ";

		// // Fill in remaining independent params with 0.
		// for (int dim = (currentlyExploringDim + 1);
		// 		dim < (NUM_DIMS - NUM_DIMS_DEPENDENT); ++dim) {
		// 	ss << "0 ";
		// }

		for (int dim = 0;
				dim < (NUM_DIMS - NUM_DIMS_DEPENDENT); ++dim) {
					if (dim == currentDimIndex){
						if (nextconfiguration[currentlyExploringDim] == GLOB_dimensioncardinality[currentlyExploringDim]-1)
							currentDimDone = true;
						int nextValue = extractConfigPararm(nextconfiguration, currentlyExploringDim) + 1;
						ss << nextValue << " ";
					} else if (bestIndex[dim]==1){
						ss << extractConfigPararm(bestConfig, dim) << " ";
					} else{
						ss << "0 ";
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
			currentDimIndex++;
			currentDimDone = false;
		}

		// Signal that DSE is complete after this configuration.
		if (currentDimIndex == (NUM_DIMS - NUM_DIMS_DEPENDENT))
			isDSEComplete = true;
		
	}
	return nextconfiguration;
}


