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
const int kDefaultServerPort = 50103;

float true_positive = 0.97;
float true_negative = 0.9;

float grid_probabilities[grid_height][grid_width];
GLfloat grid[grid_height][grid_width];

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

void init_grid()
{
	for(int i = 0; i < grid_height; i++)
	{
		for(int j = 0; j < grid_width; j++)
		{
			grid[i][j] = .5;
			grid_probabilities[i][j] = .5;
		}
	}
}

void grid_agent(BZRC MyTeam, int * argc, char ** argv)
{
	init_window(argc, argv);
	init_grid();
	update_grid(grid);

	double repulsiveRadius = 20;
	Vec2 coordinate;

	string my_color = "blue";
	vector<flag_t> flags;
	MyTeam.get_flags(&flags);
	flag_t my_flag = get_flag(flags, "blue");
	Vec2 my_flag_pos(my_flag.pos);
	vector<tank_t> tanks;
	MyTeam.get_mytanks(&tanks);
	int num_tanks = 2;
	if(tanks.size() < 2)
	{
		num_tanks = tanks.size();
	}
	tanks.clear();
	Vec2 destination[num_tanks];
	bool first_destination[num_tanks];
	for(int i = 0; i < num_tanks; i++)
	{
		first_destination[i] = true;
	}
	int y_step = 75;
	int offset = y_step / 3;
	for(int i = 0; i < num_tanks; i++)
	{
		destination[i] = Vec2(-grid_width / 2 + offset, grid_height / 2 - offset - i * 75);
	}
	int max_row_y_visited = destination[num_tanks - 1].y;
	vector<grid_t> occ_grid;
	float p_s;
	float p_o_s = true_positive;
	float p_o_ns = 1 - true_negative;
	float p_no_s = 1 - true_positive;
	float p_no_ns = true_negative;
	float probability;
	int my_grid_x;
	int my_grid_y;
	grid_t grid_item;
	int occ_grid_size;
	while(true)
	{
		tanks.clear();
		MyTeam.get_mytanks(&tanks);
		for(int l = 0; l < num_tanks; l++)
		{
			occ_grid.clear();
			MyTeam.get_occgrid(&occ_grid, l);
			grid_item = occ_grid[0];
			occ_grid_size = grid_item.xdim * grid_item.ydim;
			for(int i = 0; i < occ_grid_size; i++)
			{
				my_grid_y = grid_item.y + i / grid_item.xdim + grid_height / 2;
				my_grid_x = grid_item.x + i % grid_item.xdim + grid_width / 2;
				p_s = grid_probabilities[my_grid_y][my_grid_x];
				if(grid_item.grid[i])
				{
					grid_probabilities[my_grid_y][my_grid_x] = p_o_s * p_s / (p_o_s * p_s + p_o_ns * (1 - p_s));
				}
				else
				{
					//grid_probabilities[grid_item.y][grid_item.x] = p_no_s * p_s / (p_no_s * p_s + p_no_s * p_s);
					grid_probabilities[my_grid_y][my_grid_x] = p_no_s * p_s / (p_no_s * p_s + p_no_ns * (1 - p_s));
				}

				probability = grid_probabilities[my_grid_y][my_grid_x];

				if(probability >= .9)
				{
					grid[my_grid_y][my_grid_x] = 1;
				}
				else if(probability <= .1)
				{
					grid[my_grid_y][my_grid_x] = 0;
				}
				else
				{
					grid[my_grid_y][my_grid_x] = .5;
				}
			}
			MyTeam.shoot(l);
			tank_t agent = tanks[l];

			//TODO reached_destinationn to not always be false.
			bool reached_destination = get_distance(Vec2(agent.pos), destination[l]) < 10;
			if(reached_destination)
			{
				if(first_destination[l])
				{
					destination[l] = Vec2(grid_width / 2 - offset, destination[l].y);
				}
				else if(max_row_y_visited > -grid_height / 2 + y_step)
				{
					max_row_y_visited = max_row_y_visited - y_step;
					if(destination[l].x > 0)
					{
						destination[l] = Vec2(-grid_width / 2 + offset, max_row_y_visited);
					}
					else
					{
						destination[l] = Vec2(grid_width / 2 - offset, max_row_y_visited);
					}
				}
				else if(max_row_y_visited > -grid_height / 2 + offset)
				{
					max_row_y_visited = -grid_height / 2 + offset;
					if(destination[l].x > 0)
					{
						destination[l] = Vec2(-grid_width / 2 + offset, max_row_y_visited);
					}
					else
					{
						destination[l] = Vec2(grid_width / 2 - offset, max_row_y_visited);
					}
				}
				else
				{
					destination[l] = my_flag_pos;
				}
				first_destination[l] = false;
			}

			// Go Towards Flag (Attractive field - Seek Goal)
			double dist = get_distance(destination[l],Vec2(agent.pos));
			double attractiveAngle = normalize_angle(get_angle(Vec2(agent.pos), destination[l]) - agent.angle);
			double attractiveSpeed = dist;

			//TODO figure out how to avoid obstacles with the new system.
			// Avoid obstacles
			/*Vec2 near;
			Vec2 nearest;
			double nearestDist = numeric_limits<double>::max();;
			obstacle_t obstacle;
			for(int k = 0; k < obstacles.size(); k++)
			{
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
			}*/

			attractiveAngle = normalize_angle(attractiveAngle);
			double repulsiveAngle = 0;
			repulsiveAngle = normalize_angle(repulsiveAngle);
			MyTeam.angvel(l, normalize_angle(attractiveAngle + repulsiveAngle));
			MyTeam.speed(l, attractiveSpeed);

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
		}

		update_grid(grid);
		draw_grid();
	}
	printf("Captured the flag\n");
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
