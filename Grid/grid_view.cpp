#include <GL/glut.h>

const int grid_width = 640;
const int grid_height = 480;
GLfloat grid[grid_height][grid_width];

void draw_grid()
{
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

int main(int argc, char ** argv)
{
	init_window(&argc, argv);
}
