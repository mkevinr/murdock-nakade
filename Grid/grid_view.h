/*
 * grid_view.h
 *
 *  Created on: May 19, 2014
 *      Author: mkevinr
 */

#ifndef GRID_VIEW_H_
#define GRID_VIEW_H_

#include <GL/glut.h>
#include <iostream>

using namespace std;

// These were not put in a class so that draw_grid can be passed to glutDisplayFunc easily
const int grid_width = 640;
const int grid_height = 480;
GLfloat grid[grid_height][grid_width];

void draw_grid()
{
	cout << "grid[2][2]: " << grid[2][2] << endl;
    glRasterPos2f(-1, -1);
    glDrawPixels(grid_width, grid_height, GL_LUMINANCE, GL_FLOAT, grid);
    glFlush();
    glutSwapBuffers();
}

void update_grid(GLfloat new_grid[grid_height][grid_width])
{
    for(int i = 0; i < grid_height; i++)
    {
    	for(int j = 0; j < grid_width; j++)
    	{
    		grid[i][j] = new_grid[i][j];
    	}
    }
}

void init_grid()
{
	for(int i = 0; i < grid_height; i++)
	{
		for(int j = 0; j < grid_width; j++)
		{
			grid[i][j] = 0;
		}
	}
}


void init_window(int * argc, char ** argv)
{
    init_grid();
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(grid_width, grid_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Grid filter");
    glutDisplayFunc(draw_grid);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glutMainLoop();
}

#endif /* GRID_VIEW_H_ */
