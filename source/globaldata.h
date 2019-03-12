//
//  globaldata.h
//  softplot
//
//  Created by András Libál on 8/7/18.
//  Copyright © 2018 András Libál. All rights reserved.
//

#ifndef globaldata_h
#define globaldata_h

#include <stdio.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imguifilesystem.h"

struct object_struct
{
    unsigned int color;
    float x;
    float y;
    float R;
};

struct stat_struct
{
    unsigned int time;
    float *data;
};

struct window_struct
{
    GLFWwindow *window;
    ImVec4 background_color;

    int margin;
};

struct zoom_struct
{
    // 0 - left  upper coner
    // 1 - right lower coner
    ImVec2 corners[2];
    // size of the zoomed area
    float width, height;
};

struct video_window
{
    unsigned int width;
    unsigned int height;

    unsigned int step;

    unsigned int button_size;
    bool play;

    size_t location_length;
    char *play_img_location;
    char *pause_img_location;
    char *rewind_img_location;
    char *fastforward_img_location;
    char *back_img_location;
    char *next_img_location;
};

struct movie_window
{
    // window's sizes
    unsigned int width;
    unsigned int height;

    // window's top left corner
    unsigned int poz_x;
    unsigned int poz_y;

    // the upper corner where drawing starts
    unsigned int draw_x;
    unsigned int draw_y;

    unsigned int draw_width;
    unsigned int draw_height;

    float proportion_x, proportion_y;

    bool trajectories_on;
    unsigned int particles_tracked;
    ImVec4 traj_color;
    float traj_width;

    bool show_grid_lines;
    float grid_line_width;
    ImVec4 grid_color;

    bool show_particles;
    bool show_pinningsites;
    bool show_just_center_pinningsites;

    bool monocrome_particles;
    ImVec4 particle_color;

    bool monocrome_pinningsites;
    ImVec4 pinningsite_color;

    ImDrawList *draw_list;
    struct zoom_struct zoom; 
};

struct graph_window
{
    unsigned int width;
    unsigned int height;

    char **column_names;

    bool show_all;
    bool *show;

    unsigned int t_min, t_max;
    float min, max; // min and max values on the y axis

    ImVec4 *line_colors;
};

struct settings_window
{
    unsigned int width;
    unsigned int height;

    unsigned int poz_x;
    unsigned int poz_y;

    int open; // 0 - movie
              // 1 - stats
};

struct global_struct
{
    char *path;
    size_t path_length;

    //OpenGl window size
    unsigned int Windowsize_x; //window size in pixels x direction
    unsigned int Windowsize_y; //window size in pixels y direction

    //System Size
    double SX;
    double SY;

    float radius_particle;
    float radius_vertex;

    char *moviefilename;
    FILE *moviefile;
    char *moviefile_error;

    unsigned int N_frames;
    int current_frame;

    char *statfilename;
    FILE *statfile;
    char *statfile_error;

    unsigned int length;

    unsigned int N_stats;

    unsigned int N_objects;
    unsigned int N_particles;
    unsigned int N_pinningsites;

    double pinningsite_r;
    double particle_r;

    //all objects in the movie file
    struct object_struct **objects;
    struct window_struct window;
    struct video_window video;
    struct movie_window movie;
    struct graph_window graph;
    struct settings_window settings;

    size_t stat_name_length;
    char **stat_names;
    size_t number_of_columns;
    struct stat_struct *stats;
};

extern struct global_struct global;

void resetZoom();
void reallocateFileNames(size_t);
char *removeExtension(const char *);
char *getExtension(const char *);
char *substr(const char *, int, int);
void replaceLast(char *, const char *, const char *);
void initializeGlobalData(char *);

void getCurrentWorkingDir(char *);
void getRelativePathToProjectRoot(char *, size_t);

void freeArrays();

void readMoviefileData(bool = true);
void readStatisticsfileData(bool = true);

void writeFrameDataToFile();

#endif /* globaldata_h */
