/*
 * plot.h
 *
 *  Created on: May 31, 2014
 *      Author: rnakade
 */

#ifndef PLOT_H_
#define PLOT_H_

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <math.h>

#include "kalman_lab.h"

const double pi = 3.141592653589779;

class Plotter
{
	FILE * p;
	string file_name;
	bool plot_shown = false;

public:
	Plotter(string file_name):file_name(file_name)
	{
		initPlot();
	}

	Plotter()
	{
		initPlot();
	}

private:

	void initPlot()
	{
		p = popen("gnuplot", "w");
		//Display using ImageMagick
		//fprintf(p, "set output '| display gif:-'");
		fprintf(p, "clear\n");
		fprintf(p, "reset\n");
		if(file_name.length() > 0)
		{
			// This should set the animation delay to .5s. I am not entirely sure about this though.
			fprintf(p, "set terminal gif animate delay 5\n");
			fprintf(p, "set output \"%s\"\n", file_name.c_str());
		}
		fprintf(p, "set xrange [-400.0: 400.0]\n");
		fprintf(p, "set yrange [-400.0: 400.0]\n");
		fprintf(p, "set pm3d\n");
		fprintf(p, "set view map\n");
		fprintf(p, "unset key\n");
		fprintf(p, "set size square\n");
		fprintf(p, "set palette model RGB functions 1-gray, 1-gray, 1-gray\n");
		fprintf(p, "set isosamples 100\n");
		fflush(p);
	}

public:

	/*void reset(string newFileName)
	{
		fileName = newFileName;
		initPlot();
	}*/

	void end()
	{
		fprintf(p, "e\n");
		fprintf(p, "exit\n");
		pclose(p);
	}

	void plotMultivariateNormal(double x, double y, int sigma_x, int sigma_y, double rho)
	{
		fprintf(p, "sigma_x = %i\n", sigma_x);
		fprintf(p, "sigma_y = %i\n", sigma_y);
		fprintf(p, "x_naught = %g\n", x);
		fprintf(p, "y_naught = %g\n", y);
		fprintf(p, "rho = %g\n", rho);

		if(!plot_shown)
		{
			fprintf(p, "splot 1.0/(2.0 * pi * sigma_x * sigma_y * sqrt(1 - rho**2) ) \\\n * exp(-1.0/2.0 * ((x - x_naught)**2 / sigma_x**2 + (y - y_naught)**2 / sigma_y**2 \\\n - 2.0*rho*(x - x_naught)*(y - y_naught)/(sigma_x*sigma_y) ) ) with pm3d\n");
			if(file_name.length() == 0)
			{
				fprintf(p, "show plot\n");
			}
			plot_shown = true;
		}
		else
		{
			fprintf(p, "replot\n");
		}
		fflush(p);
	}

};

#endif /* PLOT_H_ */
