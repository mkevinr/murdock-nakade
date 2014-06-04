/*
 * kalman_lab.cpp
 *
 *  Created on: May 31, 2014
 *      Author: rnakade
 */

#include "kalman_lab.h"
#include "plot.h"
#include "timer.h"
#include "vec2.h"

using namespace std;
using namespace arma;

const char *kDefaultServerName = "localhost";
const int kDefaultServerPort = 50100;

fmat u, sigma_x(6,6), F(6,6), F_transpose, sigma_z(2,2), H(2,6),
		H_transpose, K_t_plus_1, z(2,1), sigma_t(6,6), I(6,6), X, F_future(6,6), u_future;
double t = 0.05;

void update_F(float t);

int main(int argc, char *argv[])
{

	if(argc < 4)
	{
		cout << "Usage: <host> <port> [kalman | constant | wild]" << endl;
		exit(0);
	}

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

	if(strcmp(argv[3], "kalman") == 0)
	{
		kalman_agent(MyTeam);
	}
	else if(strcmp(argv[3], "constant") == 0)
	{
		conforming_agent(MyTeam);
	}
	else if(strcmp(argv[3], "wild") == 0)
	{
		non_conforming_agent(MyTeam);
	}
	else
	{
		cout << "Usage: <host> <port> [kalman | constant | wild]" << endl;
		exit(0);
	}

	MyTeam.Close();

	return 1;
}

void conforming_agent(BZRC MyTeam)
{
	cout << "Starting conforming agent..." << endl;
	while(true)
	{
		MyTeam.speed(0, 1);
		if(MyTeam.get_connection_closed())
		{
			exit(0);
		}
		sleep(2);
	}

}

void non_conforming_agent(BZRC MyTeam)
{
	cout << "Starting non-conforming agent..." << endl;
	int random_speed, random_angvel;
	bool success;
	while(!MyTeam.get_connection_closed())
	{
		random_speed = (double)(rand() % 250 ) / 10;
		random_angvel = (double)(rand() % 500 ) / 10 - 25;
		success = MyTeam.speed(0, random_speed);
		success = MyTeam.angvel(0, random_angvel);
		sleep(2);
	}
	cout << "Exiting..." << endl;
}

void init_matrices()
{
	u << 0 << endr
	  << 0 << endr
	  << 0 << endr
	  << -100 << endr
	  << 0 << endr
	  << 0 << endr;

	sigma_x.fill(0);
	sigma_x(0,0) = .1;
	sigma_x(1,1) = .1;
	sigma_x(2,2) = 20;
	sigma_x(3,3) = .1;
	sigma_x(4,4) = .1;
	sigma_x(5,5) = 20;

	F.fill(0);
	F(0,0) = 1;
	F(1,1) = 1;
	F(2,2) = 1;
	F(3,3) = 1;
	F(4,4) = 1;
	F(5,5) = 1;
	update_F(t);

	F_future.fill(0);
	F_future(0,0) = 1;
	F_future(1,1) = 1;
	F_future(2,2) = 1;
	F_future(3,3) = 1;
	F_future(4,4) = 1;
	F_future(5,5) = 1;

	sigma_z.fill(0);
	sigma_z(0,0) = 25;
	sigma_z(1,1) = 25;

	H.fill(0);
	H(0,0) = 1;
	H(1,3) = 1;

	H_transpose = trans(H);

	F_transpose = trans(F);

	z.fill(0);

	sigma_t.fill(0);
	sigma_t(0,0) = 25;
	sigma_t(1,1) = .1;
	sigma_t(2,2) = .1;
	sigma_t(3,3) = 25;
	sigma_t(4,4) = .1;
	sigma_t(5,5) = .1;

	I.fill(0);
	I(0,0) = 1;
	I(1,1) = 1;
	I(2,2) = 1;
	I(3,3) = 1;
	I(4,4) = 1;
	I(5,5) = 1;
}

void apply_kalman()
{
	// X is just here to save computation since this part of the formula occurs 3 times.
	X = F * sigma_t * F_transpose + sigma_x;
	// .i() takes the inverse. * does matrix multiplication
	K_t_plus_1 = X * H_transpose * (H * X * H_transpose + sigma_z).i();
	u = F * u + K_t_plus_1 * (z - H * F * u);
	sigma_t = (I - K_t_plus_1 * H) * X;
}

void update_F(float t)
{
	float t_squared = t * t;
	F(0,1) = t;
	F(0,2) = t_squared / 2;
	F(1,2) = t;
	F(3,4) = t;
	F(3,5) = t_squared / 2;
	F(4,5) = t;

	/*F =  1,     t, t*t/2,     0,     0,     0,
	       0,     1,     t,     0,     0,     0,
	       0,     0,     1,     0,     0,     0,
	       0,     0,     0,     1,     t, t*t/2,
	       0,     0,     0,     0,     1,     t,
	       0,     0,     0,     0,     0,     1;*/

	F_transpose = F.t();
}

void update_F_future(float t)
{
	float t_squared = t * t;
	F_future(0,1) = t;
	F_future(0,2) = t_squared / 2;
	F_future(1,2) = t;
	F_future(3,4) = t;
	F_future(3,5) = t_squared / 2;
	F_future(4,5) = t;
}

// Predict where the tank is going to be in just enough time in the future for the bullet to reach it.
Vec2 predict_future(BZRC & MyTeam)
{
	vector<tank_t> my_tanks;
	MyTeam.get_mytanks(&my_tanks);
	Vec2 my_pos(my_tanks[0].pos[0], my_tanks[0].pos[1]);
	Vec2 other_pos;
	float dist;
	float bullet_time;
	float diff;
	float min_t = -1;
	float min_diff = 10000000;
	// loop over a lot of time steps. Find one that is the right distance given the t value. By this I mean that
	 // the t value is just enough for the bullet to travel the distance between the tanks at that t value. This doesn't
	 // find the perfect t. It is just trying a lot of t values and choosing the best. The t values goes to the maximum
	 // distance that a bullet can travel, which is 350 (or 3.5 t).
	 //
	for(float future_t = t; !(future_t > 3.5); future_t += t)
	{
		// F_future is based off of a different t each time, so it has to change.
		update_F_future(future_t);
		u_future = F_future * u;
		other_pos = Vec2(u_future(0,0), u_future(3,0));
		dist = get_distance(my_pos, other_pos);
		// I am assuming that the bullet travels 100 world distance units in a second. There is a constant
		 // named speed that is set to 100. I am assuming this is what it means. By world distance units I mean
		 // 1 / the 800 height or width that the world is set to.
		 //
		bullet_time = dist / 100;
		diff = bullet_time - future_t;
		if(diff < 0)
		{
			diff = -diff;
		}

		if(diff < min_diff)
		{
			min_t = future_t;
			min_diff = diff;
		}
	}

	update_F_future(min_t);
	u_future = F_future * u;
	return Vec2(u_future(0,0), u_future(3,0));
}

void aim_at(BZRC& MyTeam, Vec2 pos, float distance)
{
	vector<tank_t> my_tanks;
	//double max_angular_velocity = 0.785398163397;
	MyTeam.get_mytanks(&my_tanks);
	tank_t my_tank = my_tanks[0];
	float attractive_angle = normalize_angle(get_angle(Vec2(my_tank.pos[0], my_tank.pos[1]), pos) - my_tank.angle);
	//if(attractive_angle > 0.05 || attractive_angle<-0.05)
	//{
		MyTeam.angvel(0, 400/distance * attractive_angle);
	//}
	//else
	//{
	//	MyTeam.angvel(0, 0);
	//}
}

void print_constants(BZRC& MyTeam)
{
	vector<constant_t> constants;

	cout << "Constants:" << endl;
	MyTeam.get_constants(&constants);
	for(int i = 0; i < constants.size(); i++)
	{
		cout << "name: " << constants[i].name << " value: " << constants[i].value << endl;
	}
}

void kalman_agent(BZRC MyTeam)
{
	cout << "Starting Kalman agent..." << endl;

	Timer timer;
	Timer timer2;
	double duration;
	vector<otank_t> other_tanks;
	bool new_tank = true;

	print_constants(MyTeam);

	Plotter * plotter;
	bool plot = true;
	if(plot)
	{
		// Uncomment this line and comment out the next to save the distribution as a gif.
		//plotter = new Plotter("gaussian.gif");
		plotter = new Plotter;
	}

	Vec2 target;
	float target_angle;
	for(int i = 0; true; i++)
	{
		timer.start();
		other_tanks.clear();
		MyTeam.get_othertanks(&other_tanks);
		if(strcmp(other_tanks[0].status.c_str(), "dead") == 0)
		{
			MyTeam.angvel(0, 0);
			new_tank = true;
			duration = timer.stop();
		    chrono::microseconds sleep_time((int)(1000000 * (t - duration)));
		    this_thread::sleep_for(sleep_time);
		    sleep(t - duration);
		    continue;
		}
		if(new_tank)
		{
			init_matrices();
		}
		z(0,0) = other_tanks[0].pos[0];
		z(1,0) = other_tanks[0].pos[1];

		apply_kalman();
		if(plot && i % 10 == 0)
		{
			plotter->plotMultivariateNormal(u(0,0), u(3,0), sigma_t(0,0), sigma_t(3,3), 0);
		}

		Vec2 current_pos = Vec2(u(0,0), u(3,0));
		vector<otank_t> other_tanks;
		// Get the point we want to aim at giving enough time for the bullet to travel there.
		target = predict_future(MyTeam);

		vector<tank_t> my_tanks;
		MyTeam.get_mytanks(&my_tanks);
		Vec2 my_pos(my_tanks[0].pos[0], my_tanks[0].pos[1]);

		target_angle = normalize_angle(get_angle(my_pos, target));
		float normalized_tank_angle = normalize_angle(my_tanks[0].angle);
		float angle_diff = normalized_tank_angle - target_angle;

		float dist = get_distance(my_pos, target);

		if(angle_diff < .1 && angle_diff > -.1 && dist < 400)
		{
			MyTeam.shoot(0);
			if(MyTeam.get_connection_closed())
			{
				if(plot)
				{
					plotter->end();
				}
				cout << "Exiting..." << endl;
				exit(0);
			}
		}

		timer2.start();
		aim_at(MyTeam, target, dist);
		if(MyTeam.get_connection_closed())
		{
			if(plot)
			{
				plotter->end();
			}
			cout << "Exiting..." << endl;
			exit(0);
		}

		duration = timer.stop();
		cout << "duration: " << duration << endl;
		if(t > duration)
		{
			// From http://en.cppreference.com/w/cpp/thread/sleep_for
		    chrono::microseconds sleep_time((int)(1000000 * (t - duration)));
		    this_thread::sleep_for(sleep_time);
		}
		else
		{
			cout << "\n\nError: loop running slower than t = " << t << "\n\n" << endl;
			cout << "duration: " << duration << endl;
		}
		new_tank = false;
	}
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
