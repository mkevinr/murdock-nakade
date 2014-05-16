#ifndef PLOT_H
#define PLOT_H

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <math.h>

#include "pfagent.h"

using namespace std;

class Plotter
{
	FILE * p;
	string fileName;

public:
	Plotter(string fileName):fileName(fileName)
	{
		initPlot();
	}

private:
	void initPlot()
	{
		p = popen("gnuplot", "w");
		fprintf(p, "set term png\n");
		fprintf(p, "set output '%s'\n", fileName.c_str());
		fprintf(p, "set xrange [-400.0: 400.0]\n");
		fprintf(p, "set yrange [-400.0: 400.0]\n");
		fprintf(p, "unset key\n");
		fprintf(p, "set size square\n");
		fflush(p);
	}

public:

	void plotObstacles(vector<obstacle_t> obstacles)
	{
		fprintf(p, "unset arrow\n");
		obstacle_t obstacle;
		for(int i = 0; i < obstacles.size(); i++)
		{
			obstacle = obstacles[i];
			for(int j = 0; j < obstacle.courner_count; j++)
			{
				fprintf(p, "set arrow from %f, %f to %f, %f nohead lt 3\n", obstacle.o_corner[j][0]
				        , obstacle.o_corner[j][1], obstacle.o_corner[(j + 1) % obstacle.courner_count][0]
				        , obstacle.o_corner[(j + 1) % obstacle.courner_count][1]);
				fflush(p);
			}
		}

		fprintf(p, "plot '-' with vectors head\n");
	}

	void reset(string newFileName)
	{
		fileName = newFileName;
		initPlot();
	}

	void plotField(double x, double y, double angle, double speed)
	{
		fprintf(p, "%f, %f, %f, %f\n", x, y, cos(angle) * speed, sin(angle) * speed);
	}

	void end()
	{
		fprintf(p, "e\n");
		pclose(p);
	}
};

#endif
