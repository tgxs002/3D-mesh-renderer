#include "controller.h"
#include "drawer.h"


float controller::scale;
float controller::transX;
float controller::transY;
float controller::transZ;
float controller::angle;
float controller::x1 = 0, controller::y1 = 0, controller::z1 = 0, controller::x2 = 0, controller::y2 = 0, controller::z2 = 0;
float controller::x_max = -999, controller::x_min = 999, controller::y_max = -9990, controller::y_min = 999, controller::z_max = -9990, controller::z_min = 999;
IO_stat controller::stat = FREE;
Line_algorithm controller::algorithm = Color;
int controller::r, controller::g, controller::b;
std::string controller::load_path = std::string("bunny.txt");
std::vector<polyhedron_drawer>* controller::polygons = new std::vector<polyhedron_drawer>;
controller* controller::p = NULL;
int controller::current = -1;
nanogui::FormHelper* controller::gui = NULL;

pthread_t p__;

nanogui::Color color(1.0f, 1.0f, 1.0f, 1.0f);
nanogui::Color ambient(1.0f, 1.0f, 1.0f, 0.0f);
float ambient_intensity = 0.01;
float source_intensity = 1.0;
float eye_x = 5.0;
float eye_y = 5.0;
float eye_z = 5.0;
float f_x = 5.0;
float f_y = 5.0;
float f_z = 5.0;
float distance = 20;
bool use_phong;

void *wrap_loop(void*){
	nanogui::mainloop();
}

inline float min(float x, float y){
	return x > y? y : x;
}

void draw_pix(int x, int y);
void draw_mega(int x, int y);
void draw_mega_smooth(int x, int y);

controller::controller(){
    nanogui::Screen *screen = nullptr;

    nanogui::init();

    int w = 430, h = 700;
    screen = new nanogui::Screen(nanogui::Vector2i(w, h), "Controller", false);

    bool enabled = true;
    nanogui::FormHelper *gui = new nanogui::FormHelper(screen);
    nanogui::ref<nanogui::Window> window = gui->addWindow(Eigen::Vector2i(250, 0), "");

    nanogui::FormHelper *gui2 = new nanogui::FormHelper(screen);
    nanogui::ref<nanogui::Window> window2 = gui2->addWindow(Eigen::Vector2i(0, 0), "");
    //gui2->addGroup("test Settings");    

    gui2->addGroup("Algorithm Settings");
    gui2->addVariable("Drawing Algorithm", controller::algorithm, enabled)
       ->setItems({"Color", "Halftone", "Halftone smooth"});
    gui2->addVariable("Phong Model", use_phong);
    gui2->addVariable("polyhedron index:", controller::current, true);
    gui2->addButton("Set!", []() { 
    	while (controller::stat == READ);
    	controller::stat = WRITE;
    	controller::gui->refresh();
    	if (controller::algorithm == Color){
    		config::use_halftone = false;
            config::register_draw_func(draw_pix);
            config::set_window_size(600, 600);
    	}
    	else if (controller::algorithm == Halftone){
    		config::use_halftone = true;
            config::register_draw_func(draw_mega);
            config::set_window_size(200, 200);
    	}
        else{
            config::use_halftone = true;
            config::register_draw_func(draw_mega_smooth);
            config::set_window_size(200, 200);
        }
    	if (controller::current >= controller::polygons->size()){
    		controller::current = controller::polygons->size() - 1;
    	}
    	controller::gui->refresh();
        config::use_phong = use_phong;
        if (controller::current == -1){
            ;
        }
        else{
            while (controller::stat == READ);
            controller::stat = WRITE;
            for (int i = 0; i < controller::polygons->size(); i++){
                (*controller::polygons)[i].reset_color();
            }
            controller::stat = FREE;
        }
    	controller::stat = FREE;
     });
    gui2->addGroup("Colors");
    gui2->addVariable("Source:", color);
    gui2->addVariable("Source intensity:", source_intensity);
    gui2->addVariable("Ambient:", ambient);
    gui2->addVariable("Ambient intensity:", ambient_intensity);
    gui2->addButton("Go!", [](){
        config::source_intensity_val = source_intensity;
        config::ambient_intensity = ambient_intensity;
        config::ambient = mat31(ambient.r(), ambient.g(), ambient.b());
        config::source_intensity = mat31(color.r(), color.g(), color.b());
        if (controller::current == -1){
            ;
        }
        else{
            while (controller::stat == READ);
            controller::stat = WRITE;
            (*controller::polygons)[controller::current].reset_color();
            controller::stat = FREE;
        }
    });
    gui2->addGroup("Eye");
    gui2->addVariable("x", eye_x);
    gui2->addVariable("y", eye_y);
    gui2->addVariable("z", eye_z);
    gui2->addButton("Go!", [](){
        config::eye = mat31(eye_x, eye_y, eye_z);
        if (controller::current == -1){
            ;
        }
        else{
            while (controller::stat == READ);
            controller::stat = WRITE;
            (*controller::polygons)[controller::current].reset_color();
            controller::stat = FREE;
        }
    });
    gui2->addGroup("Constant");
    gui2->addVariable("distance", distance);
    gui2->addButton("Go!", [](){
        config::distance = distance;
        if (controller::current == -1){
            ;
        }
        else{
            while (controller::stat == READ);
            controller::stat = WRITE;
            (*controller::polygons)[controller::current].reset_color();
            controller::stat = FREE;
        }
    });
    gui2->addGroup("Light Source");
    gui2->addVariable("x", f_x);
    gui2->addVariable("y", f_y);
    gui2->addVariable("z", f_z);
    gui2->addButton("Go!", [](){
        config::f = mat31(f_x, f_y, f_z);
        if (controller::current == -1){
            ;
        }
        else{
            while (controller::stat == READ);
            controller::stat = WRITE;
            (*controller::polygons)[controller::current].reset_color();
            controller::stat = FREE;
        }
    });

    gui->addGroup("IO");
    gui->addVariable("Load path:", controller::load_path);
    gui->addButton("Load!", controller::load_polygons);

    gui->addGroup("Translation");
    gui->addVariable("x", controller::transX);
    gui->addVariable("y", controller::transY);
    gui->addVariable("Z", controller::transZ);
    gui->addButton("Go!", []() { 
    	while (controller::stat == READ);
    	controller::stat = WRITE;
    	controller::gui->refresh();
    	if (controller::current == -1){
    		;
    	}
    	else{
    		(*controller::polygons)[controller::current].transport(mat31(controller::transX, controller::transY, controller::transZ));
    	}
        controller::renew_limit();
    	controller::stat = FREE;
     });

    gui->addGroup("Rotation");
    gui->addVariable("x1", controller::x1);
    gui->addVariable("y1", controller::y1);
    gui->addVariable("z1", controller::z1);
    gui->addVariable("x2", controller::x2);
    gui->addVariable("y2", controller::y2);
    gui->addVariable("z2", controller::z2);
    gui->addVariable("angle", controller::angle);
    gui->addButton("Go!", []() { 
    	if (controller::current == -1){
    		return;
    	}
    	float rad = controller::angle * 3.1415926 / 180;
    	while (controller::stat == READ);
    	controller::stat = WRITE;
    	controller::gui->refresh();
    	mat31 trans1(-controller::x1, -controller::y1, -controller::z1);
    	mat31 trans1_tr(controller::x1, controller::y1, controller::z1);
    	float dx = controller::x2 - controller::x1;
    	float dy = controller::y2 - controller::y1;
    	float dz = controller::z2 - controller::z1;
    	float l = std::sqrt(dx * dx + dy * dy + dz * dz);
    	if (l == 0){
    		std::cout << "Direction can not be 0." << std::endl;
    		return;
    	}
    	float xy = std::sqrt(dx * dx + dy * dy);
    	mat33 r;
    	mat33 r_tr;
    	if (xy == 0){
    		r = eye33();
    		r_tr = eye33();
    		if (dz < 0){
    			rad = -rad;
    		}
    	}
    	else{
    		float r11 = dx * dz / xy / l, r12 =  dz * dy / l / xy, r13 = -xy / l;
	    	float r21 = -dy     /     xy, r22 =  dx      /     xy, r23 =       0;
	    	float r31 =  dx     /      l, r32 =  dy      /      l, r33 = dz  / l;
	    	r    = mat33(r11, r12, r13, r21, r22, r23, r31, r32, r33);
	    	r_tr = mat33(r11, r21, r31, r12, r22, r32, r13, r23, r33);
    	}

    	(*controller::polygons)[controller::current].transport(trans1);
    	(*controller::polygons)[controller::current].linear_transform(r);
    	(*controller::polygons)[controller::current].linear_transform(mat33(cos(rad), -sin(rad), 0, sin(rad), cos(rad), 0, 0, 0, 1));
    	(*controller::polygons)[controller::current].linear_transform(r_tr);
    	(*controller::polygons)[controller::current].transport(trans1_tr);
        controller::renew_limit();
    	controller::stat = FREE;
     });

	gui->addGroup("Scaling");
    gui->addVariable("scale", controller::scale);
    gui->addButton("Go!", []() { 
    	if (scale == 0){
    		std::cout << "Please don't scale by 0." << std::endl;
    		return;
    	}
    	while (controller::stat == READ);
    	controller::stat = WRITE;
    	controller::gui->refresh();
    	if (controller::current == -1){
			;
    	}
    	else{
    		(*controller::polygons)[controller::current].linear_transform(eye33() * controller::scale, (*controller::polygons)[controller::current].getCentroid());
    	}
        controller::renew_limit();
    	controller::stat = FREE;
     });

    screen->setVisible(true);
    screen->performLayout();
    controller::gui = gui;

    int ret = pthread_create(&p__, NULL, wrap_loop, NULL);
    if (ret != 0){
    	std::cout << "Error creating thread." << std::endl;
    }
}

void controller::load_polygons(){
	std::ifstream input(controller::load_path);
	try{
		if (!input){
			std::cout << "can not find input file" << std::endl;
			return;
		}
		else{
			int number;
			input >> number;
			for (int i = 0; i < number; i++){
				int vertex_number;
				input >> vertex_number;
				std::vector<point_3d> l;
				for (int j = 0; j < vertex_number; j++){
					float x, y, z;
					input >> x;
					input >> y;
					input >> z;
					l.push_back(point_3d(x, y, z));
				}
                for (int j = 0; j < vertex_number; j++){
                    float x, y, z;
                    input >> x;
                    l[j].r = x / 255.0;
                    input >> y;
                    l[j].g = y / 255.0;
                    input >> z;
                    l[j].b = z / 255.0;
                }
				int edge_number;
				input >> edge_number;
				std::vector<int> starts;
                std::vector<int> middles;
				std::vector<int> ends;
				for (int j = 0; j < edge_number; j++){
					int start, middle, end;
					input >> start;
                    input >> middle;
					input >> end;
					starts.push_back(start);
                    middles.push_back(middle);
					ends.push_back(end);
				}
                std::vector<float> specularity;
                for (int j = 0; j < edge_number; j++){
                    int s;
                    input >> s;
                    specularity.push_back(s);
                }
				polyhedron_drawer dr(l, starts, middles, ends, specularity);
                dr.reset_color();
				while (controller::stat == READ);
    			controller::stat = WRITE;
				controller::polygons->push_back(dr);
				controller::current = controller::polygons->size() - 1;
				controller::stat = FREE;
			}
			input.close();
            controller::renew_limit();
			controller::gui->refresh();
		}		
	}
	catch (char* error){
		std::cout << error << std::endl;
	}
}

void controller::move_current_polygon(float delta_x, float delta_y){
	while (controller::stat == READ);
	controller::stat = WRITE;
	if (controller::current == -1){
		;
	}
	else{
		(*controller::polygons)[controller::current].transport(mat21(delta_x, delta_y));
	}
	controller::stat = FREE;
}

void controller::renew_limit(){
    controller::x_max = -999;
    controller::x_min = 999;
    controller::y_max = -9990;
    controller::y_min = 999;
    controller::z_max = -9990;
    controller::z_min = 999;
    for (int i = 0; i < controller::polygons->size(); i++){
        std::vector<point_3d> points = (*controller::polygons)[i].points;
        for (auto p : points){
            if (p.getX() < controller::x_min){
                controller::x_min = p.getX();
            }
            else if (p.getX() > controller::x_max){
                controller::x_max = p.getX();
            }
            if (p.getY() < controller::y_min){
                controller::y_min = p.getY();
            }
            else if (p.getY() > controller::y_max){
                controller::y_max = p.getY();
            }
            if (p.getZ() < controller::z_min){
                controller::z_min = p.getZ();
            }
            else if (p.getZ() > controller::z_max){
                controller::z_max = p.getZ();
            }
        }
    }
    config::x_max = controller::x_max;
    config::x_min = controller::x_min;
    config::y_max = controller::y_max;
    config::y_min = controller::y_min;
    config::scaleX = min(config::width / (config::x_max - config::x_min), config:: height / (config::y_max - config::y_min));
    config::scaleY = config::scaleX;
}