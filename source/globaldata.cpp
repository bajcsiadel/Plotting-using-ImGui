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
#include <time.h>
#include <cstring>

// http://www.codebind.com/cpp-tutorial/c-get-current-directory-linuxwindows/
#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else   // LINUX or MAC
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

struct global_struct global;


void getCurrentWorkingDir(char *current_working_dir) {
    char buff[FILENAME_MAX];
    GetCurrentDir(buff, FILENAME_MAX);
    memcpy(current_working_dir, buff, strlen(buff));
}

void getRelativePathToProjectRoot(char *path, size_t path_size) {
    char *current_working_dir, *plot;

    current_working_dir = (char *) malloc(255);
    getCurrentWorkingDir(current_working_dir);
    plot = strstr(current_working_dir, "Plotting-using-ImGui");
    strncpy(path, "", 1);
    char *buff, *const end = path + path_size;
    buff = path;
    
    for (; *plot != '\0'; plot++) {
        if (*plot == '\\' || *plot == '/') {
            buff += snprintf(buff, end - buff, "../");
        }
    }
    free(current_working_dir);
}

const char *movies_dir = "movies";
const char *stats_dir = "stats";

const char *movie_extension = ".mvi";
const char *stat_extension = ".txt";

void initializeGlobalData(char *filename)
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
    
    size_t len = strlen(filename);
    char default_filename[] = "../../Time-Crystals/results/movies/05pinningsitesR.mvi";
    global.length = (len != 0 ? len : strlen(default_filename)) + 1;
    global.moviefilename = (char *) malloc(global.length);
    if (len == 0) {
        snprintf(filename, global.length, "%s", default_filename);
    }
    strncpy(global.moviefilename, filename, global.length);
    global.moviefilename[global.length] = '\0';

    global.N_frames = 0;
    global.current_frame = 0;
    
    global.movie.trajectories_on = false;
    global.movie.particles_tracked = 5;
    global.movie.traj_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);
    global.movie.traj_width = 0.5;
    global.movie.grid_line_width = 0.5;
    global.movie.show_grid_lines = false;

    global.movie.monocrome_particles = true;
    global.movie.particle_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);

    global.movie.monocrome_pinningsites = false;
    global.movie.pinningsite_color = ImVec4(1.0000, 0.0000, 0.0000, 1.0000);

    global.path_length = 255;
    global.path = (char *) malloc(global.path_length);
    getRelativePathToProjectRoot(global.path, global.path_length);

    // this is the max length of a  location
    global.video.location_length = strlen(global.path) + 21;

    global.video.play_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.play_img_location, global.path);
    strncat(global.video.play_img_location, "img/play.png", 12);

    global.video.pause_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.pause_img_location, global.path);
    strncat(global.video.pause_img_location, "img/pause.png", 13);

    global.video.rewind_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.rewind_img_location, global.path);
    strncat(global.video.rewind_img_location, "img/rewind.png", 14);

    global.video.fastforward_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.fastforward_img_location, global.path);
    strncat(global.video.fastforward_img_location, "img/fast-forward.png", 20);

    global.video.back_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.back_img_location, global.path);
    strncat(global.video.back_img_location, "img/back.png", 12);

    global.video.next_img_location = (char *) malloc(global.video.location_length);
    strcpy(global.video.next_img_location, global.path);
    strncat(global.video.next_img_location, "img/next.png", 12);
}

void reallocateFileNames(size_t len)
{
    if (global.length < len)
    {
        global.length = len + 1;
        global.moviefilename = (char *) realloc(global.moviefilename, global.length);
        global.statfilename  = (char *) realloc(global.statfilename,  global.length);
        if (global.moviefile_error != NULL) global.moviefile_error = (char *) realloc(global.moviefile_error, global.length);
        if (global.statfile_error  != NULL) global.statfile_error  = (char *) realloc(global.statfile_error,  global.length);
    }
}

void readMoviefileData(bool first_call)
{
    unsigned int reserved, i;
    int intholder;
    float floatholder;
    char *filename, *extension;
    bool can_read = true;

    if (global.objects != NULL) {
        for (i = 0; i < global.N_frames; i++) free(global.objects[i]);
        free(global.objects);
    }
    if (global.moviefile_error != NULL)
        free(global.moviefile_error);

    if ((extension = getExtension(global.moviefilename)) == NULL || strcmp(extension, movie_extension) != 0)
    {
        COLOR_ERROR;
        printf("ERROR (%s: line %d)\n\tExtension do not match. Expected %s but got %s\n", strrchr(__FILE__, '/') + 1, __LINE__,movie_extension, extension);
        COLOR_DEFAULT;

        global.moviefile_error = (char *) malloc(global.length);
        size_t len = snprintf(NULL, 0, "%s - Extension do not match. Expected %s but got %s", global.moviefilename, movie_extension, extension);
        reallocateFileNames(len);
        snprintf(global.moviefile_error, global.length, "%s - Extension do not match. Expected %s but got %s", global.moviefilename,  movie_extension, extension);

        global.N_frames = 0;
        global.N_objects = 0;
        global.N_particles = 0;
        global.N_pinningsites = 0;

        global.objects = NULL;

        global.pinningsite_r = 0.0;
        global.particle_r = 0.0;

        global.movie.show_grid_lines = false;
        can_read = false;
        free(extension);
    }
    else
    {
        //open the file, if it cannot be found, exit with error
        global.moviefile = ImFileOpen(global.moviefilename, "rb");
        if (global.moviefile == NULL)
        {
            COLOR_ERROR;
            printf("ERROR (%s: line %d)\n\tCannot find/open movie file: %s\n", strrchr(__FILE__, '/') + 1, __LINE__, global.moviefilename);
            COLOR_DEFAULT;

            global.moviefile_error = (char *) malloc(global.length);
            size_t len = snprintf(NULL, 0, "%s - Cannot find/open movie file", global.moviefilename);
            reallocateFileNames(len);
            snprintf(global.moviefile_error, global.length, "%s - Cannot find/open movie file", global.moviefilename);

            global.N_frames = 0;
            global.N_objects = 0;
            global.N_particles = 0;
            global.N_pinningsites = 0;

            global.objects = NULL;

            global.pinningsite_r = 0.0;
            global.particle_r = 0.0;

            global.movie.show_grid_lines = false;
            can_read = false;
        }
    }

    if (can_read)
    {
        //pre-scan the file to find out how many frames/partciles we have
        //this could be modified to be capable of finding the frames that are complete
        COLOR_NOTE;
        printf("Read movie file %s\n", global.moviefilename);
        COLOR_DEFAULT;

        global.moviefile_error = NULL;

        global.N_frames = 0;
        global.N_objects = 0;
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
        global.pinningsite_r = global.objects[0][0].R;
        // considering that the first element is pinningsite
        global.N_pinningsites = 1;
        for (i = 1; i < global.N_objects; i++) 
            if (global.pinningsite_r == global.objects[0][i].R) {
                global.N_pinningsites ++;
            } 
            else 
                if (global.pinningsite_r < global.objects[0][i].R)
                {
                    double holder = global.pinningsite_r;
                    global.pinningsite_r = global.objects[0][i].R;
                    global.particle_r = holder;
                    global.N_pinningsites = i - global.N_pinningsites;
                    // adding the current element as pinningsite
                    global.N_pinningsites ++;
                } 
                else
                    global.particle_r = global.objects[0][i].R;
        global.N_particles = global.N_objects - global.N_pinningsites; 

        printf("Movie has %d frames\n", global.N_frames);
        printf("Movie has %d objects in a frame from which %d particles and %d pinningsites\n", global.N_objects, global.N_particles, global.N_pinningsites);

        fclose(global.moviefile);
    }

    if (first_call)
    {
        size_t len;
        filename = removeExtension(global.moviefilename);
        if (global.statfilename == NULL)
            global.statfilename = (char *) malloc(global.length);
        replaceLast(filename, movies_dir, stats_dir);
        len = snprintf(NULL, 0, "%s.mvi", filename);
        reallocateFileNames(len);
        snprintf(global.statfilename, global.length, "%s.txt", filename);
        readStatisticsfileData(false);
        free(filename);
    }

    if (global.moviefile_error != NULL) memcpy(global.moviefilename, global.moviefile_error, global.length);
}

void readStatisticsfileData(bool first_call)
{
    unsigned int reserved;
    char* filename, *extension;
    bool can_read = true;
    size_t buff_size = 255;
    char buff[buff_size];

    if (global.stats != NULL)
    {
        for (size_t i = 0; i < global.number_of_columns; i++) free(global.stat_names[i]);
        free(global.stat_names);
        for (size_t i = 0; i < global.N_stats; i++) free(global.stats[i].data);
        free(global.stats);
        free(global.graph.show);
        free(global.graph.line_colors);
    }
    if (global.statfile_error != NULL) 
        free(global.statfile_error);

    if (((extension = getExtension(global.statfilename)) == NULL) || strcmp(extension, stat_extension) != 0)
    {
        COLOR_ERROR;
        printf("ERROR (%s: line %d)\n\tExtension do not match. Expected %s but got %s\n", strrchr(__FILE__, '/') + 1, __LINE__, stat_extension, extension);
        COLOR_DEFAULT;
        
        // we know that statfile_error is NULL because it was deallocated at the beginning of the function
        global.statfile_error = (char *) malloc(global.length);
        size_t len = snprintf(NULL, 0, "%s - Extension do not match. Expected %s but got %s", global.statfilename, stat_extension, extension);
        reallocateFileNames(len);
        snprintf(global.statfile_error, global.length, "%s - Extension do not match. Expected %s but got %s", global.statfilename, stat_extension, extension);

        global.N_stats = 0;
        global.stats = NULL;

        global.graph.show_all = false;
        global.graph.show = NULL;
        global.graph.line_colors = NULL;
        global.stat_names = NULL;
        can_read = false;
        free(extension);
    }
    else 
    {
        global.statfile = ImFileOpen(global.statfilename, "r");
        if (global.statfile == NULL)
        {
            COLOR_ERROR;
            printf("ERROR (%s: line %d)\n\tCannot find/open statistics file: %s\n", strrchr(__FILE__, '/') + 1, __LINE__, global.statfilename);
            COLOR_DEFAULT;

            global.statfile_error = (char *) malloc(global.length);
            size_t len = snprintf(NULL, 0, "%s - Cannot find/open statistics file", global.statfilename);
            reallocateFileNames(len);
            snprintf(global.statfile_error, global.length, "%s - Cannot find/open statistics file", global.statfilename);

            global.N_stats = 0;
            global.stats = NULL;

            global.graph.show_all = false;
            global.graph.show = NULL;
            global.graph.line_colors = NULL;
            global.stat_names = NULL;
            can_read = false;
        }
    }

    if (can_read)
    {
        COLOR_NOTE;
        printf("Read stat file %s\n", global.statfilename);
        COLOR_DEFAULT;

        global.statfile_error = NULL;

        // getting headers from file
        fgets(buff, buff_size, global.statfile);
        // replacing endline character (\n) with null character
        buff[strlen(buff) - 1] = '\0';

        char *ptr;
        reserved = 2;
        global.number_of_columns = 0;
        global.stat_names = (char **) malloc(reserved * sizeof(char *));
        while ((ptr = strrchr(buff, ' ')) != NULL)
        {
            *ptr = '\0';
            ptr ++;
            if (global.number_of_columns == reserved - 1)
            {
                reserved += 2;
                global.stat_names = (char **) realloc(global.stat_names, reserved * sizeof(char *));
            }
            global.stat_names[global.number_of_columns] = (char *) malloc(50);
            size_t len = strnlen(ptr, 50);
            strncpy(global.stat_names[global.number_of_columns], ptr, len);
            global.stat_names[global.number_of_columns][len] = '\0';

            global.number_of_columns ++;
        }

        // flipping the list
        for (size_t i = 0; i < global.number_of_columns / 2; i++)
        {
            char *holder = global.stat_names[i];
            global.stat_names[i] = global.stat_names[global.number_of_columns - i - 1];
            global.stat_names[global.number_of_columns - i - 1] = holder;
        }

        // initially generating random color for the lines on graph
        srand(time(NULL));
        global.graph.line_colors = (ImVec4 *) malloc(global.number_of_columns * sizeof(ImVec4));
        for (size_t i = 0; i < global.number_of_columns; i++)
            global.graph.line_colors[i] = ImVec4((float) rand() / RAND_MAX, (float) rand() / RAND_MAX, (float) rand() / RAND_MAX, 1.0000);

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
            fgets(buff, buff_size, global.statfile);
            // there is an extra space at the end of the row
            buff[strlen(buff) - 2] = '\0';
            global.stats[global.N_stats].data = (float *) malloc( global.number_of_columns * sizeof(float));
            size_t n = global.number_of_columns - 1;
            // there is a space at the begining of the row
            while ((ptr = strrchr(buff, ' ')) != NULL)
            {
                *ptr = '\0';
                ptr ++;
                global.stats[global.N_stats].data[n] = strtof(ptr, NULL);
                n --;
            }

            global.N_stats ++;
        }

        // there will be a row with zeros because the end of file. Hence decreese the valueable number of raws.
        global.N_stats --;
        printf("Statistics file has %d data\n", global.N_stats);
        printf("Statistics file contains %zu nr. of data columns\n", global.number_of_columns + 1);

        global.graph.show_all = true;
        global.graph.show = (bool *) malloc(global.number_of_columns * sizeof(bool));
        for (size_t i = 0; i < global.number_of_columns; i++) global.graph.show[i] = true;

        fclose(global.statfile);
    }

    if (first_call)
    {
        size_t len;
        filename = removeExtension(global.statfilename);
        if (global.moviefilename == NULL)
            global.moviefilename = (char *) malloc(global.length);
        replaceLast(filename, stats_dir, movies_dir);
        len = snprintf(NULL, 0, "%s.mvi", filename);
        reallocateFileNames(len);
        snprintf(global.moviefilename, global.length, "%s.mvi", filename);
        readMoviefileData(false);
        free(filename);
    }

    if (global.statfile_error != NULL) memcpy(global.statfilename, global.statfile_error, global.length);
}

void writeFrameDataToFile()
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
    if (global.objects)
    {
        for (unsigned int i = 0; i < global.N_frames; i++)
            free(global.objects[i]);
        free(global.objects);
    }

    if (global.stats)
    {
        for (size_t i = 0; i < global.number_of_columns; i++) free(global.stat_names[i]);
        free(global.stat_names);
        for (size_t i = 0; i < global.N_stats; i++) free(global.stats[i].data);
        free(global.stats);
    }

    free(global.path);

    free(global.video.play_img_location);
    free(global.video.pause_img_location);
    free(global.video.rewind_img_location);
    free(global.video.fastforward_img_location);
    free(global.video.back_img_location);
    free(global.video.next_img_location);

    if (global.moviefile_error) free(global.moviefile_error);
    if (global.statfile_error)  free(global.statfile_error);
}

char* removeExtension(const char* filename)
{
    if (getExtension(filename) != NULL)
    {
        const size_t len = strlen(filename);
        size_t i;

        for (i = len - 1; i >= 0 && filename[i] != '.'; i--);
        return substr(filename, 0, i);
    }
    else
        return NULL;
    
}

char* getExtension(const char *filename)
{
    const size_t len = strlen(filename);
    int i;

    for (i = len - 1; i >= 0 && (filename[i] != '.' && filename[i] != '/' && filename[i] != '\\'); i--);
    if (i == -1 || filename[i] == '/' || filename[i] == '\\') return NULL;
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

void replaceLast(char *in, const char *to_replace, const char *replace_with)
{
    int diff;
    size_t n, i, j;
    size_t len_to_replace, len_replace_with, len_in, len_tail;
    char *ptr = strstr(in, to_replace);
    char *tail;

    if (ptr == NULL)
    {
        COLOR_WARNING;
        printf("WARNING (%s: line %d)\n\tNo much found %s in %s\n", strrchr(__FILE__, '/') + 1, __LINE__, to_replace, in);
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