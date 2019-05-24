#ifndef drawing_h
#define drawing_h

#include "globaldata.h"

unsigned int add_file_location(const char* filename);
void calculate_coordinates_on_graph(int frame_nr);
int calculate_number_length(double number);
char *check_path(const char *filename);
void cleanup();
void draw_grid(ImDrawList *draw_list);
size_t estimated_label_row_number();
void general_transform_coordinates(double *x, double x_max, int x_size, int distance_from_origin, bool flip = false);
void general_transform_coordinates(unsigned int *x, unsigned int x_max, unsigned int x_size, int distance_from_origin, bool flip = false);
void init_graph_window(bool *show);
void init_movie();
void init_settings_menubar();
void init_settings_window(bool *show);
void init_video_window(bool *show);
int init_window();
void max_stats(unsigned int *t_max, double *data_max);
void min_stats(unsigned int *t_min, double *data_min);
void pop_disable();
void push_disable();
void read_image(GLuint *texture, const char *filename);
void reset_zoom();
int setup_GLFW();
void set_video_buttons_location();
bool show_at_least_one_stat_data();
void show_help_marker(const char* text);
void start_main_loop();
void transform_movie_coordinates(double *x, double *y, ImVec2 draw_pos, ImVec2 proportion);
void transform_distance(double *r);
void zoom();

#endif /* drawing_h */