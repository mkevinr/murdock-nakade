/*
 * pfagent.cpp
 *
 *  Created on: May 2, 2014
 *      Author: rnakade
 */


#define _CRT_SECURE_NO_DEPRECATE 1
#include <iostream>
#include "pfagent.h"
using namespace std;

const char *kDefaultServerName = "localhost";
const int kDefaultServerPort = 50104;

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
	pf_agent(MyTeam);
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

void pf_agent(BZRC MyTeam)
{
	bool flag_captured = false;
	while(!flag_captured)
	{
		// Go Towards Flag (Attractive field - Seek Goal)
		vector<flag_t> flags;
		MyTeam.get_flags(&flags);
		vector<tank_t> tanks;
		MyTeam.get_mytanks(&tanks);
		tank_t agent = tanks[0];
		flag_t destination = flags[1];
		double distToGoal[] = {destination.pos[0] - agent.pos[0]
				, destination.pos[1] - agent.pos[1]};
		double dist = sqrt((distToGoal[0]*distToGoal[0] + distToGoal[1] * distToGoal[1]));
		double theta = atan2(distToGoal[1], distToGoal[0]);
		double angVelConst = 1;
		MyTeam.angvel(0, angVelConst * (theta - agent.angle));
		double alpha = 1;
		MyTeam.shoot(0);
		MyTeam.speed(0, alpha * dist);


		// Avoid Other Tanks
		vector <otank_t> other_tanks;
		MyTeam.get_othertanks(&other_tanks);
		for(int i = 0; i<other_tanks.size() ; i++)
		{
			double distToTank[] = {other_tanks[i].pos[0] - agent.pos[0]
					, other_tanks[i].pos[1] - agent.pos[1]};
			double dist = sqrt((distToTank[0]*distToTank[0] + distToTank[1] * distToTank[1]));
			double theta = atan2(distToTank[1], distToTank[0]);
			if(dist < 10)
			{
				double angVelConst = -0.1;
				MyTeam.angvel(0, angVelConst * (theta - agent.angle));
				double alpha = -0.01;
				MyTeam.shoot(0);
				MyTeam.speed(0, alpha * dist);
			}

		}

		// Avoid your own Tanks
		for(int i = 1; i<tanks.size() ; i++)
		{
			double distToTank[] = {tanks[i].pos[0] - agent.pos[0]
					, tanks[i].pos[1] - agent.pos[1]};
			double dist = sqrt((distToTank[0]*distToTank[0] + distToTank[1] * distToTank[1]));
			double theta = atan2(distToTank[1], distToTank[0]);
			if(dist < 10)
			{
				double angVelConst = -0.1;
				MyTeam.angvel(0, angVelConst * (theta - agent.angle) );
				double alpha = -0.1;
				MyTeam.shoot(0);
				MyTeam.speed(0, alpha * dist);
			}
		}

		//Check if flag is captured
		if(destination.poss_color.compare("blue") == 0)
		{
			flag_captured = true;

		}
	}
	printf("Captured the flag\n");
	//Come back to base after getting the flag
	/*
	bool at_base = false;
	while(!at_base)
	{

	}
	*/

}

