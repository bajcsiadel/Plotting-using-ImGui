//
//  globaldata.c
//  softplot
//
//  Created by András Libál on 8/7/18.
//  Copyright © 2018 András Libál. All rights reserved.
//
#include "imgui.h"
#include "imgui_internal.h"
#include "globaldata.h"
#include "color.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct global_struct global;

const char *movies_dir = "movies";
const char *stats_dir = "stats";

const char *movie_extension = ".mvi";
const char *stat_extension = ".txt";

void initialize_global_data()
{

    global.Windowsize_x = 1280;
    global.Windowsize_y = 1200;

    global.SX = 72.0;
    global.SY = 72.0;
    
    global.zoom_x0 = 0.0;
    global.zoom_x1 = global.SX;
    global.zoom_deltax = global.zoom_x1 - global.zoom_x0;
    
    global.zoom_y0 = 0.0;
    global.zoom_y1 = global.SY;
    global.zoom_deltay = global.zoom_y1 - global.zoom_y0;
    
    global.length = 100;
    global.moviefilename = (char *) malloc(global.length);
    strncpy(global.moviefilename, "../../Time-Crystals/results/movies/10ero_4417.mvi", 50);
    global.moviefilename[50] = '\0';
    
    global.N_frames = 0;
    global.current_frame = 0;
    
    global.movie.trajectories_on = false;
    global.movie.particles_tracked = 5;
    global.movie.traj_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);
    global.movie.traj_width = 0.5;
    global.movie.grid_line_width = 0.5;
    global.movie.show_grid_lines = false;

    global.graph.show_x = true;
    global.graph.show_y = true;
    global.graph.show_z = true;
}

void read_moviefile_data(bool first_call)
{
    unsigned int reserved, i;
    int intholder;
    float floatholder;
    char* filename;

    if (global.objects != NULL) {
        for (i = 0; i < global.N_frames; i++) free(global.objects[i]);
        free(global.objects);
    }

    if (strcmp(get_extension(global.moviefilename), movie_extension) != 0)
    {
        COLOR_ERROR;
        printf("ERROR (globaldata.cpp: line 77)\n\tExtension do not match. Expected %s but got %s\n", movie_extension, get_extension(global.moviefilename));
        COLOR_DEFAULT;

        global.N_frames = 0;
        global.N_objects = 0;
        global.N_particles = 0;
        global.N_pinningsites = 0;

        global.objects = NULL;

        global.pinningsite_r = 0.0;
        global.particle_r = 0.0;

        global.movie.show_grid_lines = false;
        return;
    }

    //open the file, if it cannot be found, exit with error
    global.moviefile = ImFileOpen(global.moviefilename, "rb");
    if (global.moviefile == NULL)
    {
        COLOR_ERROR;
        printf("ERROR (globaldata.cpp: line 73)\n\tCannot find/open movie file: %s\n", global.moviefilename);
        COLOR_DEFAULT;

        global.N_frames = 0;
        global.N_objects = 0;
        global.N_particles = 0;
        global.N_pinningsites = 0;

        global.objects = NULL;

        global.pinningsite_r = 0.0;
        global.particle_r = 0.0;

        global.movie.show_grid_lines = false;
        return;
    }

    //pre-scan the file to find out how many frames/partciles we have
    //this could be modified to be capable of finding the frames that are complete
    printf("Read movie file %s\n", global.moviefilename);
    global.N_frames = 0;
    global.N_objects = 0;
    global.N_pinningsites = 0;
    reserved = 100;
    global.objects = (struct object_struct **) malloc(reserved * sizeof(struct object_struct *));

    while(!feof(global.moviefile))
    {
        if (reserved <= global.N_frames)
        {
            reserved += 50;
            global.objects = (struct object_struct **) realloc(global.objects, reserved * sizeof(struct object_struct *));
        }

        fread(&intholder, sizeof(int), 1, global.moviefile);
        if (global.N_frames == 0)
            global.N_objects = (unsigned int) intholder;
        
        // reading frame number
        fread(&intholder, sizeof(int), 1, global.moviefile);
        
        global.objects[global.N_frames] = (struct object_struct *) malloc(global.N_objects * sizeof(struct object_struct));
        for (i = 0; i < global.N_objects; i++) {
            //read in the color
            fread(&intholder, sizeof(int), 1, global.moviefile);
            global.objects[global.N_frames][i].color = intholder;
            //read in the ID
            fread(&intholder, sizeof(int), 1, global.moviefile);
            //read in the x coordinate
            fread(&floatholder, sizeof(float), 1,  global.moviefile);
            global.objects[global.N_frames][i].x = floatholder;
            //read in the y coordinate
            fread(&floatholder, sizeof(float), 1,  global.moviefile);
            global.objects[global.N_frames][i].y = floatholder;
            //read in extra data that is unused (legacy)
            fread(&floatholder, sizeof(float), 1, global.moviefile);
            global.objects[global.N_frames][i].R = floatholder;
        }
        global.N_frames ++;
    }
    global.N_frames --;
    for (i = 0; i < global.N_objects; i++) 
        if (global.objects[0][i].color != 2 && global.objects[0][i].color != 3) {
            global.N_pinningsites ++;
            global.pinningsite_r = global.objects[0][i].R;
        } else {
            global.particle_r = global.objects[0][i].R;
        }
    global.N_particles = global.N_objects - global.N_pinningsites; 

    printf("Movie has %d frames\n", global.N_frames);
    printf("Movie has %d objects in a frame from which %d particles and %d pinningsites\n", global.N_objects, global.N_particles, global.N_pinningsites);

    fclose(global.moviefile);

    if (first_call)
    {
        size_t len;
        filename = remove_extension(global.moviefilename);
        if (global.statfilename == NULL)
            global.statfilename = (char *) malloc(global.length);
        replace_last(filename, movies_dir, stats_dir);
        len = snprintf(NULL, 0, "%s.mvi", filename);
        if (global.length < len)
        {
            global.length = len + 1;
            global.statfilename = (char *) realloc(global.statfilename, global.length);
        }
        snprintf(global.statfilename, global.length, "%s.txt", filename);
        read_statisticsfile_data(false);
        free(filename);
    }
}

void read_statisticsfile_data(bool first_call)
{
    unsigned int reserved;
    float dummy;
    char* filename;

    if (global.stats != NULL)
        free(global.stats);

    if (strcmp(get_extension(global.statfilename), stat_extension) != 0)
    {
        COLOR_ERROR;
        printf("ERROR (globaldata.cpp: line 202)\n\tExtension do not match. Expected %s but got %s\n", stat_extension, get_extension(global.statfilename));
        COLOR_DEFAULT;

        global.N_stats = 0;
        global.stats = NULL;

        global.graph.show_x = false;
        global.graph.show_y = false;
        global.graph.show_z = false;
        return;
    }

    global.statfile = ImFileOpen(global.statfilename, "r");
    if (global.statfile == NULL)
    {
        COLOR_ERROR;
        printf("ERROR (globaldata.cpp: line 169)\n\tCannot find/open statistics file: %s\n", global.statfilename);
        COLOR_DEFAULT;

        global.N_stats = 0;
        global.stats = NULL;

        global.graph.show_x = false;
        global.graph.show_y = false;
        global.graph.show_z = false;
        return;
    }

    printf("Read stat file %s\n", global.statfilename);
    global.N_stats = 0;
    reserved = 100;
    global.stats = (struct stat_struct *) malloc(reserved * sizeof(struct stat_struct));
    while(!feof(global.statfile))
    {
        if (reserved <= global.N_stats)
        {
            reserved += 50;
            global.stats = (struct stat_struct*) realloc(global.stats, reserved * sizeof(struct stat_struct));
        }

        fscanf(global.statfile, "%d", &global.stats[global.N_stats].time);
        fscanf(global.statfile, "%f", &global.stats[global.N_stats].x);
        fscanf(global.statfile, "%f", &global.stats[global.N_stats].y);
        fscanf(global.statfile, "%f", &global.stats[global.N_stats].z);
        fscanf(global.statfile, "%f", &dummy);
        fscanf(global.statfile, "%f", &dummy);

        global.N_stats ++;
    }

    // there will be a row with zeros because the end of file. Hence decreese the valueable number of raws.
    global.N_stats --;
    fclose(global.statfile);

    if (first_call)
    {
        size_t len;
        filename = remove_extension(global.statfilename);
        if (global.moviefilename == NULL)
            global.moviefilename = (char *) malloc(global.length);
        replace_last(filename, stats_dir, movies_dir);
        len = snprintf(NULL, 0, "%s.mvi", filename);
        if (global.length < len)
        {
            global.length = len + 1;
            global.moviefilename = (char *) realloc(global.moviefilename, global.length);
        }
        snprintf(global.moviefilename, global.length, "%s.mvi", filename);
        read_moviefile_data(false);
        free(filename);
    }
}

void write_frame_data_to_file()
{
    FILE *outfile[6];
    unsigned int i;
    int file_i;
    int color;
    int frame;

    frame = 35;


    outfile[0] = ImFileOpen("hex80k_f35_verttype0.txt", "wt");
    outfile[1] = ImFileOpen("hex80k_f35_verttype1.txt", "wt");
    outfile[2] = ImFileOpen("hex80k_f35_verttype2.txt", "wt");
    outfile[3] = ImFileOpen("hex80k_f35_verttype3.txt", "wt");
    outfile[4] = ImFileOpen("hex80k_f35_verttype4.txt", "wt");
    outfile[5] = ImFileOpen("hex80k_f35_verttypegs.txt", "wt");

    for(i = 0; i < global.N_objects; i++)
    {
        color = global.objects[frame][i].color;
        
        if (color == 10) color = 9;
        if ((color >= 4) && (color <= 9))
        {
            file_i = color - 4;
            fprintf(outfile[file_i], "%f %f\n", global.objects[frame][i].x, global.objects[frame][i].y);
        }
    }

    for(i = 0; i <= 5; i++)
        fclose(outfile[i]);
}

void freeArrays()
{
    free(global.moviefilename);
    free(global.statfilename);
    for (unsigned int i = 0; i < global.N_frames; i++)
        free(global.objects[i]);
    free(global.objects);
    free(global.stats);
}

char* remove_extension(const char* filename)
{
    if (get_extension(filename) != NULL)
    {
        const size_t len = strlen(filename);
        size_t i;

        for (i = len - 1; i >= 0 && filename[i] != '.'; i--);
        return substr(filename, 0, i);
    } else
    {
        return NULL;
    }
    
}

char* get_extension(const char *filename)
{
    const size_t len = strlen(filename);
    int i;

    for (i = len - 1; i >= 0 && filename[i] != '.'; i--);
    if (i == -1) return NULL;
    return substr(filename, i, len - i);
}

char* substr(const char* from, int start, int count)
{
    char* result = (char *) malloc(count + 1);
    int i, n;
    for (i = start, n = 0; i < start + count; i++, n++)
    {
        result[n] = from[i];
    }
    result[n] = '\0';
    return result;
}

void replace_last(char *in, const char *to_replace, const char *replace_with)
{
    int diff;
    size_t n, i, j;
    size_t len_to_replace, len_replace_with, len_in, len_tail;
    char *ptr = strstr(in, to_replace);
    char *tail;

    if (ptr == NULL)
    {
        COLOR_WARNING;
        printf("WARNING (globaldata.cpp: line 283)\n\tNo much found %s in %s\n", to_replace, in);
        COLOR_DEFAULT;
        return;
    }

    len_to_replace = strlen(to_replace);
    len_replace_with = strlen(replace_with);
    len_in = strlen(in);
    n = ptr - in;
    tail = substr(in, n + len_to_replace, len_in - n - len_to_replace);
    len_tail = strlen(tail);

    diff = len_to_replace - len_replace_with;
    if (diff != 0)
        in = (char *) realloc(in, diff + len_in + 1);

    for (i = n, j = 0; i < n + len_replace_with; i++, j++)
        in[i] = replace_with[j];
    for (j = 0; j < len_tail; j++, i++)
        in[i] = tail[j];
    in[i] = '\0';
    free(tail);
}