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
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imguifilesystem.h"

struct object_struct
{
    int color;
    float x;
    float y;
    float R;
};

struct stat_struct {
    int time;
    float x, y, z;
};

struct window_struct
{
    SDL_DisplayMode current;
    SDL_Window* window;
    SDL_GLContext gl_context;
    ImGuiIO io;
    ImVec4 clear_color;

    int margin;
};

struct video_window
{
    int width;
    int height;

    GLuint play_image;
    GLuint pause_image;
    GLuint next_image;
    GLuint back_image;
    GLuint fastforward_image;
    GLuint rewind_image;

    int button_size;
    bool play;
};

struct movie_window
{
    int width;
    int height;
};

struct graph_window
{
    int width;
    int height;
};

struct settings_window
{
    int width;
    int height;

    int poz_x;
    int poz_y;

    int open; // 0 - movie
              // 1 - stats
};

struct global_struct
{
    
    //OpenGl window size
    int Windowsize_x;           //window size in pixels x direction
    int Windowsize_y;           //window size in pixels y direction
    
    //System Size
    double SX;
    double SY;
    
    //Sytem Zoom in
    float zoom_x0,zoom_y0;         //lower left corner
    float zoom_x1,zoom_y1;         //upper right corner
    float zoom_deltax,zoom_deltay; //size of the zoomed area

    float movie_proportion_x, movie_proportion_y;
    
    float radius_particle;
    float radius_vertex;
    
    char *moviefilename;
    FILE *moviefile;
    
    int N_frames;
    int current_frame;
    
    char *statfilename;
    FILE *statfile;

    int length;

    int N_stats;
    
    bool trajectories_on;
    int particles_tracked;
    ImVec4 traj_color;
    float traj_width;

    int N_objects;
    
    //all objects in the movie file
    struct object_struct **objects;
    struct window_struct window;
    struct video_window video;
    struct movie_window movie;
    struct graph_window graph;
    struct settings_window settings;
    struct stat_struct *stats;
    bool show_x, show_y, show_z;
};

extern struct global_struct global;

void initialize_global_data(void);

void open_movie_file(void);
void dummy_read_cmovie_frame(void);
void read_moviefile_data();
void read_statisticsfile_data();

void write_frame_data_to_file();

#endif /* globaldata_h */
