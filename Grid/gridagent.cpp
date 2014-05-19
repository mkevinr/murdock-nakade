/*
 * pfagent.cpp
 *
 *  Created on: May 2, 2014
 *      Author: rnakade
 */


#define _CRT_SECURE_NO_DEPRECATE 1
#include <iostream>
#include <math.h>
#include <limits>
#include <string.h>
#include <vector>

#include "gridagent.h"
#include "grid_view.h"

using namespace std;

const double pi = 3.141592653589779;

const char *kDefaultServerName = "localhost";
const int kDefaultServerPort = 34937;

void get_nearest_point(double[2], double[2], double[2], double[2]);

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
	grid_agent(MyTeam, &argc, argv);
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

void get_obstacle_center(obstacle_t obstacle, double center[2])
{
	for(int i = 0; i < obstacle.courner_count; i++)
	{
		center[0] += obstacle.o_corner[i][0];
		center[1] += obstacle.o_corner[i][1];
	}
	center[0] /= obstacle.courner_count;
	center[1] /= obstacle.courner_count;
}

double normalize_angle(double angle)
{
	angle -= 2 * pi * (int) (angle / (2 * pi));
	if(angle <= -pi)
	{
		angle += 2 * pi;
	}
	else if(angle > pi)
	{
		angle -= 2 * pi;
	}

	return angle;
}

void grid_agent(BZRC MyTeam, int * argc, char ** argv)
{
	init_grid();//
	GLfloat test_grid[480][640];
	for(int i = 0; i < 480; i++)
	{
		for(int j = 0; j < 640; j++)
		{
			grid[i][j] = 0.5;
		}
	}
	init_window(argc, argv);
	vector<obstacle_t> obstacles;
	MyTeam.get_obstacles(&obstacles);

	vector<flag_t> flags;
	MyTeam.get_flags(&flags);
	flag_t destination = get_flag(flags, "red");
	cout << "destination flag color: " << destination.color << endl;
	Vec2 destination_pos = Vec2(destination.pos);
	double alpha = 1;
	double beta = 1;

	int step = 40;
	double dist = 0;
	double speed = 0;
	double repulsiveRadius = 20;
	double angleToFlag = 0;
	Vec2 coordinate;

	bool flag_captured = false;
	string oppose_color = "red";
	string my_color = "green";
	vector<tank_t> tanks;
	while(!flag_captured)
	{
		update_grid(test_grid);
		draw_grid();
		MyTeam.shoot(0);
		flags.clear();
		tanks.clear();
		MyTeam.get_flags(&flags);
		MyTeam.get_mytanks(&tanks);
		tank_t agent = tanks[0];
		// If the current tank has a flag
		if(agent.flag.length() > 2)
		{
			destination = get_flag(flags, my_color);
		}
		else
		{
			destination = get_flag(flags, oppose_color);
		}
		destination_pos = Vec2(destination.pos);
		// Go Towards Flag (Attractive field - Seek Goal)
		double dist = get_distance(destination.pos,agent.pos);
		double attractiveAngle = normalize_angle(get_angle(agent.pos, destination.pos) - agent.angle);
		double attractiveSpeed = dist;
		double angVelConst = 1;

		//MyTeam.speed(0,attractiveSpeed);
		//MyTeam.angvel(0,attractiveAngle);

		// Avoid obstacles
		Vec2 near;
		Vec2 nearest;
		double nearestDist = numeric_limits<double>::max();;
		obstacle_t obstacle;
		for(int k = 0; k < obstacles.size(); k++)
		{
			obstacle = obstacles[k];
			/*get_obstacle_center(obstacle, near);
			if(dist < nearestDist)
			{
				nearestDist = dist;
				nearest[0] = near[0];
				nearest[1] = near[1];
			}*/
			for(int l = 0; l < obstacle.courner_count; l++)
			{
				near = get_nearest_point(Vec2(obstacle.o_corner[l])
					, Vec2(obstacle.o_corner[(l + 1) % obstacle.courner_count]), Vec2(agent.pos));
				dist = get_distance(Vec2(agent.pos), near);
				if(dist < nearestDist)
				{
					nearestDist = dist;
					nearest = Vec2(near);
				}
			}
		}

		double repulsiveAngle = 0;
		double tangentialAngle = 0;

		if(nearestDist < repulsiveRadius)
		{
			repulsiveAngle = normalize_angle(get_angle(agent.pos, nearest));
			tangentialAngle = normalize_angle(get_angle(agent.pos, nearest) + pi / 2);
		}

		attractiveAngle = normalize_angle(attractiveAngle);
		//cout << "attractive angle: " << attractiveAngle << endl;
		repulsiveAngle = normalize_angle(repulsiveAngle);
		//MyTeam.angvel(0, attractiveAngle + (repulsiveAngle - agent.angle));
		//MyTeam.speed(0, attractiveSpeed - beta * repulsiveSpeed);
		MyTeam.angvel(0, normalize_angle(attractiveAngle + repulsiveAngle));
		MyTeam.speed(0, attractiveSpeed);
		//MyTeam.speed(0, attractiveSpeed - beta * repulsiveSpeed);



		// Avoid Other Tanks
		/*vector <otank_t> other_tanks;
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
		}*/

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

flag_t get_flag(vector<flag_t> flags, string color)
{
	for(int i = 0; i < 4; i++)
	{
		if(flags[i].color.compare(color) == 0)
		{
			return flags[i];
		}
	}

	return flag_t();
}

double get_distance_squared(Vec2 a, Vec2 b)
{
	double xDiff = b.x - a.x;
	double yDiff = b.y - a.y;
	return xDiff * xDiff + yDiff * yDiff;
}


// This code adapted from http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
Vec2 get_nearest_point(Vec2 v, Vec2 w, Vec2 p) {
  // Return the closest point from p on line segment vw
  const double l2 = get_distance_squared(v, w);  // i.e. |w-v|^2 -  avoid a sqrt
  if (l2 == 0.0)
  {
	  return v;   // v == w case
  }
  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of point p onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2
  const double t = (p - v).dot(w - v) / l2;
  if (t < 0.0)
  {
	  return v;       // Beyond the 'v' end of the segment
  }
  else if (t > 1.0)
  {
	  return w;  // Beyond the 'w' end of the segment
  }
  return v + (w - v) * t;  // Projection falls on the segment
}

double get_distance(Vec2 a, Vec2 b)
{
	double xDiff = b.x - a.x;
	double yDiff = b.y - a.y;
	return sqrt(xDiff * xDiff + yDiff * yDiff);
}

double get_angle(Vec2 a, Vec2 b)
{
	double xDiff = b.x - a.x;
	double yDiff = b.y - a.y;
	return atan2(yDiff, xDiff);
}
