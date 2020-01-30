/* 
	Name: 	drawer.h
	Author: Blakey Wu
	Date:	10/7/2019
	Usage:
		draw elements defined in elements.h.
		do clipping and rasterization, and setting each pixel onto the screen.
		for line drawing, two methods, DDA and B. are implemented.
	Purpose:
		It is designed to finish the homework, so clarity is the first proior, not efficiency.
*/

#ifndef DRAWER
#define DRAWER

#include "elements.h"
#include <vector>

#include <GL/gl.h>
#include <GL/glut.h>

typedef void (*callback)();
typedef void (*draw_func)(int,int);

class coordinate_transformer;

enum plane{
	XY,
	YZ,
	XZ
};

class config{

private:
	config();
	~config();
	static config* p_config;
	static std::vector<callback> cb_func_list;
	static void update();
	static bool use_DDA_;
	static draw_func set_pix;
	static coordinate_transformer* transformer[8];

public:
	static float x_min, x_max, y_min, y_max, z_min, z_max, center_x, center_y, center_z;
	static mat31 ambient, source_intensity, f, eye, intensity;
	static float distance;

public:
	static int width, height;
	static void set_bounding_box_size(float x_min, float x_max, float y_min, float y_max);
	static void set_window_size(int width, int height);
	static void use_DDA() { config::use_DDA_ = true; }
	static void use_Bresenham() { config::use_DDA_ = false; }
	static void register_update_cb(callback cb);
	static void register_draw_func(draw_func);
	static void remove_update_cb(void func());
	static float scaleX;
	static float scaleY;
	static bool use_halftone;
	static bool use_phong;
	static float source_intensity_val, ambient_intensity;

	friend class drawer;
	friend class line_drawer;
	friend class polygon_drawer;
	friend class controller;
	friend class line_drawer_3d;
};

class point_4_bit_extension{

public:
	point p;
	// defined as (0,0,0,0,u,d,l,r)
	char  extension;
	// default set as "1111", means do not exist
	point_4_bit_extension(): extension(-1){};
	point_4_bit_extension(const point& p);
	point_4_bit_extension(const point_4_bit_extension& p_): p(p_.p), extension(p_.extension){}
	~point_4_bit_extension() = default;
};

struct clip_result{
	bool p1_stay, p2_stay;
	point_4_bit_extension middle1, middle2;
	clip_result(bool p1_stay = false, bool p2_stay = false, const point_4_bit_extension &middle1 = point_4_bit_extension(), const point_4_bit_extension &middle2 = point_4_bit_extension()):
		p1_stay(p1_stay), p2_stay(p2_stay), middle1(middle1), middle2(middle2){
		};
};

/* the function of this class is to: 
	give a encode to transform any coordinate to {x > 0 && 0 < y < x}
	and a encode to transform it back;
*/

struct coordinate{
	int x;
	int y;
	coordinate(int x = 0, int y = 0): x(x), y(y){};
};

class coordinate_transformer{

public:
	 virtual coordinate encode(coordinate coord) = 0;
	 virtual coordinate decode(coordinate coord) = 0;
};

class coordinate_transformer10: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coord; }
	 inline coordinate decode(coordinate coord) { return coord; }
};

class coordinate_transformer11: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(coord.y, coord.x); }
	 inline coordinate decode(coordinate coord) { return coordinate(coord.y, coord.x); }
};

class coordinate_transformer20: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(coord.y, -coord.x); }
	 inline coordinate decode(coordinate coord) { return coordinate(-coord.y, coord.x); }
};

class coordinate_transformer21: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(-coord.x, coord.y); }
	 inline coordinate decode(coordinate coord) { return coordinate(-coord.x, coord.y); }
};

class coordinate_transformer30: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(-coord.x, -coord.y); }
	 inline coordinate decode(coordinate coord) { return coordinate(-coord.x, -coord.y); }
};

class coordinate_transformer31: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(-coord.y, -coord.x); }
	 inline coordinate decode(coordinate coord) { return coordinate(-coord.y, -coord.x); }
};

class coordinate_transformer40: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(-coord.y, coord.x); }
	 inline coordinate decode(coordinate coord) { return coordinate(coord.y, -coord.x); }
};

class coordinate_transformer41: public coordinate_transformer{

public:
	 inline coordinate encode(coordinate coord) { return coordinate(coord.x, -coord.y); }
	 inline coordinate decode(coordinate coord) { return coordinate(coord.x, -coord.y); }
};

class line_drawer: public line{

private:
	void DDA(const coordinate & p1, const coordinate & p2, coordinate_transformer* transformer);
	void Bresenham(const coordinate & p1, const coordinate & p2, coordinate_transformer* transformer);
	clip_result clipped_info;
	int transformer_index;
	draw_func set_pix;
public:
	float offset_x;
	float offset_y;

public:
	coordinate *buffer;
	void clip();
	line_drawer(const line & l, int offset_x = 0, int offset_y = 0);
	~line_drawer(){delete[] buffer;}
	void transport(const mat21 & v);
	void linear_transform(const mat22 & r, const point & origin);
	void draw();
	void set_draw_func(draw_func d_f){ this->set_pix = d_f; }
	friend class polygon_drawer;
};

class polygon_drawer: public polygon{
public:
	int width, height;
	int x_start, y_start;
private:
	mat31* color_buffer = NULL;
	bool fill_color_tri(const point & p1, const point & p2, const point & p3);
	void DDA(const point & p0_, const point & p1_);
public:
	std::vector<point> clipped_info;
	polygon_drawer(const polygon & p, int offset_x, int offset_y);
	~polygon_drawer(){ delete[] color_buffer;}
	void draw();
	float offset_x;
	float offset_y;
	friend class buffer_drawer;
	void create_color_buffer();
	void interpolate();
};

class line_drawer_3d: public line_3d{

private:
	draw_func set_pix;

public:
	line_drawer_3d(const line_3d & l): line_3d(l), set_pix(config::set_pix){};
	virtual ~line_drawer_3d() = default;
	void draw(plane p);
	void set_draw_func(draw_func d_f){ this->set_pix = d_f; }
};

class polyhedron_drawer{
public:
	std::vector<point_3d> points;
	float min_x, min_y, min_z;
	std::vector<int> starts;
	std::vector<int> middles;
	std::vector<int> ends;
	std::vector<float> specularity;
	std::vector<point_3d> normals;
	std::vector<point_3d> face_normals;
	std::vector<mat31> colors;
	void reset_color();
	polyhedron_drawer(const std::vector<point_3d> points, std::vector<int> starts, std::vector<int> middles, std::vector<int> ends, std::vector<float> specularity):
		points(points), starts(starts), middles(middles), ends(ends), specularity(specularity){
			std::vector<int> s;
			std::vector<float> specu;
			for (int i = 0; i < points.size(); i++){
				normals.push_back(point_3d(0.0, 0.0, 0.0));
				s.push_back(0);
				colors.push_back(mat31(points[i].r, points[i].g, points[i].b));
				specu.push_back(0.0);
			}
			for (int i = 0; i < specularity.size(); i++){
				int start = starts[i] - 1;
				int middle = middles[i] - 1;
				int end = ends[i] - 1;

				point_3d a = points[start];
				point_3d b = points[middle];
				point_3d c = points[end];

				mat31 normal = mat31(a - b).cross(mat31(b - c));
				normal /= normal.norm2();
				face_normals.push_back(point_3d(normal));

				normals[start] += normal;
				normals[middle] += normal;
				normals[end] += normal;

				specu[start] += specularity[i];
				specu[middle] += specularity[i];
				specu[end] += specularity[i];

				s[start] ++;
				s[middle] ++;
				s[end] ++;
			}
			for (int i = 0; i < normals.size(); i++){
				normals[i] = mat31(normals[i] / s[i]);
				specu[i] /= s[i];
			}
			specularity = specu;
			renew_min();
		};
	~polyhedron_drawer() = default;
	void transport(mat31 trans){
		for (int i = 0; i < points.size(); i++){
			points[i].transport(trans);
		}
		reset_color();
		min_x += trans.getX();
		min_y += trans.getY();
		min_z += trans.getZ();
	};
	void linear_transform(mat33 rotation, mat31 trans = mat31()){
		int size = points.size();
		for (int i = 0; i < size; i++){
			points[i].linear_transform(rotation, trans);
			normals[i].linear_transform(rotation, mat31());
		}
		size = face_normals.size();
		for (int i = 0; i < size; i++){
			face_normals[i].linear_transform(rotation, mat31());
		}
		reset_color();
		renew_min();
	};
	point_3d getCentroid() const{
		point_3d p(0,0,0);
		for (int i = 0; i < points.size(); i++){
			p += points[i];
		}
		p /= points.size();
		return p;
	}
	void renew_min(){
		float x = 9999, y = 9999, z = 9999;
		int size = points.size();
		for (int i = 0; i  < size; i++){
			if (points[i].getX() < x){
				x = points[i].getX();
			}
			if (points[i].getY() < y){
				y = points[i].getY();
			}
			if (points[i].getZ() < z){
				z = points[i].getZ();
			}
		}
		min_x = x;
		min_y = y;
		min_z = z;
	}
	void draw();
	void drawX();
	void drawY();
	void drawZ();
};

#endif