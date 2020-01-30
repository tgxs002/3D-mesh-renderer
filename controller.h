#ifndef CONTROLLER
#define CONTROLLER

#include <nanogui/screen.h>
#include <nanogui/formhelper.h>
#include <nanogui/window.h>

#include "elements.h"
#include "drawer.h"
#include <iostream>
#include <pthread.h>
#include <vector>
#include <fstream>
#include <cmath>

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



enum Line_algorithm {
	Color = 0,
	Halftone,
	Halftone_smooth,
};

enum IO_stat{
	WRITE,
	READ,
	FREE
};

class controller{

private:
	static float scale;
	static float transX;
	static float transY;
	static float transZ;
	static float angle;
	static Line_algorithm algorithm;
	static int r, g, b;
	static std::string load_path;
	static float x1, y1, z1, x2, y2, z2;
public:
	static float x_max, x_min, y_max, y_min, z_max, z_min;
	static std::vector<polyhedron_drawer>* polygons;
	static void save_polygons();
	static void load_polygons();
	controller();
	static controller* p;
	~controller(){delete polygons;}
	static nanogui::FormHelper* gui;

public:
	static IO_stat stat;
	static int current;
	static void init(){controller::p = new controller();}
	static std::vector<polyhedron_drawer>* getPolygons(){return polygons;}
	static void move_current_polygon(float delta_x, float delta_y);
	static void renew_limit();
};

#endif