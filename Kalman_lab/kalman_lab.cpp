/*
 * kalman_lab.cpp
 *
 *  Created on: May 31, 2014
 *      Author: rnakade
 */

#include "kalman_lab.h"

using namespace std;

const char *kDefaultServerName = "localhost";
const int kDefaultServerPort = 50100;


int main(int argc, char *argv[])
{

	const char *pcHost;
	int nPort;

	if(argc < 2) {
		pcHost = kDefaultServerName;
	}
	else {
		pcHost = argv[1];
	}
	if(argc < 3) {
		nPort = kDefaultServerPort;
	}
	else {
		nPort = atoi(argv[2]);
	}

	BZRC MyTeam = BZRC(pcHost, nPort, false);
	if(!MyTeam.GetStatus()) {
		cout << "Can't connect to BZRC server." << endl;
		exit(1);
	}

	// Calling agent code
	conforming_agent(MyTeam);
	non_conforming_agent(MyTeam);

	kalman_agent(MyTeam);

	MyTeam.Close();


	return 1;
}

void conforming_agent(BZRC MyTeam)
{
	MyTeam.speed(0, 1);
}

void non_conforming_agent(BZRC MyTeam)
{
	int random_speed, random_angvel;
	while(true)
	{
		random_speed = (rand() % 100 ) / 100;
		random_angvel = (rand() % 100 ) / 100;
		MyTeam.speed(1, random_speed);
		MyTeam.angvel(1, random_angvel);
	}

}

void kalman_agent(BZRC MyTeam)
{

}
