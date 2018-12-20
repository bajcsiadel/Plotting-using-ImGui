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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct global_struct global;

void initialize_global_data()
{

    global.Windowsize_x = 1280;
    global.Windowsize_y = 800;

    global.SX = 72.0;
    global.SY = 72.0;
    
    global.zoom_x0 = 0.0;
    global.zoom_x1 = global.SX;
    global.zoom_deltax = global.zoom_x1 - global.zoom_x0;
    
    global.zoom_y0 = 0.0;
    global.zoom_y1 = global.SY;
    global.zoom_deltay = global.zoom_y1 - global.zoom_y0;
    
    global.radius_vertex = 0.01;
    global.radius_particle = 0.01;
    
    strncpy(global.moviefilename, "../../Time-Crystals/results/movies/3irany00ero.mvi", 50);
    global.moviefilename[50] = '\0';

    strncpy(global.statfilename, "../../Time-Crystals/results/stats/3irany00ero.txt", 49);
    global.statfilename[49] = '\0';
    
    global.N_frames = 0;
    global.current_frame = 0;
    
    global.trajectories_on = false;
    global.particles_tracked = 5;
    global.traj_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);
    global.traj_width = 0.5;
}

//try to open the movie file, exit if not found with an error message

void open_movie_file()
{

    global.moviefile = ImFileOpen(global.moviefilename, "rb");
    if (global.moviefile == NULL)
    {
        printf("\033[1;31mCannot find/open movie file: %s\033[0m\n", global.moviefilename);
        exit(1);
    }
}

void dummy_read_cmovie_frame()
{
    int intholder;
    float floatholder;
    int i;

    //read one frame but don't store the values (dummy read)
    fread(&intholder, sizeof(int), 1, global.moviefile);
    if (!feof(global.moviefile))
    {
        global.N_objects = intholder;
        //printf("N_particles = %d\n", N_particles);
        fread(&intholder, sizeof(int), 1, global.moviefile);
        //printf("Frame = %d\n", intholder);

        for(i = 0;i < global.N_objects; i++)
        {
            //intholder = global_variables.particles[i].color;
            fread(&intholder, sizeof(int), 1, global.moviefile);
            //intholder = i;//ID
            fread(&intholder, sizeof(int), 1, global.moviefile);
            //floatholder = (float)global_variables.particles[i].x;
            fread(&floatholder, sizeof(float), 1, global.moviefile);
            //floatholder = (float)global_variables.particles[i].y;
            fread(&floatholder, sizeof(float), 1, global.moviefile);
            //floatholder = 1.0;//cum_disp, cmovie format
            fread(&floatholder, sizeof(float), 1, global.moviefile);
        }
    }
}

void read_cmovie_frame(int j)
{
    int intholder;
    float floatholder;
    int i;

    fread(&intholder, sizeof(int), 1, global.moviefile);
    if (!feof(global.moviefile))
    {
        global.N_objects = intholder;
        //printf("N_objects = %d\n", N_objects);
        fread(&intholder, sizeof(int), 1, global.moviefile);
        //printf("Frame = %d\n", intholder);

        for (i = 0; i < global.N_objects; i++)
        {
            //read in the color
            fread(&intholder, sizeof(int), 1, global.moviefile);
            global.objects[j][i].color = intholder;
            //read in the ID
            fread(&intholder, sizeof(int), 1, global.moviefile);
            //read in the x coordinate
            fread(&floatholder, sizeof(float), 1,  global.moviefile);
            global.objects[j][i].x = floatholder;
            //read in the y coordinate
            fread(&floatholder, sizeof(float), 1,  global.moviefile);
            global.objects[j][i].y = floatholder;
            //read in extra data that is unused (legacy)
            fread(&floatholder, sizeof(float), 1, global.moviefile);
            global.objects[j][i].R = floatholder;
        }
    }
}

void read_moviefile_data()
{
    int i;

    //open the file, if it cannot be found, exit with error
    open_movie_file();

    //pre-scan the file to find out how many frames/partciles we have
    //this could be modified to be capable of finding the frames that are complete
    printf("Pre-scanning movie file %s\n", global.moviefilename);
    global.N_frames = 0;
    while (!feof(global.moviefile))
    {
        dummy_read_cmovie_frame();
        if (!feof(global.moviefile)) global.N_frames++;
    }

    printf("Movie has %d frames\n", global.N_frames);
    printf("Movie has %d partciles in a frame\n", global.N_objects);

    //allocate space for the data

    global.objects = (struct object_struct **) malloc(global.N_frames * sizeof(struct object_struct *));
    for(i = 0; i < global.N_frames; i++)
        global.objects[i] = (struct object_struct *) malloc(global.N_objects * sizeof(struct object_struct));
 
    //re-open the movie file
    fclose(global.moviefile);
    global.moviefile = ImFileOpen(global.moviefilename, "rb");

    //read the actual data
    //reads in all the data from the movie file
    //this usually fits in the memory - if not this needs a rewrite
    i = 0;
    while (!feof(global.moviefile) && (i < global.N_frames))
    {
        read_cmovie_frame(i);
        i ++;
    }
    fclose(global.moviefile);
}

void read_statisticsfile_data()
{
    int reserved;

    global.statfile = ImFileOpen(global.statfilename, "r");
    if (global.statfile == NULL)
    {
        printf("\033[1;31mCannot find/open statistics file: %s\033[0m\n", global.statfilename);
        exit(1);
    }

    global.N_stats = 0;
    reserved = 1;
    global.stats = (struct stat_struct*) malloc(reserved * sizeof(struct stat_struct));
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

        global.N_stats ++;
    }

    // there will be a row with zeros because the end of file. Hence decreese the valueable number of raws.
    global.N_stats --;
    fclose(global.statfile);
    global.show_x = true;
    global.show_y = true;
    global.show_z = true;
}

void write_frame_data_to_file()
{
    FILE *outfile[6];
    int i;
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
