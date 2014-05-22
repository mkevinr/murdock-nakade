/*
 * grid_view.h
 *
 *  Created on: May 19, 2014
 *      Author: mkevinr
 */

#ifndef GRID_VIEW_H_
#define GRID_VIEW_H_

#include <iostream>
#include <GL/glut.h>
#include <GL/gl.h>

using namespace std;

// These were not put in a class so that draw_grid can be passed to glutDisplayFunc easily
const int grid_width = 800;
const int grid_height = 800;
GLfloat opengl_grid[grid_height][grid_width];

void draw_grid()
{
    glRasterPos2f(-1, -1);
    glDrawPixels(grid_width, grid_height, GL_LUMINANCE, GL_FLOAT, opengl_grid);
    glFlush();
    glutSwapBuffers();
}

void update_grid(GLfloat new_grid[grid_height][grid_width])
{
    for(int i = 0; i < grid_height; i++)
    {
    	for(int j = 0; j < grid_width; j++)
    	{
    		opengl_grid[i][j] = new_grid[i][j];
    	}
    }
}

void init_opengl_grid()
{
	for(int i = 0; i < grid_height; i++)
	{
		for(int j = 0; j < grid_width; j++)
		{
			opengl_grid[i][j] = 0;
		}
	}
}


void init_window(int * argc, char ** argv)
{
    init_opengl_grid();
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
    glutInitWindowSize(grid_width, grid_height);
    glutInitWindowPosition(grid_width + 30, 0);
    glutCreateWindow("Grid filter");
    glutDisplayFunc(draw_grid);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glutMainLoop();
}

#endif /* GRID_VIEW_H_ */
