#ifndef drawing_h
#define drawing_h

#include "globaldata.h"

unsigned int add_file_location(const char*);
void calculate_coordinates_on_graph(int);
int calculate_number_length(double);
void cleanup();
void draw_grid(ImDrawList*);
size_t estimated_label_row_number();
void general_transform_coordinates(unsigned int*, unsigned int, unsigned int, int, bool = false);
void general_transform_coordinates(double*, double, int, int, bool = false);
void init_graph_window(bool*);
void init_movie(bool*);
void init_settings_menubar();
void init_settings_window(bool*);
void init_video_window(bool*);
int init_window();
void max_stats(unsigned int*, double*);
void min_stats(unsigned int*, double*);
void pop_disable();
void push_disable();
void read_image(GLuint*, const char*);
void reset_zoom();
int setup_GLFW();
void set_video_buttons_location();
bool show_at_least_one_stat_data();
void show_help_marker(const char*);
void start_main_loop();
void transform_movie_coordinates(double *x, double *y);
void transform_distance(double *r);
void zoom();

#endif /* drawing_h */