#include "drawing.h"
#include "globaldata.h"
#include "color.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "imguifilesystem.h"
#include "imgui_internal.h"

#include <SFML/Graphics/Image.hpp>

#include <stdio.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <cmath>

using namespace ImGui;
using namespace ImGuiFs;

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

const ImVec4 colors[] = {
    ImVec4(1.0000, 0.0000, 0.0000, 1.0000), // 0 - red
    ImVec4(0.7000, 0.7000, 0.7000, 1.0000), // 1 - gray
    ImVec4(0.7000, 0.7000, 0.7000, 1.0000), // 2 - gray
    ImVec4(0.0000, 0.0000, 0.0000, 1.0000), // 3 - black
    ImVec4(0.8941, 0.1019, 0.1098, 1.0000), // 4 - red shade
    ImVec4(0.4941, 0.6117, 0.9215, 1.0000), // 5 - blue
    ImVec4(0.3019, 0.6862, 0.2901, 1.0000), // 6 - green
    ImVec4(0.8900, 0.6120 ,0.2160, 1.0000), // 7 - orange
    ImVec4(0.8500, 0.1290, 0.1250, 1.0000), // 8 - almost red
};

int setupGLFW()
{
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 0;
    return 1;
}

int initWindow()
{
    // Setup window
    global.window.window = glfwCreateWindow(global.Windowsize_x, global.Windowsize_y, "Plot", NULL, NULL);
    if (global.window.window == NULL)
    {
        COLOR_ERROR;
        printf("ERROR (%s: line %d)\n\tCould not create window!\n", strrchr(__FILE__, '/') + 1, __LINE__);
        COLOR_DEFAULT;
        return 0;
    }

    glfwMakeContextCurrent(global.window.window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    CreateContext();
    global.window.io = GetIO(); (void)global.window.io;
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(global.window.window, true);
    ImGui_ImplOpenGL2_Init();

    // Setup Style
    StyleColorsLight();

    global.SY = global.SX * (sqrt(3.0) / 2.0);

    global.window.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    global.window.margin = 10;
    global.video.button_size = 28;

    global.video.height = global.video.width
        = global.Windowsize_y * 2 / 3 - 2 * global.window.margin;
    global.video.play = true;
    global.video.step = 1;

    global.movie.width = global.video.width - 2 * global.window.margin;
    global.movie.height = global.video.height - 5 * global.window.margin - global.video.button_size - 20; // filename height
        
    global.movie.proportion_x = ((double) global.movie.width - 20) / global.SX;
    global.movie.proportion_y = ((double) global.movie.height - 20) / global.SY;

    global.movie.poz_x = 3 * global.window.margin;
    global.movie.grid_color = colors[1];

    global.movie.show_particles = true;
    global.movie.show_pinningsites = true;

    global.graph.width = global.video.width;
    global.graph.height = global.video.height / 2;

    global.settings.width = global.Windowsize_x - global.video.width - 3 * global.window.margin;
    global.settings.height = global.Windowsize_y - 2 * global.window.margin;

    global.settings.poz_x = global.window.margin + global.video.width + global.window.margin;
    global.settings.poz_y = global.window.margin;

    return 1;
}

void drawGrid(ImDrawList *draw_list)
{
    unsigned int N_rows, N_columns;
    unsigned int i;
    float dx, dy;
    ImU32 color;

    N_columns = (int) (global.SX / (2 * global.pinningsite_r)) + 1;
    N_rows = (int) (global.SY / (2 * global.pinningsite_r)) + 1;

    dx = global.SX / N_columns;
    dy = global.SY / N_rows;

    color = ImColor(global.movie.grid_color);

    for (i = 1; i < N_rows; i++) {
        float y0 = i * dy, y1 = i * dy;
        float x0 = 0.0, x1 = global.SX;

        transformMovieCoordinates(&x0, &y0);
        transformMovieCoordinates(&x1, &y1);

        draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, global.movie.grid_line_width);
    }

    for (i = 1; i < N_columns; i++) {
        float x0 = i * dx, x1 = i * dx;
        float y0 = 0.0, y1 = global.SY;

        transformMovieCoordinates(&x0, &y0);
        transformMovieCoordinates(&x1, &y1);

        draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, global.movie.grid_line_width);
    }
}

void initMovie(bool show_video_window)
{
    unsigned int i, n, c;
    int j;
    float x, y, r, x1, y1, x2, y2;
    ImDrawList *draw_list;
    ImVec2 poz;

    BeginChild("Test", ImVec2(global.movie.width, global.movie.height), true);

        float sy = global.SY;
        transformDistance(&sy);
        poz = GetWindowPos();
        global.movie.poz_y = (global.movie.height - sy) / 2 + poz.y + 30;

        draw_list = GetWindowDrawList();
        if (global.movie.show_grid_lines)
            drawGrid(draw_list);
        n = 0;
        if (global.objects != NULL)
        {
            for (i = 0; i < global.N_objects; i++)
            {
                x = global.objects[global.current_frame][i].x;
                y = global.objects[global.current_frame][i].y;
                r = global.objects[global.current_frame][i].R;
                transformMovieCoordinates(&x, &y);
                transformDistance(&r);
                c = global.objects[global.current_frame][i].color;
                if (c < 0 || c > 9) c = 0;
                ImU32 col32 = ImColor(colors[c]);
                if (global.objects[global.current_frame][i].R == global.particle_r) 
                {
                    if (global.movie.show_particles) {
                        if (global.movie.monocrome_particles) col32 = ImColor(global.movie.particle_color);
                        draw_list->AddCircleFilled(ImVec2(x, y), r, col32, 36);
                        if (global.movie.trajectories_on)
                        {
                            if (n < global.movie.particles_tracked)
                            {
                                n ++;
                                for (j = 0; j < global.current_frame - 1; j++)
                                {
                                    x1 = global.objects[j][i].x;
                                    y1 = global.objects[j][i].y;

                                    x2 = global.objects[j + 1][i].x;
                                    y2 = global.objects[j + 1][i].y;
                                    if ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < 2.0)
                                    {
                                        transformMovieCoordinates(&x1, &y1);
                                        transformMovieCoordinates(&x2, &y2);
                                        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(global.movie.traj_color), global.movie.traj_width);
                                    }
                                }
                            }
                        }
                    }
                }
                else 
                    if (global.movie.show_pinningsites) {
                        if (global.movie.monocrome_pinningsites) col32 = ImColor(global.movie.pinningsite_color);
                        if (global.movie.show_just_center_pinningsites)
                            draw_list->AddCircle(ImVec2(x, y), r / 3, col32, 36, 1);
                        else
                            draw_list->AddCircle(ImVec2(x, y), r, col32, 36, 1);
                    }
            }
        }
    EndChild();
}

void initVideoWindow(bool *show_video_window)
{
    SetNextWindowPos(ImVec2(global.window.margin, global.window.margin));
    SetNextWindowSize(ImVec2(global.video.width, global.video.height));
    Begin("Video", show_video_window,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove);
        AddFileLocation(global.moviefilename);
        initMovie(true);
        
        GLuint play_image, pause_image, back_image, next_image, rewind_image, fastforward_image;
        // https://www.flaticon.com/packs/music
        readImage(&play_image, global.video.play_img_location);
        readImage(&pause_image, global.video.pause_img_location);
        readImage(&back_image, global.video.back_img_location);
        readImage(&next_image, global.video.next_img_location);
        readImage(&rewind_image, global.video.rewind_img_location);
        readImage(&fastforward_image, global.video.fastforward_img_location);
        
        PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 1.0, 1.0, 1.0));
        PushStyleColor(ImGuiCol_ButtonActive, global.window.clear_color);

        Separator();
        ImageButton((void *) (intptr_t) back_image, ImVec2(global.video.button_size, global.video.button_size));
        if (IsItemClicked())
            global.current_frame = 0;
        SameLine();

        ImageButton((void *) (intptr_t) rewind_image, ImVec2(global.video.button_size, global.video.button_size));
        if (IsItemClicked())
            global.video.step = (global.video.step != 1 ? global.video.step / 2 : global.video.step);
        SameLine();

        if (global.video.play) 
            ImageButton((void *) (intptr_t) pause_image, ImVec2(global.video.button_size, global.video.button_size));
        else
            ImageButton((void *) (intptr_t) play_image, ImVec2(global.video.button_size, global.video.button_size));
        if (IsItemClicked())
            global.video.play = !global.video.play;
        SameLine();

        ImageButton((void *) (intptr_t) fastforward_image, ImVec2(global.video.button_size, global.video.button_size));
        if (IsItemClicked())
            global.video.step = (global.video.step < 128 ? global.video.step * 2 : global.video.step);
        SameLine();

        ImageButton((void *) (intptr_t) next_image, ImVec2(global.video.button_size, global.video.button_size));
        if (IsItemClicked())
            global.current_frame = global.N_frames - 1;
        SameLine();
        PopStyleColor(2);

        PushItemWidth(-50);
        SliderInt("Frames", &global.current_frame, 0, global.N_frames - 1);
    End();
    if (global.video.play)
        global.current_frame = (global.current_frame < (int) global.N_frames - (int) global.video.step ? global.current_frame + global.video.step : 0);
}

void drawDecartesCoordinateSystem(ImDrawList *draw_list, 
    unsigned int *poz_x, unsigned int *poz_y, 
    unsigned int *size_x, unsigned int *size_y, 
    unsigned int x_max, ImVec2 y_lims, unsigned int *origins_y_pozition)
{
    // x_lims = ImVec2(t_min, t_max)
    // y_lims = ImVec2(min, max)
    unsigned int j, t_value, t_step, axis, tick_poz;
    float range, y_value, y_step;
    ImU32 black, gray;
    const unsigned int a = 7;
    const unsigned int x = *poz_x,
        y = *poz_y,
        y2 = *poz_y + *size_y;
    const unsigned int x0 = *poz_x - 50,
        x01 = *poz_x + *size_x,
        y0 = (y_lims.x < 0 ? *poz_y + *size_y * y_lims.y / (y_lims.y - y_lims.x) : *poz_y + *size_y - 2 * a);

    gray    = ImColor(colors[2]);
    black   = ImColor(colors[3]);

    // place thicks on vertical axis
    range = y_lims.y - (y_lims.x  < 0 ? y_lims.x : 0.0f);
    y_step = range / 4 / 4;
    axis = (y0 - *poz_y) / 4 / 4;
    for (y_value = y_step, tick_poz = y0 - axis, j = 1; y_value < y_lims.y; y_value += y_step, tick_poz -= axis, j++) {
        draw_list->AddLine(ImVec2(x, tick_poz), ImVec2(x01 * 2, tick_poz), gray);
        if (j % 4 == 0 || y_value + y_step >= y_lims.y) {
            char *number = new char[10];
            size_t len = snprintf(number, 9, "%.3f", y_value);

            draw_list->AddLine(ImVec2(x - 7, tick_poz), ImVec2(x + 7, tick_poz), black);
            draw_list->AddText(ImVec2(x - len * 8, tick_poz - 7), black, number);
            *poz_y = tick_poz;

            delete[] number;
        } else {
            draw_list->AddLine(ImVec2(x - 4, tick_poz), ImVec2(x + 4, tick_poz), black);
        }
    }
    for (y_value = -y_step, tick_poz = y0 + axis, j = 1; y_value >= y_lims.x; y_value -= y_step, tick_poz += axis, j++) {
        draw_list->AddLine(ImVec2(x, tick_poz), ImVec2(x01 * 2, tick_poz), gray);
        if (j % 4 == 0 || y_value - y_step < y_lims.x) {
            char *number = new char[10];
            size_t len = snprintf(number, 9, "%.3f", y_value);

            draw_list->AddLine(ImVec2(x - 7, tick_poz), ImVec2(x + 7, tick_poz), black);
            draw_list->AddText(ImVec2(x - len * 8, tick_poz - 7), black, number);
            *size_y = tick_poz - *poz_y;

            delete[] number;
        } else {
            draw_list->AddLine(ImVec2(x - 4, tick_poz), ImVec2(x + 4, tick_poz), black);
        }
    }

    // vertical axis
    draw_list->AddLine(ImVec2(x, y), ImVec2(x, y2), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x, y), ImVec2(x - a / 2, y + sqrt(3) * a / 2), black);
    draw_list->AddLine(ImVec2(x, y), ImVec2(x + a / 2, y + sqrt(3) * a / 2), black);

    // ticks on horisontal axis
    t_step = x_max / 5 / 4;
    axis = (x01 - sqrt(3) * a / 2 - x0 - 50) / 20;
    for (t_value = t_step, tick_poz = *poz_x + axis, j = 1; t_value <= x_max; t_value += t_step, tick_poz += axis, j++) {
        draw_list->AddLine(ImVec2(tick_poz, 0), ImVec2(tick_poz, 2 * y2), gray);
        if (j % 5 == 0) {
            char *number = new char[10];
            size_t len = snprintf(number, 9, "%d", t_value);

            draw_list->AddLine(ImVec2(tick_poz, y0 - 7), ImVec2(tick_poz, y0 + 7), black);
            draw_list->AddText(ImVec2(tick_poz - len * 4, y0 + 7), black, number);
            *size_x = tick_poz - *poz_x;

            delete[] number;
        } else {
            draw_list->AddLine(ImVec2(tick_poz, y0 - 4), ImVec2(tick_poz, y0 + 4), black);
        }
    }

    // horizontal axis
    draw_list->AddLine(ImVec2(x0, y0), ImVec2(x01, y0), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 - a / 2), black);
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 + a / 2), black);

    draw_list->AddText(ImVec2(x - 8, y0), black, "0");

    *poz_x = x;
    *origins_y_pozition = (unsigned int)y0 - *poz_y;
}

bool showAtLeastOneStatData()
{
    bool show = false;
    for (size_t j = 0; j < global.number_of_columns; j++)
        show |= global.graph.show[j];
    return show;
}

void initGraphWindow(bool *show)
{
    unsigned int size_x, size_y;
    unsigned int poz_x, poz_y;
    unsigned int i, y0;
    int time_index;
    unsigned int t1, t2, t_max, t_min, t_frame, t0, tn;
    float *data1;
    float *data2;
    float *data_max, max;
    float *data_min, min;
    ImDrawList *draw_list;

    poz_x = global.window.margin;
    poz_y = global.window.margin + global.video.height + global.window.margin;

    data1 = (float *) malloc(sizeof(float) * global.number_of_columns);
    data2 = (float *) malloc(sizeof(float) * global.number_of_columns);
    
    SetNextWindowPos(ImVec2(poz_x, poz_y));
    SetNextWindowSize(ImVec2(global.graph.width, global.graph.height));
    Begin("Graph", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove);
        AddFileLocation(global.statfilename);
        size_x = global.graph.width - 2 * global.window.margin;
        size_y = global.graph.height - 8.2 * global.window.margin;
        BeginChild("", ImVec2(size_x, size_y), true);
            draw_list = GetWindowDrawList();
            
            if (global.stats != NULL)
            {
                poz_x += 60;
                poz_y += 52;
                size_y -= 16;
                size_x -= 60;

                data_max = (float *) malloc(sizeof(float) * global.number_of_columns);
                data_min = (float *) malloc(sizeof(float) * global.number_of_columns);

                maxStats(&t_max, data_max);
                minStats(&t_min, data_min);

                min = HUGE_VAL_F32;
                for (size_t j = 0; j < global.number_of_columns - 1; j++)
                    if (global.graph.show[j] && min > data_min[j]) min = data_min[j];
                if ((global.graph.show[global.number_of_columns - 1] && min > data_min[global.number_of_columns - 1]) || min == HUGE_VAL_F32) min = data_min[global.number_of_columns - 1];

                max = (-1) * HUGE_VAL_F32;
                for (size_t j = 0; j < global.number_of_columns - 1; j++)
                    if (global.graph.show[j] && max < data_max[j]) max = data_max[j];
                if ((global.graph.show[global.number_of_columns - 1] && max < data_max[global.number_of_columns - 1]) || max == (-1) * HUGE_VAL_F32) max = data_max[global.number_of_columns - 1];
                
                free(data_max);
                free(data_min);

                drawDecartesCoordinateSystem(draw_list, &poz_x, &poz_y, &size_x, &size_y, t_max, ImVec2(min, max), &y0);

                max = (min < 0 ? max - min : max);

                memcpy(data2, global.stats[0].data, sizeof(float) * global.number_of_columns);
                t2 = global.stats[0].time;

                generalTransformCoordinates(&t2, t_max, size_x, poz_x);
                for (size_t j = 0; j < global.number_of_columns; j++)
                {
                    generalTransformCoordinates(&data2[j], max, size_y, poz_y, true);
                    // it has to shift the curves with the distance of negative values (size_y - y0)
                    data2[j] -= size_y - y0;
                }

                t0 = t2;
                t_frame = global.current_frame * 100;
                generalTransformCoordinates(&t_frame, t_max, size_x, poz_x);
                time_index = -1;
                for (i = 1; i < global.N_stats; i++)
                {
                    memcpy(data1, data2, sizeof(float) * global.number_of_columns);
                    tn = t1 = t2;

                    memcpy(data2, global.stats[i].data, sizeof(float) * global.number_of_columns);
                    t2 = global.stats[i].time;

                    generalTransformCoordinates(&t2, t_max, size_x, poz_x);
                    for (size_t j = 0; j < global.number_of_columns; j++)
                    {
                        generalTransformCoordinates(&data2[j], max, size_y, poz_y, true);
                        // it has to shift the curves with the distance of negative values (size_y - y0)
                        data2[j] -= size_y - y0;
                        if (global.graph.show[j]) draw_list->AddLine(ImVec2(t1, (int) data1[j]), ImVec2(t2, (int) data2[j]), ImColor(colors[6 + j]), 0.5);
                    }

                    if (showAtLeastOneStatData())
                        if (t_frame > t1 && t_frame <= t2)
                        {
                            time_index = i;  // t_frame value is between (i - 1) and i
                            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }
                }

                free(data1);
                free(data2);

                if (showAtLeastOneStatData())
                    if (time_index == -1)
                    {
                        if (t_frame <= t0)
                        {
                            time_index = 0;
                            t_frame += 2;
                            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }

                        if (t_frame > tn)
                        {
                            time_index = global.N_stats - 1;
                            t_frame -= 2;
                            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }
                    }
            }
        EndChild();
        if (global.stats != NULL) calculateCoordinatesOnGraph(time_index);
    End();
}

void calculateCoordinatesOnGraph(int i)
{
    float *data1, *data2;
    float *values;
    int t1, t2, t_frame;
    char *text;

    data1 = (float *) malloc(sizeof(float) * global.number_of_columns);
    data2 = (float *) malloc(sizeof(float) * global.number_of_columns);
    values = (float *) malloc(sizeof(float) * global.number_of_columns);

    t_frame = global.current_frame * 100;
    memcpy(data2, global.stats[i].data, sizeof(float) * global.number_of_columns);
    t2 = global.stats[i].time;

    if (i != 0)
    {
        memcpy(data1, global.stats[i].data, sizeof(float) * global.number_of_columns);
        t1 = global.stats[i - 1].time;
    } else {
        memcpy(data1, data2, sizeof(float) * global.number_of_columns);
        t1 = t2;
    }
    
    if (t1 == t2 || t1 == t_frame) 
        memcpy(values, data1, sizeof(float) * global.number_of_columns);
    else
        for (size_t j = 0; j < global.number_of_columns; j++)
            values[j] = ((t_frame - t1) * (data2[j] - data1[j])) / (t2 - t1) + data1[j];

    for (size_t j = 0; j < global.number_of_columns; j++)
        if (global.graph.show[j])
        {
            text = (char*) malloc(100);
            snprintf(text, 100, "%s = %2.8f\t", global.stat_names[j], values[j]);
            TextColored(colors[6 + j], "%s", text);
            SameLine();
            free(text);
        }

    free(values);
    free(data2);
    free(data1);
}

void initSettingsMenuBar()
{
    size_t length;
    static ImGuiFs::Dialog dlg;
    bool open_movie, open_stats;
    // bool save_movie, save_stats;

    open_movie = false;
    open_stats = false;

    // save_movie = false;
    // save_stats = false;
    
    if (BeginMenuBar())
    {
        if (BeginMenu("File"))
        {
            if (BeginMenu("Open"))
            {
                if (MenuItem("Movie file", "CTRL+M")) 
                {
                    open_movie = true;
                    global.settings.open = 0;
                }
                
                if (MenuItem("Statistics file", "CTRL+T"))
                {
                    open_stats = true;
                    global.settings.open = 1;
                }
                EndMenu();
            }
            if (BeginMenu("Save"))
            {
                MenuItem("Movie to avi");
                MenuItem("Graph");
                EndMenu();
            }
            EndMenu();
        }
        if (BeginMenu("Edit"))
        {
            if (MenuItem("Undo", "CTRL+Z")) {}
            if (MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            Separator();
            if (MenuItem("Cut", "CTRL+X")) {}
            if (MenuItem("Copy", "CTRL+C")) {}
            if (MenuItem("Paste", "CTRL+V")) {}
            EndMenu();
        }
        EndMenuBar();
    }
    if (open_movie || open_stats) global.video.play = false;
    dlg.chooseFileDialog(open_movie || open_stats, "../../Time-Crystals/results", ".mvi;.txt");
    if ((length = strlen(dlg.getChosenPath())) > 0)
    {
        if (strncmp(dlg.getChosenPath(), global.moviefilename, (length < global.length ? length : global.length)) != 0 &&
            strncmp(dlg.getChosenPath(), global.statfilename, (length < global.length ? length : global.length)) != 0)
        {
            if (length > global.length)
            {
                global.length = length;
                global.moviefilename = (char *) realloc(global.moviefilename, length);
                global.statfilename = (char *) realloc(global.statfilename, length);
            }
            if (global.settings.open == 1)
            {
                strncpy(global.statfilename, dlg.getChosenPath(), length);
                global.statfilename[length] = '\0';
                readStatisticsfileData();
                open_stats = false;
                global.settings.open = -1;
            } else if (global.settings.open == 0)
            {
                strncpy(global.moviefilename, dlg.getChosenPath(), length);
                global.moviefilename[length] = '\0';
                readMoviefileData();
                open_movie = false;
                global.settings.open = -1;
            }
            global.current_frame = 0;
            global.video.play = true;
        }
    }
}

void initSettingsWindow(bool *show)
{
    SetNextWindowPos(ImVec2(global.settings.poz_x, global.settings.poz_y));
    SetNextWindowSize(ImVec2(global.settings.width, global.settings.height));
    Begin("Settings", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_MenuBar);

        initSettingsMenuBar();

        if (CollapsingHeader("Help"))
        {
            BulletText("Double-click on title bar to collapse window.");
            BulletText("CTRL+Click on a slider to input value as text.");
        }

        if (global.objects == NULL) pushDisable();
        if (CollapsingHeader("Movie"))
        {
            Text("Pinningsites");
            if (global.N_pinningsites == 0) pushDisable();
            Checkbox("Show pinningsites", &global.movie.show_pinningsites);
            if (!global.movie.show_pinningsites) pushDisable();
            Checkbox("Show pinningsite grid lines", &global.movie.show_grid_lines);
            if (global.movie.show_grid_lines) {
                ColorEdit3("Grid line color", (float*)&global.movie.grid_color);
                DragFloat("Grid line width", &global.movie.grid_line_width, 0.05f, 0.1f, 5.0f, "%.2f");
            }
            Checkbox("Decreese pinningsite radius", &global.movie.show_just_center_pinningsites);
            Checkbox("Monocrome pinningsites", &global.movie.monocrome_pinningsites);
            if (global.movie.monocrome_pinningsites)
                ColorEdit3("Pinningsite color", (float*)&global.movie.pinningsite_color);
            Separator();
            if (!global.movie.show_pinningsites) popDisable();
            if (global.N_pinningsites == 0) popDisable();

            Text("Particles");
            if (global.N_particles == 0) pushDisable();
            Checkbox("Show particles", &global.movie.show_particles);
            if (!global.movie.show_particles) pushDisable();
            Checkbox("Toggle trajectory", &global.movie.trajectories_on);
            if (global.movie.trajectories_on)
            {
                float spacing = GetStyle().ItemInnerSpacing.x;
                PushButtonRepeat(true);
                if (ArrowButton("##left", ImGuiDir_Left)) { if (global.movie.particles_tracked > 1) global.movie.particles_tracked--; }
                SameLine(0.0f, spacing);
                if (ArrowButton("##right", ImGuiDir_Right)) { if (global.movie.particles_tracked < global.N_objects/4) global.movie.particles_tracked++; }
                PopButtonRepeat();
                SameLine();
                Text("%d", global.movie.particles_tracked);

                ColorEdit3("Trajectory color", (float*)&global.movie.traj_color);
                SameLine(); 
                ShowHelpMarker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");
                DragFloat("Trajectory width", &global.movie.traj_width, 0.05f, 0.1f, 5.0f, "%.2f");
                ShowHelpMarker("Click and drag to change the value");
            }
            Checkbox("Monocrome particles", &global.movie.monocrome_particles);
            if (global.movie.monocrome_particles)
                ColorEdit3("Particle color", (float*)&global.movie.particle_color);
            if (!global.movie.show_particles) popDisable();
            Separator();
            if (global.N_particles == 0) popDisable();

            if (TreeNode("Zoom"))
            {
                TreePop();
                Separator();
            }
        }
        if (global.objects == NULL) popDisable();

        if (global.stats == NULL) pushDisable();
        if (CollapsingHeader("Graph"))
        {
            if (TreeNode("Data shown"))
            {
                for (size_t j = 0; j < global.number_of_columns; j++)
                    Checkbox(global.stat_names[j], &global.graph.show[j]);
                TreePop();
                Separator();
            }
        }
        if (global.stats == NULL) popDisable();
    
    End();
}
    
void startMainLoop()
{
    // Main loop
    while (!glfwWindowShouldClose(global.window.window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        NewFrame();

        initVideoWindow(NULL);
        initGraphWindow(NULL);
        initSettingsWindow(NULL);

        // Rendering
        Render();
        int display_w, display_h;
        glfwGetFramebufferSize(global.window.window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(global.window.clear_color.x, global.window.clear_color.y, global.window.clear_color.z, global.window.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL2_RenderDrawData(GetDrawData());
        glfwMakeContextCurrent(global.window.window);
        glfwSwapBuffers(global.window.window);

    }
    cleanup();
}

void cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    DestroyContext();

    glfwDestroyWindow(global.window.window);
    glfwTerminate();
}

void maxStats(unsigned int *t_max, float *data_max)
{
    unsigned int i, j;

    if (global.stats != NULL) 
    {
        *t_max = global.stats[0].time;
        for (j = 0; j < global.number_of_columns; j++) data_max[j] = global.stats[0].data[j];

        for (i = 1; i < global.N_stats; i++)
        {
            if (global.stats[i].time > *t_max) *t_max = global.stats[i].time;
            for (j = 0; j < global.number_of_columns; j++)
                if (global.stats[i].data[j] > data_max[j]) data_max[j] = global.stats[i].data[j];
        }
    } else {
        COLOR_WARNING;
        printf("WARNING (%s: line %d)\n\tNo statistics data found!\n", strrchr(__FILE__, '/') + 1, __LINE__);
        COLOR_DEFAULT;
    }
}

void minStats(unsigned int *t_min, float *data_min)
{
    unsigned int i, j;

    if (global.stats != NULL)
    {
        *t_min = global.stats[0].time;
        for (j = 0; j < global.number_of_columns; j++) data_min[j] = global.stats[0].data[j];

        for (i = 1; i < global.N_stats; i++)
        {
            if (global.stats[i].time < *t_min) *t_min = global.stats[i].time;
            for (j = 0; j < global.number_of_columns; j++)
                if (global.stats[i].data[j] < data_min[j]) data_min[j] = global.stats[i].data[j];
        }
    } else {
        COLOR_WARNING;
        printf("WARNING (%s: line %d)\n\tNo statistics data found!\n", strrchr(__FILE__, '/') + 1, __LINE__);
        COLOR_DEFAULT;
    }
}

void generalTransformCoordinates(float *x, float x_max, int x_size, int distance_from_origin, bool flip)
{
    if (flip) *x = x_max - *x;
    *x = *x * (float) x_size / x_max + distance_from_origin;
}

void generalTransformCoordinates(unsigned int *x, unsigned int x_max, unsigned int x_size, int distance_from_origin, bool flip)
{
    if (flip) *x = x_max - *x;
    *x = *x * x_size / x_max + distance_from_origin;
}

void transformMovieCoordinates(float *x, float *y)
{
    *x = *x - global.zoom_x0;
    *y = *y - global.zoom_y0;
    *y = global.SY - *y;

    *x = *x / global.zoom_deltax * global.movie.proportion_x * global.SX + global.movie.poz_x;
    *y = *y / global.zoom_deltay * global.movie.proportion_y * global.SY + global.movie.poz_y;
}

//transforms from simulation unit distances to Opengl units
void transformDistance(float *r)
{
    *r = *r * global.movie.proportion_x;
}

void readImage(GLuint *texture, const char *filename)
{
    sf::Image img_data;
    if (!img_data.loadFromFile(filename))
    {
        COLOR_ERROR;
        printf("ERROR (%s: line %d)\n\tCould not load '%s'.\n", strrchr(__FILE__, '/') + 1, __LINE__, filename);
        COLOR_DEFAULT;
    }
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        img_data.getSize().x, img_data.getSize().y,
        0,
        GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ShowHelpMarker(const char *desc)
{
    TextDisabled("(?)");
    if (IsItemHovered())
    {
        BeginTooltip();
        PushTextWrapPos(GetFontSize() * 35.0f);
        TextUnformatted(desc);
        PopTextWrapPos();
        EndTooltip();
    }
}

void AddFileLocation(const char *filename)
{
    char *ptr;
    ptr = realpath(filename, NULL);
    if (ptr == NULL) {
        size_t len = strlen(filename);
        ptr = (char *) malloc(len + 1);
        // copying len + 1 characters to get null pointer at the end as well
        memcpy(ptr, filename, len + 1);
    }
    PushTextWrapPos(0.0f);
    TextUnformatted(ptr);
    PopTextWrapPos();
    free(ptr);
}

void pushDisable()
{
    PushItemFlag(ImGuiItemFlags_Disabled, true);
    PushStyleVar(ImGuiStyleVar_Alpha, GetStyle().Alpha * 0.5f);
}

void popDisable()
{
    PopItemFlag();
    PopStyleVar();
}
