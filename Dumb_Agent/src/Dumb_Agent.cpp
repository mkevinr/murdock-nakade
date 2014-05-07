//============================================================================
// Name        : Dumb_Agent.cpp
// Author      : rnakade
// Version     :
// Copyright   : Your copyright notice
// Description : Dumb Agent in C++, Ansi-style
//============================================================================


#define _CRT_SECURE_NO_DEPRECATE 1
#include <iostream>
#include "Dumb_Agent.h"
using namespace std;

const char *kDefaultServerName = "localhost";
const int kDefaultServerPort = 4000;

int main(int argc, char *argv[]) {
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
	world_init(&MyTeam);
	dumb_agent(MyTeam);

	MyTeam.Close();
	//free(&MyTeam);
	return 0;
}

void world_init(BZRC *my_team){};


void dumb_agent(BZRC MyTeam)
{
	for(int i=1; i>0; i++)
	{
		MyTeam.speed(1,1.0);
		MyTeam.speed(2,1.0);
		MyTeam.shoot(1);
		MyTeam.shoot(2);
		sleep(5);
		MyTeam.speed(1,0);
		MyTeam.speed(2,0);
		MyTeam.angvel(1, 1);
		MyTeam.angvel(2, 1);
		sleep(3);
		MyTeam.angvel(1, 0);
		MyTeam.angvel(2, 0);
		MyTeam.shoot(1);
		MyTeam.shoot(1);
	}
}
