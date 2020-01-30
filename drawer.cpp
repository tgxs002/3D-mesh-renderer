/* 
	Name: 	drawer.cpp
	Author: Blakey Wu
	Date:	10/7/2019
*/

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <cstring>
#include "drawer.h"
#include "controller.h"

float inline interpolate(float x0, float y0, float x1, float y1, float x_x){
	assert(x0 != x1);
	return (y1 - y0) / (x1 - x0) * (x_x - x0) + y0;
}

bool inline has_1_bit(char x){
	return x & (x - 1) == 0;
}

bool inline get_bit(const char x, int t){
	return (x >> t) & char(1);
}

inline float min(float x, float y){
	return x > y? y : x;
}

inline float max(float x, float y){
	return x < y? y : x;	
}

inline int round(float x){
	return std::floor(x + 0.5);
}

config* config::p_config = NULL;
std::vector<callback> config::cb_func_list = std::vector<callback>();
float config::x_min = -1.5f;
float config::x_max = 1.5f;
float config::y_min = -1.5f;
float config::y_max = 1.5f;
int config::width = 100;
int config::height = 100;
bool config::use_DDA_ = false;
draw_func config::set_pix = NULL;
coordinate_transformer* config::transformer[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
float config::scaleX = 1;
float config::scaleY = 1;
float config::center_x = 0.0f;
float config::center_y = 0.0f;
mat31 config::intensity = mat31();
bool config::use_halftone = false;
mat31 config::ambient = mat31(0.0, 0.0, 0.0);
mat31 config::source_intensity = mat31(1.0, 1.0, 1.0);
mat31 config::f = mat31(5, 5, 5);
mat31 config::eye = mat31(5, 5, 5);
float config::distance = 20;
bool config::use_phong = false;
float config::source_intensity_val = 1.0;
float config::ambient_intensity = 0.01;

config::config(){
	config::transformer[0] = new coordinate_transformer10();
	config::transformer[1] = new coordinate_transformer11();
	config::transformer[2] = new coordinate_transformer21();
	config::transformer[3] = new coordinate_transformer20();
	config::transformer[4] = new coordinate_transformer41();
	config::transformer[5] = new coordinate_transformer40();
	config::transformer[6] = new coordinate_transformer30();
	config::transformer[7] = new coordinate_transformer31();
}

config::~config(){
	for (int i = 0; i < 8; i++){
		delete config::transformer[i];
	}
}


void config::update(){
	if (!config::p_config){
		config::p_config = new config();
	}
	for (auto func : config::cb_func_list){
		func();
	}
}

void config::set_bounding_box_size(float x_min, float x_max, float y_min, float y_max){
	if (!config::p_config){
		config::p_config = new config();
	}
	assert(x_max > x_min && y_max > y_min);
	config::x_min = x_min;
	config::x_max = x_max;
	config::y_min = y_min;
	config::y_max = y_max;
}

void config::set_window_size(int width, int height){
	if (!config::p_config){
		config::p_config = new config();
	}
	config::width = width;
	config::height = height;
}

void config::register_update_cb(callback cb){
	if (!config::p_config){
		config::p_config = new config();
	}
	config::cb_func_list.push_back(cb);
}

void config::remove_update_cb(callback func){
	if (!config::p_config){
		config::p_config = new config();
	}
	auto p = std::find(config::cb_func_list.begin(), config::cb_func_list.end(), func);
	if (p != config::cb_func_list.end()){
		config::cb_func_list.erase(p);
	}
}

void config::register_draw_func(draw_func func){
	if (!config::p_config){
		config::p_config = new config();
	}
	config::set_pix = func;
}

point_4_bit_extension::point_4_bit_extension(const point& p): p(p){
		extension = 0;
		extension += this->p.getY() > config::y_max;
		extension <<= 1;
		extension += this->p.getY() < config::y_min;
		extension <<= 1;
		extension += this->p.getX() < config::x_min;
		extension <<= 1;
		extension += this->p.getX() > config::x_max;
}

line_drawer::line_drawer(const line& l, int offset_x, int offset_y): line(l), clipped_info(), transformer_index(-1), set_pix(config::set_pix), offset_x(offset_x), offset_y(offset_y)
{ 
	buffer = new coordinate[2];
	this->clipped_info = clip_result(true, true, point_4_bit_extension(), point_4_bit_extension());
	// get coordinate
	int counter = 0;
	point buffer_[2];
	if (this->clipped_info.p1_stay){
		buffer_[counter++] = this->p1;
	}
	if (this->clipped_info.p2_stay){
		buffer_[counter++] = this->p2;
	}
	if (counter < 2){
		buffer_[counter++] = this->clipped_info.middle1.p;
	}
	if (counter < 2){
		buffer_[counter++] = this->clipped_info.middle2.p;
	}
	mat21 dir = buffer_[1] - buffer_[0];
	this->buffer[0] = coordinate(round(config::scaleX * (buffer_[0].getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (buffer_[0].getY() - config::center_y)) + config::height / 4 + offset_y);
	this->buffer[1] = coordinate(round(config::scaleX * (buffer_[1].getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (buffer_[1].getY() - config::center_y)) + config::height / 4 + offset_y);
	float d_x = this->buffer[1].x - this->buffer[0].x;
	float d_y = this->buffer[1].y - this->buffer[0].y;
	this->transformer_index = ((((d_y < 0) << 1) + (d_x < 0)) << 1) + (std::fabs(d_y) >= std::fabs(d_x));
};

void line_drawer::clip(){
	// both inside
	point_4_bit_extension p1 = point_4_bit_extension(this->p1);
	point_4_bit_extension p2 = point_4_bit_extension(this->p2);
	char index1 = p1.extension | p2.extension;
	char index2 = p1.extension & p2.extension;
	bool p1_stay = p1.extension == 0;
	bool p2_stay = p2.extension == 0;
	this->clipped_info = clip_result(true, true, point_4_bit_extension(), point_4_bit_extension());		

	// get coordinate
	int counter = 0;
	point buffer_[2];
	if (this->clipped_info.p1_stay){
		buffer_[counter++] = this->p1;
	}
	if (this->clipped_info.p2_stay){
		buffer_[counter++] = this->p2;
	}
	if (counter < 2){
		buffer_[counter++] = this->clipped_info.middle1.p;
	}
	if (counter < 2){
		buffer_[counter++] = this->clipped_info.middle2.p;
	}
	mat21 dir = buffer_[1] - buffer_[0];
	this->buffer[0] = coordinate(round(config::scaleX * (buffer_[0].getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (buffer_[0].getY() - config::center_y)) + config::height / 4 + offset_y);
	this->buffer[1] = coordinate(round(config::scaleX * (buffer_[1].getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (buffer_[1].getY() - config::center_y)) + config::height / 4 + offset_y);
	float d_x = this->buffer[1].x - this->buffer[0].x;
	float d_y = this->buffer[1].y - this->buffer[0].y;
	this->transformer_index = ((((d_y < 0) << 1) + (d_x < 0)) << 1) + (std::fabs(d_y) >= std::fabs(d_x));

}

void line_drawer::transport(const mat21 & v){
	this->line::transport(v);
	this->clip();
}

void line_drawer::linear_transform(const mat22 & r, const point & origin){
	this->line::linear_transform(r, origin);
	this->clip();
}

void line_drawer::draw(){
	if (this->transformer_index == -1){
		return;
	}
	coordinate coord0 = this->buffer[0];
	coordinate coord1 = this->buffer[1];
	// std::cout << coord0.x << " " << coord0.y << std::endl;
	// std::cout << coord1.x << " " << coord1.y << std::endl;
	// special case
	if (coord1.x == coord0.x){
		int y1 = coord0.y;
		int y2 = coord1.y;
		if (y1 > y2){
			std::swap(y1, y2);
		}
		/* parallel */
		for (int i = y1; i <= y2; i++){
			set_pix(coord0.x, i);
		}
		return;
	}
	else if (coord0.y == coord1.y){
		int x1 = coord0.x;
		int x2 = coord1.x;
		if (x1 > x2){
			std::swap(x1, x2);
		}
		/* parallel */
		for (int i = x1; i <= x2; i++){
			set_pix(i, coord0.y);
		}
		return;
	}
	coordinate_transformer* transformer = config::transformer[this->transformer_index];
	if (config::use_DDA_){
		this->DDA(coord0, coord1, transformer);
	}
	else{
		this->Bresenham(coord0, coord1, transformer);
	}
}

void line_drawer::DDA(const coordinate & p0, const coordinate & p1, coordinate_transformer* transformer){
	coordinate encode0 = transformer->encode(p0);
	coordinate encode1 = transformer->encode(p1);
	float k = float(encode1.y - encode0.y) / (encode1.x - encode0.x);
	/* parallel */
	for (int i = encode0.x, iter = 0; i <= encode1.x; i++, iter++){
		coordinate coord = transformer->decode(coordinate(i, round(encode0.y + k * iter)));
		this->set_pix(coord.x, coord.y);
	}
}

void line_drawer::Bresenham(const coordinate & p0, const coordinate & p1, coordinate_transformer* transformer){
	//std::cout << p0.x << " " << p0.y << " " << p1.x << " " << p1.y << std::endl;
	coordinate encode0 = transformer->encode(p0);
	coordinate encode1 = transformer->encode(p1);
	int delta_x = encode1.x - encode0.x;
	int delta_y = encode1.y - encode0.y;
	int p = 2 * delta_y - delta_x;
	set_pix(p0.x, p0.y);
	set_pix(p1.x, p1.y);
	/* parallel */
	for (int x = encode0.x + 1, y = encode0.y; x < encode1.x; x++){
		if (p >= 0){
			y += 1;
			p -= 2 * delta_x;
		}
		p += 2 * delta_y;
		coordinate coord = transformer->decode(coordinate(x, y));
		this->set_pix(coord.x, coord.y);
	}
}

polygon_drawer::polygon_drawer(const polygon & p, int offset_x, int offset_y): polygon(p), color_buffer(NULL), offset_x(offset_x), offset_y(offset_y){
	this->create_color_buffer();
	this->interpolate();
}

bool polygon_drawer::fill_color_tri(const point & p1, const point & p2, const point & p3){
	int x = round(config::scaleX * ((p1.getX() + p2.getX() + p3.getX()) / 3 - config::center_x)) + config::width / 4 + offset_x;
	int y = round(config::scaleY * ((p1.getY() + p2.getY() + p3.getY()) / 3 - config::center_y)) + config::height / 4 + offset_y;
	mat31 c = color_buffer[width * (y - y_start) + x - x_start];
	if (c.getX() < 0){
		color_buffer[width * (y - y_start) + x - x_start] = mat31((p1.r + p2.r + p3.r) / 3, (p1.g + p2.g + p3.g) / 3, (p1.b + p2.b + p3.b) / 3);
		point pc((p1.getX() + p2.getX() + p3.getX()) / 3, (p1.getY() + p2.getY() + p3.getY()) / 3, (p1.r + p2.r + p3.r) / 3, (p1.g + p2.g + p3.g) / 3, (p1.b + p2.b + p3.b) / 3);
		fill_color_tri(p1, p2, pc);
		fill_color_tri(p1, p3, pc);
		fill_color_tri(p2, p3, pc);
	}
}

void polygon_drawer::interpolate(){
	for (int i = 0; i < height; i++){
		int state = -1;
		mat31 a[2];
		int num[2];
		for (int j = 0; j < width && state < 2; j++){
			float critic = this->color_buffer[width * i + j].getX();
			if (state == -1){
				if (critic < -0.5){
				}
				else{
					state = 0;
					a[0] = this->color_buffer[width * i + j];
					num[0] = j;
				}
				continue;
			}
			switch (state){
				case 0:
				{
					if (critic > -0.5){
						a[0] = this->color_buffer[width * i + j];
						num[0] = j;
					}
					else{
						state = 1;
					}
					break;
				}
				case 1:
				{
					if (critic > -0.5){
						state = 2;
						a[1] = this->color_buffer[width * i + j];
						num[1] = j;
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
		if (state == 2){
			mat31 change = (a[1] - a[0]) / (num[1] - num[0]);
			mat31 work = a[0] + change;
			for (int k = num[0] + 1; k < num[1]; k++){
				this->color_buffer[width * i + k] = work;
				work += change;
			}
		}
	}
}

void polygon_drawer::DDA(const point & p0_, const point & p1_){
	mat31 start(p0_.r, p0_.g, p0_.b);
	mat31 change(p1_.r - p0_.r, p1_.g - p0_.g, p1_.b - p0_.b);
	coordinate p0(round(config::scaleX * (p0_.getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (p0_.getY() - config::center_y)) + config::height / 4 + offset_y);
	coordinate p1(round(config::scaleX * (p1_.getX() - config::center_x)) + config::width / 4 + offset_x, round(config::scaleY * (p1_.getY() - config::center_y)) + config::height / 4 + offset_y);
	float d_y = p1.y - p0.y;
	float d_x = p1.x - p0.x;
	int transformer_index = ((((d_y < 0) << 1) + (d_x < 0)) << 1) + (std::fabs(d_y) >= std::fabs(d_x));
	coordinate_transformer* transformer = config::transformer[transformer_index];

	coordinate encode0 = transformer->encode(p0);
	coordinate encode1 = transformer->encode(p1);
	float k = float(encode1.y - encode0.y) / (encode1.x - encode0.x);
	/* parallel */
	if (encode1.x != encode0.x){
		change /= (encode1.x - encode0.x);
	}
	else {
		coordinate coord = transformer->decode(encode0);
		this->color_buffer[width * (coord.y - y_start) + coord.x - x_start] = start + change / 2;
		return;
	}
	// if (encode1.x - encode0.x == encode1.y - encode0.y){
	// 	for (int i = 0; i <= encode1.x - encode0.x; i++){
	// 		coordinate coord = transformer->decode(coordinate(i + encode0.x, i + encode0.y));
	// 		this->color_buffer[width * (coord.y - y_start) + coord.x - x_start] = start;
	// 	}
	// 	return;
	// }
	for (int i = encode0.x, iter = 0; i <= encode1.x; i++, iter++){
		coordinate coord = transformer->decode(coordinate(i, round(encode0.y + k * iter)));
		this->color_buffer[width * (coord.y - y_start) + coord.x - x_start] = start;
		start += change;
	}
}

void polygon_drawer::create_color_buffer(){
	float up, down, left, right;
	up = down = points[0].getY();
	left = right = points[0].getX();
	for (auto p : points){
		if (p.getX() > right){
			right = p.getX();
		}
		else if (p.getX() < left){
			left = p.getX();
		}
		if (p.getY() > up){
			up = p.getY();
		}
		else if (p.getY() < down){
			down = p.getY();
		}
	}

	up = round(config::scaleY * (up - config::center_y)) + config::height / 4 + offset_y;
	down = round(config::scaleY * (down - config::center_y)) + config::height / 4 + offset_y;
	left = round(config::scaleX * (left - config::center_x)) + config::width / 4 + offset_x;
	right = round(config::scaleX * (right - config::center_x)) + config::width / 4 + offset_x;

	x_start = left;
	y_start = down;

	width = right - left + 1;
	height = up - down + 1;
	color_buffer = new mat31[width * height];
	for (int i = 0; i < width * height; i++){
		color_buffer[i] -= mat31(1.0, 0, 0);
	}
	DDA(points[0], points[1]);
	DDA(points[0], points[2]);
	DDA(points[2], points[1]);
}

void polygon_drawer::draw(){
	if (!this->color_buffer){
		return;
	}
	mat31 * ptr = this->color_buffer;
	for (int j = 0; j < this->height; j++){
		int y = j + this->y_start;
		for (int i = 0; i < this->width; i++){
			if (ptr->getX() > -0.5){
				if (config::use_halftone){
					config::intensity  = mat31(ptr->getX(), ptr->getY(), ptr->getZ());
				}
				else{
					glColor3f(ptr->getX(), ptr->getY(), ptr->getZ());
				}
				config::set_pix(i + this->x_start, y);
			}
			ptr++;
		}
	}
	glColor3f(1, 1, 1);
}

void line_drawer_3d::draw(plane pl){
	point p1_2d, p2_2d;
	if (pl == XY){
		p1_2d = point(this->p1.getX(), this->p1.getY());
		p2_2d = point(this->p2.getX(), this->p2.getY());
	}
	else if (pl == YZ){
		p1_2d = point(this->p1.getY(), this->p1.getZ());
		p2_2d = point(this->p2.getY(), this->p2.getZ());
	}
	else if (pl == XZ){
		p1_2d = point(this->p1.getX(), this->p1.getZ());
		p2_2d = point(this->p2.getX(), this->p2.getZ());
	}
	line l(p1_2d, p2_2d);
	line_drawer drawer_(l);
	if (pl == YZ){
		drawer_.offset_x = config::height / 2;
		drawer_.offset_y = config::width / 2;
	}
	else if (pl == XZ){
		drawer_.offset_x = 0;
		drawer_.offset_y = config::height / 2;
	}
	else{
		drawer_.offset_x = 0;
		drawer_.offset_y = 0;
	}
	drawer_.set_draw_func(this->set_pix);
	drawer_.clip();
	drawer_.draw();
}

bool cmp(mat21 x, mat21 y){
	return x.getX() < y.getX();
}

void polyhedron_drawer::drawX(){
	std::vector<line_3d> lines;
	for (int i = 0; i < this->starts.size(); i++){
		line_3d l(this->points[this->starts[i] - 1], this->points[this->ends[i] - 1]);
		lines.push_back(l);
	}
	int num_face = specularity.size();

	std::vector<mat21> depth_x;

	for (int i = 0; i < specularity.size(); i++){
		int start = starts[i] - 1;
		int middle = middles[i] - 1;
		int end = ends[i] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float min_x = min(a.getX(), min(b.getX(), c.getX()));		

		depth_x.push_back(mat21(min_x, i));
	}

	std::sort(depth_x.begin(), depth_x.end(), cmp);

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_x[i].getY()].getX() < 0){
			continue;
		}
		int start = starts[depth_x[i].getY()] - 1;
		int middle = middles[depth_x[i].getY()] - 1;
		int end = ends[depth_x[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_y = (controller::z_min + controller::z_max) / 2;
		config::center_x = (controller::y_min + controller::y_max) / 2;
		sx = config::width / (controller::y_max - controller::y_min) / 2;
		sy = config::height / (controller::z_max - controller::z_min) / 2;
		config::scaleX = sx < sy ? sx : sy;
		config::scaleY = config::scaleX;
		std::vector<point> pyz;
		pyz.push_back(point(a.getY(), a.getZ(), a.r, a.g, a.b));
		pyz.push_back(point(b.getY(), b.getZ(), b.r, b.g, b.b));
		pyz.push_back(point(c.getY(), c.getZ(), c.r, c.g, c.b));
		polygon_drawer pyzd(pyz, config::height / 2, config::width / 2);
		pyzd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}
}

void polyhedron_drawer::drawY(){
	std::vector<line_3d> lines;
	for (int i = 0; i < this->starts.size(); i++){
		line_3d l(this->points[this->starts[i] - 1], this->points[this->ends[i] - 1]);
		lines.push_back(l);
	}
	int num_face = specularity.size();

	std::vector<mat21> depth_y;

	for (int i = 0; i < specularity.size(); i++){
		int start = starts[i] - 1;
		int middle = middles[i] - 1;
		int end = ends[i] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float min_y = min(a.getY(), min(b.getY(), c.getY()));

		depth_y.push_back(mat21(min_y, i));
	}

	std::sort(depth_y.begin(), depth_y.end(), cmp);

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_y[i].getY()].getY() < 0){
			continue;
		}
		int start = starts[depth_y[i].getY()] - 1;
		int middle = middles[depth_y[i].getY()] - 1;
		int end = ends[depth_y[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_x = (controller::x_min + controller::x_max) / 2;
		config::center_y = (controller::z_min + controller::z_max) / 2;
		sx = config::width / (controller::x_max - controller::x_min) / 2;
		sy = config::height / (controller::z_max - controller::z_min) / 2;
		config::scaleY = sx < sy ? sx : sy;
		config::scaleX = config::scaleY;
		std::vector<point> pxz;
		pxz.push_back(point(a.getX(), a.getZ(), a.r, a.g, a.b));
		pxz.push_back(point(b.getX(), b.getZ(), b.r, b.g, b.b));
		pxz.push_back(point(c.getX(), c.getZ(), c.r, c.g, c.b));
		polygon_drawer pxzd(pxz, 0, config::height / 2);
		pxzd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}
}

void polyhedron_drawer::drawZ(){
	std::vector<line_3d> lines;
	for (int i = 0; i < this->starts.size(); i++){
		line_3d l(this->points[this->starts[i] - 1], this->points[this->ends[i] - 1]);
		lines.push_back(l);
	}
	int num_face = specularity.size();

	std::vector<mat21> depth_z;

	for (int i = 0; i < specularity.size(); i++){
		int start = starts[i] - 1;
		int middle = middles[i] - 1;
		int end = ends[i] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float min_z = min(a.getZ(), min(b.getZ(), c.getZ()));

		depth_z.push_back(mat21(min_z, i));
	}

	std::sort(depth_z.begin(), depth_z.end(), cmp);

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_z[i].getY()].getZ() < 0){
			continue;
		}
		int start = starts[depth_z[i].getY()] - 1;
		int middle = middles[depth_z[i].getY()] - 1;
		int end = ends[depth_z[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_x = (controller::x_min + controller::x_max) / 2;
		config::center_y = (controller::y_min + controller::y_max) / 2;
		sx = config::width / (controller::x_max - controller::x_min) / 2;
		sy = config::height / (controller::y_max - controller::y_min) / 2;
		config::scaleX = sx < sy ? sx : sy;
		config::scaleY = config::scaleX;
		std::vector<point> pxy;
		pxy.push_back(point(a.getX(), a.getY(), a.r, a.g, a.b));
		pxy.push_back(point(b.getX(), b.getY(), b.r, b.g, b.b));
		pxy.push_back(point(c.getX(), c.getY(), c.r, c.g, c.b));
		polygon_drawer pxyd(pxy, 0, 0);
		pxyd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}
}

void polyhedron_drawer::draw(){
	std::vector<line_3d> lines;
	for (int i = 0; i < this->starts.size(); i++){
		line_3d l(this->points[this->starts[i] - 1], this->points[this->ends[i] - 1]);
		lines.push_back(l);
	}
	int num_face = specularity.size();

	std::vector<mat21> depth_x;
	std::vector<mat21> depth_y;
	std::vector<mat21> depth_z;

	for (int i = 0; i < specularity.size(); i++){
		int start = starts[i] - 1;
		int middle = middles[i] - 1;
		int end = ends[i] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float min_z = min(a.getZ(), min(b.getZ(), c.getZ()));
		float min_x = min(a.getX(), min(b.getX(), c.getX()));
		float min_y = min(a.getY(), min(b.getY(), c.getY()));

		

		depth_x.push_back(mat21(min_x, i));
		depth_y.push_back(mat21(min_y, i));
		depth_z.push_back(mat21(min_z, i));
	}

	std::sort(depth_x.begin(), depth_x.end(), cmp);
	std::sort(depth_y.begin(), depth_y.end(), cmp);
	std::sort(depth_z.begin(), depth_z.end(), cmp);

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_z[i].getY()].getZ() < 0){
			continue;
		}
		int start = starts[depth_z[i].getY()] - 1;
		int middle = middles[depth_z[i].getY()] - 1;
		int end = ends[depth_z[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_x = (controller::x_min + controller::x_max) / 2;
		config::center_y = (controller::y_min + controller::y_max) / 2;
		sx = config::width / (controller::x_max - controller::x_min) / 2;
		sy = config::height / (controller::y_max - controller::y_min) / 2;
		config::scaleX = sx < sy ? sx : sy;
		config::scaleY = config::scaleX;
		std::vector<point> pxy;
		pxy.push_back(point(a.getX(), a.getY(), a.r, a.g, a.b));
		pxy.push_back(point(b.getX(), b.getY(), b.r, b.g, b.b));
		pxy.push_back(point(c.getX(), c.getY(), c.r, c.g, c.b));
		polygon_drawer pxyd(pxy, 0, 0);
		pxyd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_x[i].getY()].getX() < 0){
			continue;
		}
		int start = starts[depth_x[i].getY()] - 1;
		int middle = middles[depth_x[i].getY()] - 1;
		int end = ends[depth_x[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_y = (controller::z_min + controller::z_max) / 2;
		config::center_x = (controller::y_min + controller::y_max) / 2;
		sx = config::width / (controller::y_max - controller::y_min) / 2;
		sy = config::height / (controller::z_max - controller::z_min) / 2;
		config::scaleX = sx < sy ? sx : sy;
		config::scaleY = config::scaleX;
		std::vector<point> pyz;
		pyz.push_back(point(a.getY(), a.getZ(), a.r, a.g, a.b));
		pyz.push_back(point(b.getY(), b.getZ(), b.r, b.g, b.b));
		pyz.push_back(point(c.getY(), c.getZ(), c.r, c.g, c.b));
		polygon_drawer pyzd(pyz, config::height / 2, config::width / 2);
		pyzd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}

	for (int i = 0; i < num_face; i++){
		if (face_normals[depth_y[i].getY()].getY() < 0){
			continue;
		}
		int start = starts[depth_y[i].getY()] - 1;
		int middle = middles[depth_y[i].getY()] - 1;
		int end = ends[depth_y[i].getY()] - 1;

		point_3d a = points[start];
		point_3d b = points[middle];
		point_3d c = points[end];

		float cx = config::center_x;
		float cy = config::center_y;
		float psx = config::scaleX;
		float psy = config::scaleY;
		float sx = 0;
		float sy = 0;

		config::center_x = (controller::x_min + controller::x_max) / 2;
		config::center_y = (controller::z_min + controller::z_max) / 2;
		sx = config::width / (controller::x_max - controller::x_min) / 2;
		sy = config::height / (controller::z_max - controller::z_min) / 2;
		config::scaleY = sx < sy ? sx : sy;
		config::scaleX = config::scaleY;
		std::vector<point> pxz;
		pxz.push_back(point(a.getX(), a.getZ(), a.r, a.g, a.b));
		pxz.push_back(point(b.getX(), b.getZ(), b.r, b.g, b.b));
		pxz.push_back(point(c.getX(), c.getZ(), c.r, c.g, c.b));
		polygon_drawer pxzd(pxz, 0, config::height / 2);
		pxzd.draw();

		config::center_x = cx;
		config::center_y = cy;
		config::scaleX = psx;
		config::scaleY = psy;
	}
}

void polyhedron_drawer::reset_color(){
	if (!config::use_phong){
		for (int i = 0; i < points.size(); i++){
			points[i].r = colors[i].getX();
			points[i].g = colors[i].getY();
			points[i].b = colors[i].getZ();
		}
		return;
	}
	std::vector<mat31> temp;
	float m = -200;
	float t_;
	for (int i = 0; i < points.size(); i++){
		mat31 ls = config::source_intensity;
		mat31 eye_dis = config::eye - points[i];
		float coe0 = (eye_dis.norm2() + config::distance);
		mat31 lsp = config::f - points[i];
		t_ = lsp.norm2();
		if (t_ == 0){
			std::cout << "Divide by zero(0), try other parameters" << std::endl;
			std::cout << "Light Source:" << std::endl;
			config::f.print();
			return;
		}
		lsp /= t_;
		mat31 n = normals[i];
		t_ = n.norm2();
		if (t_ == 0){
			std::cout << "Divide by zero(1), try other parameters" << std::endl;
			std::cout << "Normal for " << i << "'th vertex:" << std::endl;
			n.print();
			return;
		}
		n /= t_;
		float coe1 = lsp.dot(n);
		coe1 = coe1 > 0 ? coe1 : 0;
		float cos = n.dot(lsp);
		mat31 r = n * (2 * cos) - lsp;
		t_ = eye_dis.norm2();
		if (t_ == 0){
			std::cout << "Divide by zero(2), try other parameters" << std::endl;
			std::cout << "Eye:" << std::endl;
			config::eye.print();
			return;
		}
		float coe2 = (eye_dis / t_).dot(r);
		coe2 = coe2 > 0 ? std::pow(coe2, specularity[i]) : 0;
		float inte = ls.norm2();

		mat31 color = config::ambient * config::ambient_intensity + colors[i] * (config::source_intensity_val / coe0 * coe1) + ls * (config::source_intensity_val * coe2 / coe0);

		temp.push_back(color);
		m = max(m, max(color.getX(), max(color.getY(), color.getZ())));
	}
	if (m == 0){
		std::cout << "Divide by 0(3), try other parameters" << std::endl;
		return;
	}
	for (int i = 0; i < points.size(); i++){
		temp[i] /= m;
		points[i].r = temp[i].getX();
		points[i].g = temp[i].getY();
		points[i].b = temp[i].getZ();
	}
}