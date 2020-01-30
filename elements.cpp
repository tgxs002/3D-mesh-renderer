/* 
	Name: 	elements.cpp
	Author: Blakey Wu
	Date:	10/6/2019
*/

#include "elements.h"
#include <assert.h>
#include <iostream>
#include <cmath>
#include <vector>

mat::mat(int m, int n): m(m), n(n){
	this->size = m * n;
	this->buffer = NULL;
	assert(m > 0 && n > 0);
	this->buffer = new float[m * n];
	for (int i = 0; i < this->size; i++){
		this->buffer[i] = .0f;
	}
}

mat::~mat(){
	delete[] this->buffer;
}

mat::mat(const mat& other): m(other.m), n(other.n), size(other.size){
	this->buffer = new float[m * n];
	for (int i = 0; i < other.size; i++){
		this->buffer[i] = other.buffer[i];
	}
}

// TODO: if this is used for pixel-wise operation, it should be optimized.
mat mat::operator * (const mat& other)const{
	assert(this->n == other.m);
	mat res = mat(this->m, other.n);
	for (int i = 0; i < this->m; i++){
		for (int j = 0; j < other.n; j++){
			float counter = 0.0f;
			for (int k = 0; k < this->n; k++){
				counter += this->buffer[i * this->n + k] * other.buffer[k * other.n + j];
			}
			res.buffer[i * other.n + j] = counter;
		}
	}
	return res;
}

mat mat::operator * (float scale) const{
	mat res = mat(this->m, this->n);
	for (int i = 0; i < this->size; i++){
		res.buffer[i] = this->buffer[i] * scale;
	}
	return res;
}

mat mat::operator / (float scale) const{
	assert(scale != .0f);
	mat res = mat(this->m, this->n);
	for (int i = 0; i < this->size; i++){
		res.buffer[i] = this->buffer[i] / scale;
	}
	return res;
}

mat mat::operator + (const mat& other)const{
	assert(this->m == other.m && this->n == other.n);
	mat res = mat(this->m, this->n);
	for (int i = 0; i < this->size; i++){
		res.buffer[i] = this->buffer[i] + other.buffer[i];
	}
	return res;
}

void mat::operator += (const mat& other){
	assert(this->m == other.m && this->n == other.n);
	for (int i = 0; i < this->size; i++){
		this->buffer[i] += other.buffer[i];
	}
}

mat mat::operator - (const mat& other)const{
	assert(this->m == other.m && this->n == other.n);
	mat res = mat(this->m, this->n);
	for (int i = 0; i < this->size; i++){
		res.buffer[i] = this->buffer[i] - other.buffer[i];
	}
	return res;
}

void mat::operator -= (const mat& other){
	assert(this->m == other.m && this->n == other.n);
	for (int i = 0; i < this->size; i++){
		this->buffer[i] -= other.buffer[i];
	}
}

void mat::operator *= (float scale){
	for (int i = 0; i < this->size; i++){
		this->buffer[i] *= scale;
	}
}

void mat::operator /= (float scale){
	assert(scale != .0f);
	for (int i = 0; i < this->size; i++){
		this->buffer[i] /= scale;
	}
}

void mat::operator = (const mat& other){
	if (this->size != other.size){
		delete[] this->buffer;
		buffer = new float[other.size];
		this->size = other.size;
		this->m = other.m;
		this->n = other.n;
	}
	for (int i = 0; i < this->size; i++){
		this->buffer[i] = other.buffer[i];
	}
}

float mat::norm2()const{
	float counter = .0f;
	float number_i;
	for (int i = 0; i < this->size; i++){
		number_i = this->buffer[i];
		counter += number_i * number_i;
	}
	return std::sqrt(counter);
}

float mat::dot(const mat& other)const{
	assert(this->m == other.m && this->n == other.n);
	float counter = 0;
	for (int i = 0; i < this->size; i++){
		counter += this->buffer[i] * other.buffer[i];
	}
	return counter;
}

void mat::print()const{
	std::cout << "mat" << this->m << "*" << this->n << std::endl;
	for (int i = 0; i < this->m; i++){
		std::cout << "    ";
		for (int j = 0; j < this->n; j++){
			std::cout << this->buffer[i * this->n + j] << " ";
		}
		std::cout << std::endl;
	}
}

mat11::mat11(const mat& other): mat(other){
	assert(other.getM() == 1 && other.getN() == 1);
}

mat21::mat21(float x, float y): mat(2, 1){
	this->buffer[0] = x;
	this->buffer[1] = y;
}

mat21::mat21(const mat& other): mat(other){
	assert(other.getM() == 2 && other.getN() == 1);
}

mat31::mat31(float x, float y, float z): mat(3, 1){
	this->buffer[0] = x;
	this->buffer[1] = y;
	this->buffer[2] = z;
}

mat31::mat31(const mat& other): mat(other){
	assert(other.getM() == 3 && other.getN() == 1);
}

mat12::mat12(float x, float y): mat(1, 2){
	this->buffer[0] = x;
	this->buffer[1] = y;
}

mat12::mat12(const mat& other): mat(other){
	assert(other.getM() == 1 && other.getN() == 2);
}

mat22::mat22(float x11, float x12, float x21, float x22): mat(2, 2){
	this->buffer[0] = x11;
	this->buffer[1] = x12;
	this->buffer[2] = x21;
	this->buffer[3] = x22;
}

mat22::mat22(const mat& other): mat(other){
	assert(other.getM() == 2 && other.getN() == 2);
}

mat33::mat33(float x11, float x12, float x13, 
		float x21, float x22, float x23, 
		float x31, float x32, float x33): mat(3, 3){
	this->buffer[0] = x11;
	this->buffer[1] = x12;
	this->buffer[2] = x13;
	this->buffer[3] = x21;
	this->buffer[4] = x22;
	this->buffer[5] = x23;
	this->buffer[6] = x31;
	this->buffer[7] = x32;
	this->buffer[8] = x33;
}

mat33::mat33(const mat& other): mat(other){
	assert(other.getM() == 3 && other.getN() == 3);
}

eye22::eye22(): mat22(1.0f, .0f, .0f, 1.0f){}

eye33::eye33(): mat33(1.0f, .0f, .0f, .0f, 1.0f, .0f, .0f, .0f, 1.0f){}

point::point(float x, float y, float r, float g, float b): mat21(x, y), r(r), g(g), b(b){
}

point::point(const mat21 & other): mat21(other){}

point point::getCentroid() const{
	return *this;
}

void point::transport(const mat21 & v){
	*this += v;
}

void point::linear_transform(const mat22 & r, const point & origin){
	*this -= origin;
	float x = this->r, y = g, z = b;
	(*this) = mat21(r * (*this));
	this->r = x;
	g = y;
	b = z;
	*this += origin;
}

void point::print()const{
	std::cout << "(" << this->buffer[0] << ", " << this->buffer[1] << ")" << std::endl;
}

line::line(const point& p1, const point& p2): p1(p1), p2(p2){

}
	
line::line(const line& other): p1(other.p1), p2(other.p2){

}

/* need to be optimized if it is called frequently between points' update */
float line::getLength()const{
	return (this->p1 - this->p2).norm2();
}

/* need to be optimized if it is called frequently between points' update */
mat21 line::getDirection()const{
	mat21 delta = this->p2 - this->p1;
	return delta / delta.norm2();
}

/* need to be optimized if it is called frequently between points' update */
point line::getCentroid() const{
	return mat21((p1 + p2) / 2.0f);
}

void line::transport(const mat21 & v){
	p1 += v;
	p2 += v;
}

void line::linear_transform(const mat22 & r, const point & origin){
	p1.linear_transform(r, origin);
	p2.linear_transform(r, origin);
}

bool line::intersect_with(const line & other)const{
	mat21 t1 = other.p1 - this->p1;
	mat21 t2 = other.p2 - this->p1;
	mat21 t3 = this->p2 - this->p1;
	float c1 = t3.cross(t1) * t3.cross(t2); 
	t1 = this->p1 - other.p1;
	t2 = this->p2 - other.p1;
	t3 = other.p2 - other.p1;
	float c2 = t3.cross(t1) * t3.cross(t2); 
	if (c1 < 0 && c2 < 0){
		return true;
	}
	if (c1 > 0 || c2 > 0){
		return false;
	}
	if ((c1 * c2 == 0) && (c1 + c2 < 0)){
		return true;
	}
	float l1 = (this->getCentroid() - other.getCentroid()).norm2();
	float l2 = this->getLength() + other.getLength();
	if (2 * l1 > l2){
		return false;
	}
	else{
		return true;
	}
}

void line::print()const{
	std::cout << "Line: " << std::endl;
	std::cout << "P1: ";
	p1.print();
	std::cout << "P2: ";
	p2.print();
}

polygon::polygon(const std::vector<point>& p_set){
	assert(p_set.size() > 0);
	std::vector<line> lines;
	for (auto p = p_set.begin(); p < p_set.end() - 1; p++){
		lines.push_back(line(*p, *(p+1)));
	}
	lines.push_back(line(*p_set.rbegin(), *p_set.begin()));
	bool ok = true;
	for (auto l = lines.begin() + 1; l < lines.end() - 1; l++){
		for (auto s = lines.begin(); s < l - 1; s++){
			if (l->intersect_with(*s)){
				std::cout << "Error: " << std::endl;
				l->print();
				std::cout << " intersects with: ";
				s->print();
				ok = false;
				break;
			}
		}
		if (l != (lines.end() - 2) && lines.rbegin()->intersect_with(*l)){
			std::cout << "Error: " << std::endl;
			l->print();
			std::cout << " intersects with: ";
			lines.rbegin()->print();
			ok = false;
			break;
		}
		if (!ok){
			throw "Error: intersection";
		}
	}
	if (ok){
		this->points = p_set;
	}
}

polygon::polygon(const polygon & other): points(other.points){

}

/* need optimization if called frequently */
point polygon::getCentroid() const{
	mat21 counter(.0f, .0f);
	for (auto p : this->points){
		counter += p;
	}
	return point(mat21(counter / this->points.size()));
}

void polygon::transport(const mat21 & v){
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->transport(v);
	}
}

void polygon::linear_transform(const mat22 & r, const point & origin){
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->linear_transform(r, origin);
	}
}
void polygon::print()const{
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->print();
	}
}

point_3d::point_3d(float x, float y, float z, float r, float g, float b): mat31(x, y, z), r(r), g(g), b(b){
}

point_3d::point_3d(const mat31 & other): mat31(other){}

point_3d point_3d::getCentroid() const{
	return *this;
}

void point_3d::transport(const mat31 & v){
	*this += v;
}

void point_3d::linear_transform(const mat33 & r, const point_3d & origin){
	*this -= origin;
	float R = this->r, G = this->g, B = this->b;
	(*this) = mat31(r * (*this));
	this->r = R;
	this->g = G;
	this->b = B;
	*this += origin;
}

void point_3d::print()const{
	std::cout << "(" << this->buffer[0] << ", " << this->buffer[1] << ", " << this->buffer[2] << ")" << std::endl;
}

line_3d::line_3d(const point_3d& p1, const point_3d& p2): p1(p1), p2(p2){

}
	
line_3d::line_3d(const line_3d& other): p1(other.p1), p2(other.p2){

}

/* need to be optimized if it is called frequently between points' update */
float line_3d::getLength()const{
	return (this->p1 - this->p2).norm2();
}

/* need to be optimized if it is called frequently between points' update */
mat31 line_3d::getDirection()const{
	mat31 delta = this->p2 - this->p1;
	return delta / delta.norm2();
}

/* need to be optimized if it is called frequently between points' update */
point_3d line_3d::getCentroid() const{
	return mat31((p1 + p2) / 2.0f);
}

void line_3d::transport(const mat31 & v){
	p1 += v;
	p2 += v;
}

void line_3d::linear_transform(const mat33 & r, const point_3d & origin){
	p1.linear_transform(r, origin);
	p2.linear_transform(r, origin);
}

void line_3d::print()const{
	std::cout << "Line_3d: " << std::endl;
	std::cout << "P1: ";
	p1.print();
	std::cout << "P2: ";
	p2.print();
}

polyhedron::polyhedron(const std::vector<point_3d> points, std::vector<int> starts, std::vector<int> ends):
	points(points), starts(starts), ends(ends){}

polyhedron::polyhedron(const polyhedron & other): 
	points(other.points), starts(other.starts), ends(other.ends){}

point_3d polyhedron::getCentroid() const{
	mat31 counter(0, 0, 0);
	for (auto p : this->points){
		counter += mat31(p);
	}
	counter /= this->points.size();
	return counter;
}

void polyhedron::transport(const mat31 & v){
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->transport(v);
	}
}

void polyhedron::linear_transform(const mat33 & r, const point_3d & origin){
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->linear_transform(r, origin);
	}
}

void polyhedron::print()const{
	std::cout << "polyhedron: " << std::endl;
	for (auto p = this->points.begin(); p < this->points.end(); p++){
		p->print();
	}
	std::cout << "polyhedron end." << std::endl;
}
