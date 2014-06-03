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
double t = 0.5;

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
	//conforming_agent(MyTeam);
	//non_conforming_agent(MyTeam);

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
		random_speed = (rand() % 250 ) / 10;
		random_angvel = (rand() % 250 ) / 10;
		MyTeam.speed(1, random_speed);
		MyTeam.angvel(1, random_angvel);
		sleep(2);
	}

}

void init_matrices()
{
	u << 0 << endr
	  << 0 << endr
	  << 0 << endr
	  << -170 << endr
	  << 0 << endr
	  << 0 << endr;
	//u.print("u:");

	sigma_x.fill(0);
	sigma_x(0,0) = .1;
	sigma_x(1,1) = .1;
	sigma_x(2,2) = 20;
	sigma_x(3,3) = .1;
	sigma_x(4,4) = .1;
	sigma_x(5,5) = 20;
	//sigma_x.print("sigma_x:");

	F.fill(0);
	F(0,0) = 1;
	F(1,1) = 1;
	F(2,2) = 1;
	F(3,3) = 1;
	F(4,4) = 1;
	F(5,5) = 1;
	//F.print("F:");

	F_future.fill(0);
	F_future(0,0) = 1;
	F_future(1,1) = 1;
	F_future(2,2) = 1;
	F_future(3,3) = 1;
	F_future(4,4) = 1;
	F_future(5,5) = 1;
	//F_future.print("F_future:");

	sigma_z.fill(0);
	sigma_z(0,0) = 25;
	sigma_z(1,1) = 25;
	//sigma_z.print("sigma_z:");

	H.fill(0);
	H(0,0) = 1;
	H(1,3) = 1;
	//H.print("H:");

	H_transpose = trans(H);
	//H_transpose.print("H_transpose");

	F_transpose = trans(F);
	//F_transpose.print("F_transpose");

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
	//K_t_plus_1.print("K_t_plus_1:");
	//fmat test_matrix = H*F*u;
	//H.print("H:");
	//F.print("F:");
	//u.print("u:");
	//test_matrix.print("Matrix Test:");
	u = F * u + K_t_plus_1 * (z - H * F * u);
	//u.print("u:");
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

	//F.print("F:");

	/*F =  1,     t, t*t/2,     0,     0,     0,
	       0,     1,     t,     0,     0,     0,
	       0,     0,     1,     0,     0,     0,
	       0,     0,     0,     1,     t, t*t/2,
	       0,     0,     0,     0,     1,     t,
	       0,     0,     0,     0,     0,     1;*/

	F_transpose = F.t();
	//F_transpose.print("F_transpose");
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
	for(float t = 0.5; t <= 3.5; t+=0.5)
	{
		// F_future is based off of a different t each time, so it has to change.
		update_F_future(t);
		u_future = F_future * u;
		other_pos = Vec2(u_future(0,0), u_future(3,0));
		dist = get_distance(my_pos, other_pos);
		// I am assuming that the bullet travels 100 world distance units in a second. There is a constant
		 // named speed that is set to 100. I am assuming this is what it means. By world distance units I mean
		 // 1 / the 800 height or width that the world is set to.
		 //
		bullet_time = dist / 100;
		diff = bullet_time - t;
		if(diff < 1)
		{
			diff = -diff;
		}
		if(diff < min_diff)
		{
			min_t = t;
			min_diff = diff;
		}
	}

	update_F_future(min_t);
	u_future = F_future * u;
	return Vec2(u_future(0,0), u_future(3,0));
}

void aim_at(BZRC& MyTeam, Vec2 pos)
{
	vector<tank_t> my_tanks;
	float attractive_angle = 1;
	while(attractive_angle > 0.05 || attractive_angle<-0.05)
	{
		my_tanks.clear();
		MyTeam.get_mytanks(&my_tanks);
		tank_t my_tank = my_tanks[0];
		attractive_angle = normalize_angle(get_angle(Vec2(my_tank.pos[0], my_tank.pos[1]), pos) - my_tank.angle);
		MyTeam.angvel(0, attractive_angle);
	}
	MyTeam.angvel(0, 0);
}

void kalman_agent(BZRC MyTeam)
{
	init_matrices();
	update_F(t);
	Timer timer;
	double duration;
	vector<otank_t> other_tanks;
	Plotter plotter("gaussian.png");

	Vec2 target;
	while(true)
	{
		other_tanks.clear();
		MyTeam.get_othertanks(&other_tanks);
		timer.start();
		z(0,0) = other_tanks[0].pos[0];
		z(1,0) = other_tanks[0].pos[1];

		apply_kalman();
		plotter.plotMultivariateNormal(u(0,0), u(3,0), sigma_t(0,0), sigma_t(3,3), 0);

		Vec2 current_pos = Vec2(u(0,0), u(3,0));
		cout << "current x: " << current_pos.x << " y: " << current_pos.y << endl;
		// Get the point we want to aim at giving enough time for the bullet to travel there.
		target = predict_future(MyTeam);
		cout << "target: x: " << target.x << " y: " << target.y << endl;
		// Aim at the given point. predict_future assumes that our tank can turn faster than the enemy tank can move
		// away from us.

		vector<tank_t> my_tanks;
		MyTeam.get_mytanks(&my_tanks);
		Vec2 my_pos(my_tanks[0].pos[0], my_tanks[0].pos[1]);

		float dist = get_distance(my_pos, target);
		aim_at(MyTeam, target);

		if(dist < 400)
			MyTeam.shoot(0 );
		else
		{
			cout << " Enemy tank too far .. bullet won't reach" << endl;
			break;
		}

		duration = timer.stop();
		cout << "duration: " << duration << endl;
		if(t > duration)
		{
			// From http://en.cppreference.com/w/cpp/thread/sleep_for
		    chrono::microseconds sleep_time((int)(t - duration));
		    this_thread::sleep_for(sleep_time);
		}
		else
		{
			cout << "Error: loop running slower than t = " << 5 << endl;
			cout << "duration: " << duration << endl;
		}
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
