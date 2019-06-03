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
#include <cstdarg> // for void something(int arg, ...)

struct global_struct global;

// http://www.codebind.com/cpp-tutorial/c-get-current-directory-linuxwindows/
#ifdef _WIN32
    #include <direct.h>
    #define get_current_dir _getcwd
    int file_exists(const char *filename)
    {
        WIN32_FIND_DATA FindFileData;
        HANDLE handle = FindFirstFile(filename, &FindFileData) ;
        int found = handle != INVALID_HANDLE_VALUE;
        if (found) {
            FindClose(handle);
            return 1;
        } else
            return 0;
    }
#else   // LINUX or MAC
    #include <unistd.h>
    #define get_current_dir getcwd
    int file_exists(const char *filename)
    {
        if (access(filename, F_OK) != -1)
            return 1;
        else
            return 0;
    }
#endif

void get_current_working_dir(char *current_working_dir) {
    char buff[FILENAME_MAX];
    get_current_dir(buff, FILENAME_MAX);
    memcpy(current_working_dir, buff, strlen(buff));
}

void get_relative_path_to_project_root(char *path, size_t path_size) {
    char *current_working_dir, *plot;

    current_working_dir = (char *) malloc(255);
    get_current_working_dir(current_working_dir);
    plot = strstr(current_working_dir, global.project_name.c_str());
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

void initialize_global_data(char *filename)
{

    global.Windowsize_x = 1280;
    global.Windowsize_y = 1200;

    global.SX = 72.0;
    global.SY = 72.0;

    // not good because is specific for triangular lattice
    global.SY = global.SX * (sqrt(3.0) / 2.0);

    // initializing filename
    size_t len = strlen(filename);
    char default_filename[] = "../../Time-Crystals/results/movies/00pinningforce_100particles_20190415_13150.mvi";
    global.length = (len != 0 ? len : strlen(default_filename)) + 1;
    global.moviefilename = (char *) malloc(global.length);
    if (len == 0) {
        snprintf(filename, global.length, "%s", default_filename);
    }
    strncpy(global.moviefilename, filename, global.length);
    global.moviefilename[global.length] = '\0';

    // movie data
    global.N_frames = 0;
    global.current_frame = 0;

    global.project_name = "Plotting-using-ImGui";
}

void reallocate_filenames(size_t len)
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

void read_moviefile_data(bool first_call)
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

    if ((extension = get_extension(global.moviefilename)) == NULL || strcmp(extension, movie_extension) != 0)
    {
        print_log(stdout, ERROR, strrchr(__FILE__, '/') + 1, __LINE__, "Extension do not match", 1,
            "Expected %s but got %s\n", movie_extension, extension);

        global.moviefile_error = (char *) malloc(global.length);
        size_t len = snprintf(NULL, 0, "%s - Extension do not match. Expected %s but got %s", global.moviefilename, movie_extension, extension);
        reallocate_filenames(len);
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
            print_log(stdout, ERROR, strrchr(__FILE__, '/') + 1, __LINE__, "Cannot find/open movie file.", 1,
                "%s\n", global.moviefilename);

            global.moviefile_error = (char *) malloc(global.length);
            size_t len = snprintf(NULL, 0, "%s - Cannot find/open movie file", global.moviefilename);
            reallocate_filenames(len);
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

        print_log(stdout, NOTE, strrchr(__FILE__, '/') + 1, __LINE__, "Movie read.", 3,
            "From file: %s\n", global.moviefilename,
            "Movie has %d frames\n", global.N_frames,
            "Movie has %d objects in a frame from which %d particles and %d pinningsites\n", global.N_objects, global.N_particles, global.N_pinningsites);

        fclose(global.moviefile);
    }

    if (first_call)
    {
        size_t len;
        filename = remove_extension(global.moviefilename);
        if (global.statfilename == NULL)
            global.statfilename = (char *) malloc(global.length);
        replace_last(filename, movies_dir, stats_dir);
        len = snprintf(NULL, 0, "%s.txt", filename);
        reallocate_filenames(len + 1);
        snprintf(global.statfilename, global.length, "%s.txt", filename);
        read_statisticsfile_data(false);
        free(filename);
    }

    if (global.moviefile_error != NULL) memcpy(global.moviefilename, global.moviefile_error, global.length);
}

void read_statisticsfile_data(bool first_call)
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

    if (((extension = get_extension(global.statfilename)) == NULL) || strcmp(extension, stat_extension) != 0)
    {
        print_log(stdout, ERROR, strrchr(__FILE__, '/') + 1, __LINE__, "Extension do not match.", 1, "Expected %s but got %s\n", stat_extension, extension);
        
        // we know that statfile_error is NULL because it was deallocated at the beginning of the function
        global.statfile_error = (char *) malloc(global.length);
        size_t len = snprintf(NULL, 0, "%s - Extension do not match. Expected %s but got %s", global.statfilename, stat_extension, extension);
        reallocate_filenames(len);
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
            print_log(stdout, ERROR, strrchr(__FILE__, '/') + 1, __LINE__, "Cannot find/open file", 1, "%s\n", global.statfilename);

            global.statfile_error = (char *) malloc(global.length);
            size_t len = snprintf(NULL, 0, "%s - Cannot find/open statistics file", global.statfilename);
            reallocate_filenames(len);
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
            global.stats[global.N_stats].data = (double *) malloc( global.number_of_columns * sizeof(double));
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
        print_log(stdout, NOTE, strrchr(__FILE__, '/') + 1, __LINE__, "Statistics read.", 3, 
            "From file: %s\n", global.statfilename,
            "Statistics file has %d data\n", global.N_stats,
            "Statistics file contains %zu nr. of data columns\n", global.number_of_columns + 1);

        global.graph.show_all = true;
        global.graph.show = (bool *) malloc(global.number_of_columns * sizeof(bool));
        for (size_t i = 0; i < global.number_of_columns; i++) global.graph.show[i] = true;

        fclose(global.statfile);
    }

    if (first_call)
    {
        size_t len;
        filename = remove_extension(global.statfilename);
        if (global.moviefilename == NULL)
            global.moviefilename = (char *) malloc(global.length);
        replace_last(filename, stats_dir, movies_dir);
        len = snprintf(NULL, 0, "%s.mvi", filename);
        reallocate_filenames(len + 1);
        snprintf(global.moviefilename, global.length, "%s.mvi", filename);
        read_moviefile_data(false);
        free(filename);
    }

    if (global.statfile_error != NULL) memcpy(global.statfilename, global.statfile_error, global.length);
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

void free_arrays()
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

    if (global.moviefile_error) free(global.moviefile_error);
    if (global.statfile_error)  free(global.statfile_error);
    
    free(global.save.filename);
}

char* remove_extension(const char* filename)
{
    if (get_extension(filename) != NULL)
    {
        const size_t len = strlen(filename);
        size_t i;

        for (i = len - 1; i >= 0 && filename[i] != '.'; i--);
        return substr(filename, 0, i);
    }
    else
        return NULL;
    
}

char* get_extension(const char *filename)
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

size_t number_of_percentage(char *format)
{
    size_t perc = 0;
    bool backslash;
    for (char *traverse = format; *traverse != '\0'; traverse++)
    {
        if (*traverse == '%' && !backslash)
            perc ++;
        if (*traverse == '\\')
            backslash = true;
        else
            backslash = false;
    }
    return perc;
}

int print_log(FILE *stream, LogTypes type, const char *filename, const int line, const char *title, const size_t format_number ...)
{
    time_t now = time(NULL);
    char buff[20];
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    COLOR_LOG;
    printf("LOG [%s]: ", buff);
    COLOR_DEFAULT;

    switch (type)
    {
    case ERROR:
        COLOR_ERROR;
        printf("ERROR ");
        break;

    case WARNING:
        COLOR_WARNING;
        printf("WARNING ");
        break;

    case NOTE:
        COLOR_NOTE;
        printf("NOTE ");
        break;
    
    default:
        COLOR_WARNING;
        printf("WARNING: LogType not defined!");
        break;
    }
    printf("(%s: line %d) ", filename, line);
    if (title) printf("%s", title);
    printf("\n");
    COLOR_DEFAULT;

    int done = 1;
    va_list args;
    va_start(args, format_number);
    size_t perc = 0;
    for (size_t i = 0; i < format_number; i++)
    {
        char *format = va_arg(args, char *);
        done &= vfprintf (stdout, format, args);
        perc += number_of_percentage(format) + 1;
        va_start(args, format_number);
        for (size_t j = 0; j < perc; j++)
            va_arg(args, int);
    }
    va_end(args);

    return done;
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
    if (diff < 0)
        in = (char *) realloc(in, diff + len_in + 1);

    for (i = n, j = 0; i < n + len_replace_with; i++, j++)
        in[i] = replace_with[j];

    for (j = 0; j < len_tail; j++, i++)
        in[i] = tail[j];
    in[i] = '\0';
    free(tail);
}