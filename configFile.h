#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

/*
Header file for CONFIG struct which obtains and stores values
 for the simulation from the config file: "des_config.cnf".

by: Sean Reddington
*/

//CONFIG struct for storing data read from the config file
typedef struct CONFIG {

	const unsigned SEED;
	const unsigned INIT_TIME;
	const unsigned FIN_TIME;
	const unsigned ARRIVE_MIN;
	const unsigned ARRIVE_MAX;
	const float QUIT_PROB;
	const unsigned CPU_MIN;
	const unsigned CPU_MAX;
	const unsigned DISK1_MIN;
	const unsigned DISK1_MAX;
	const unsigned DISK2_MIN;
	const unsigned DISK2_MAX;

} CONFIG;


//Reads values from config file into CONFIG struct
CONFIG readConfigFile() {

	CONFIG configFile;
	FILE *inPtr;

	if ((inPtr = fopen("des_config.cnf", "r")) == NULL) {
		puts("Config file could not be opened");
	}
	else {
		fscanf(inPtr, "SEED%u\n"
			, &configFile.SEED);

		fscanf(inPtr, "INIT_TIME%u\nFIN_TIME%u\n"
			, &configFile.INIT_TIME, &configFile.FIN_TIME);

		fscanf(inPtr, "ARRIVE_MIN%u\nARRIVE_MAX%u\n"
			, &configFile.ARRIVE_MIN, &configFile.ARRIVE_MAX);

		fscanf(inPtr, "QUIT_PROB%f\n", &configFile.QUIT_PROB);

		fscanf(inPtr, "CPU_MIN%u\nCPU_MAX%u\n"
			, &configFile.CPU_MIN, &configFile.CPU_MAX);

		fscanf(inPtr, "DISK1_MIN%u\nDISK1_MAX%u\n"
			, &configFile.DISK1_MIN, &configFile.DISK1_MAX);

		fscanf(inPtr, "DISK2_MIN%u\nDISK2_MAX%u\n"
			, &configFile.DISK2_MIN, &configFile.DISK2_MAX);
	}

	return configFile;
	fclose(inPtr);
}
#endif