/*
 * Simple glut demo that can be used as a template for
 * other projects by sai kopparthi
 */

#include "elements.h"
#include "drawer.h"
#include <iostream>
#include <pthread.h>
#include<stdlib.h>
#include<time.h>
#include <vector>
#include <fstream>
#include "controller.h"
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>


/****set in main()****/
//the number of pixels in the grid
int grid_width;
int grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

pthread_t p_;

inline float max(const float x, const float y){
	return x < y? y : x;	
}

float buffer[305][305];

void init();
void idle();
void display();
void draw_pix(int x, int y);
void draw_mega(int x, int y);
void draw_mega_smooth(int x, int y);
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

void *wrap_loop2(void*){
	glutMainLoop();
}

static int last_x = 0;
static int last_y = 0;


int main(int argc, char **argv)
{
    //the number of pixels in the grid
    grid_width = 600;
    grid_height = 600;
    
    //the size of pixels sets the inital window height and width
    //don't make the pixels too large or the screen size will be larger than
    //your display size
    pixel_size = 1;
    
    /*Window information*/
    win_height = grid_height*pixel_size;
    win_width = grid_width*pixel_size;

    config::register_draw_func(draw_pix);
	config::set_window_size(grid_width, grid_height);
	//config::set_bounding_box_size(.0f, 3.0f, .0f, 3.0f);

	controller::init();
    
	/*Set up glut functions*/
    /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/
    
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    /*initialize variables, allocate memory, create buffers, etc. */
    //create window of size (win_width x win_height
    glutInitWindowSize(win_width,win_height);
    //windown title is "glut demo"
	glutCreateWindow("glut demo");
    
	/*defined glut callback functions*/
	glutDisplayFunc(display); //rendering calls here
	glutReshapeFunc(reshape); //update GL on window size change
	glutMouseFunc(mouse);     //mouse button events
	glutMotionFunc(motion);   //mouse movement events
	glutKeyboardFunc(key);    //Keyboard events
	glutIdleFunc(idle);       //Function called while program is sitting "idle"
    
    //initialize opengl variables
    init();
    //start glut event loop
    int ret = pthread_create(&p_, NULL, wrap_loop2, NULL);
    if (ret != 0){
    	std::cout << "Error creating thread." << std::endl;
    }
	pthread_exit(NULL);
	return 0;
}

/*initialize gl stufff*/
void init()
{
    //set clear color (Default background to white)
	glClearColor(0.0, 0.0, 0.0,1.0);
    //checks for OpenGL errors
	check();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
    //redraw the scene over and over again
	glutPostRedisplay();	
}

bool cmp(mat21 & x, mat21 & y){
	return x.getY() < y.getY();
}

bool cmpx(polyhedron_drawer x, polyhedron_drawer y){
	return x.min_x < y.min_x;
}

bool cmpy(polyhedron_drawer x, polyhedron_drawer y){
	return x.min_y < y.min_y;
}

bool cmpz(polyhedron_drawer x, polyhedron_drawer y){
	return x.min_z < y.min_z;
}

//this is where we render the screen
void display()
{
    //clears the screen
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    //clears the opengl Modelview transformation matrix
	glLoadIdentity();

	point down(.0f, config::y_min - config::y_max);
	point up(.0f, config::y_max - config::y_min);
	point left(config::x_min - config::x_max, .0f);
	point right(config::x_max - config::x_min, .0f);

	line_drawer d1((line(up, down)), config::width / 4, config::height / 4);
	line_drawer d2((line(left, right)), config::width / 4, config::height / 4);

	glColor3f(1.0,1.0,1.0);
	config::intensity = 1.0;
	d1.draw();
	d2.draw();

	if (controller::stat == WRITE){
		return;
	}
	else{
		controller::stat = READ;
	}
	std::sort(controller::getPolygons()->begin(), controller::getPolygons()->end(), cmpx);
	for (auto p = controller::getPolygons()->begin(); p < controller::getPolygons()->end(); p++){
		p->drawX();
	}
	std::sort(controller::getPolygons()->begin(), controller::getPolygons()->end(), cmpy);
	for (auto p = controller::getPolygons()->begin(); p < controller::getPolygons()->end(); p++){
		p->drawY();
	}
	std::sort(controller::getPolygons()->begin(), controller::getPolygons()->end(), cmpz);
	for (auto p = controller::getPolygons()->begin(); p < controller::getPolygons()->end(); p++){
		p->drawZ();
	}
	controller::stat = FREE;

    
    //blits the current opengl framebuffer on the screen
    glutSwapBuffers();
    //checks for opengl errors
	check();
}


//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y){
    glBegin(GL_POINTS);
    glVertex3f(x+.5,y+.5,0);
    glEnd();
}

void draw_mega(int x, int y){
	float max_int = max(config::intensity.getX(), max(config::intensity.getY(), config::intensity.getZ()));
	float all = config::intensity.getX() + config::intensity.getY() + config::intensity.getZ();
	int intensity = int(max_int * 8.99) + 1;
	std::vector<mat21> nums;
    for(int i = 0; i < 9; i++){
    	nums.push_back(mat21(i, rand()));
    }
    sort(nums.begin(), nums.end(), cmp);
    int i = 0;
    float num = 9 * max_int;
    int r = int(config::intensity.getX() * num / all);
    int g = int(config::intensity.getY() * num / all) + r;
    int b = intensity;
    glColor3f(1.0, 0, 0);
    for (; i < r; i++){
    	int index = nums[i].getX();
    	draw_pix(3 * x + index / 3 - 1, 3 * y + index % 3 - 1);
    }
    glColor3f(0, 1.0, 0);
    for (; i < g; i++){
    	int index = nums[i].getX();
    	draw_pix(3 * x + index / 3 - 1, 3 * y + index % 3 - 1);
    }
    glColor3f(0, 0, 1.0);
    for (; i < b; i++){
    	int index = nums[i].getX();
    	draw_pix(3 * x + index / 3 - 1, 3 * y + index % 3 - 1);
    }
}


void draw_mega_smooth(int x, int y){
	std::vector<mat21> nums;
	float max_int = max(config::intensity.getX(), max(config::intensity.getY(), config::intensity.getZ()));
    for (int i = 0; i < 9; i++){
    	if (rand() / double(RAND_MAX) < max_int)
	    	draw_pix(3 * x + i / 3, 3 * y + i % 3);
    }
}

/*Gets called when display size changes, including initial craetion of the display*/
void reshape(int width, int height)
{
	/*set up projection matrix to define the view port*/
    //update the ne window width and height
	win_width = width;
	win_height = height;
    
    //creates a rendering area across the window
	glViewport(0,0,width,height);
    // up an orthogonal projection matrix so that
    // the pixel space is mapped to the grid space
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0,grid_width,0,grid_height,-10,10);
    
    //clear the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //set pixel size based on width, if the aspect ratio
    //changes this hack won't work as well
    pixel_size = width/(float)grid_width;
    
    //set pixel size relative to the grid cell size
    glPointSize(pixel_size);
    //check for opengl errors
	check();
}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
	switch(ch)
	{
		default:
            //prints out which key the user hit
            printf("User hit the \"%c\" key\n",ch);
			break;
	}
    //redraw the scene after keyboard input
	glutPostRedisplay();
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
    //print the pixel location, and the grid location
    printf ("MOUSE AT PIXEL: %d %d, GRID: %d %d\n",x,y,(int)(x/pixel_size),(int)((win_height-y)/pixel_size));
	switch(button)
	{
		case GLUT_LEFT_BUTTON: //left button
            printf("LEFT ");
            break;
		case GLUT_RIGHT_BUTTON: //right button
            printf("RIGHT ");
		default:
            printf("UNKNOWN "); //any other mouse button
			break;
	}
    if(state !=GLUT_DOWN){
    	printf("BUTTON UP\n");
    	// if (x == last_x && y == last_y){
    	// 	int grid_x = (int)(x/pixel_size);
    	// 	int grid_y = (int)((win_height-y)/pixel_size);
    	// 	controller::click(grid_x, grid_y);
    	// }
    }
    else{
    	printf("BUTTON DOWN\n");  //button clicked
    }
	last_x = x;
	last_y = y;
    
    //redraw the scene after mouse click
    glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
	// //p = point((, );
	// if (controller::stat != FREE){
	// 	return;
	// }
	// // float motion_x = ((float)((x - last_x) / pixel_size) - 0.5) / grid_width * 2;
	// // float motion_y = ((float)((-(y - last_y)) / pixel_size) - 0.5) / (grid_height) * 2;
	// float motion_x = (float)(x - last_x) / pixel_size / grid_width * 2;
	// float motion_y = (float)(last_y - y) / pixel_size / grid_height * 2;
	// if (motion_x != 0 && motion_y != 0){
	// 	controller::move_current_polygon(motion_x, motion_y);
	// }
 //    //redraw the scene after mouse movement
	// last_x = x;
	// last_y = y;
	glutPostRedisplay();
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("GLERROR: There was an error %s\n",gluErrorString(err) );
		exit(1);
	}
}
