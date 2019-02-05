#include "drawing.h"
#include "globaldata.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "imguifilesystem.h"

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
        return 0;

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
        for (i = 0; i < global.N_objects; i++)
        {
            x = global.objects[global.current_frame][i].x;
            y = global.objects[global.current_frame][i].y;
            r = global.objects[global.current_frame][i].R;
            transformMovieCoordinates(&x, &y);
            transformDistance(&r);
            c = global.objects[global.current_frame][i].color;
            if (c < 0 || c > 9) c = 0;
            const ImU32 col32 = ImColor(colors[c]);
            if (global.objects[global.current_frame][i].R == global.particle_r) 
            {
                if (global.movie.show_particles) {
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
                    if (global.movie.show_just_center_pinningsites)
                        draw_list->AddCircle(ImVec2(x, y), r / 3, col32, 36, 1);
                    else
                        draw_list->AddCircle(ImVec2(x, y), r, col32, 36, 1);
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
    readImage(&play_image, "../img/play.png");
    readImage(&pause_image, "../img/pause.png");
    readImage(&back_image, "../img/back.png");
    readImage(&next_image, "../img/next.png");
    readImage(&rewind_image, "../img/rewind.png");
    readImage(&fastforward_image, "../img/fast-forward.png");
    
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
    axis = *size_y / 4 / 4;
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

void initGraphWindow(bool *show)
{
    unsigned int size_x, size_y;
    unsigned int poz_x, poz_y;
    unsigned int i, y0;
    int j;
    unsigned int t1, t2, t_max, t_min, t_frame, t0, tn;
    float x1, y1, z1;
    float x2, y2, z2;
    float x_max, y_max, z_max, max;
    float x_min, y_min, z_min, min;
    ImU32 xc, yc, zc;
    ImDrawList *draw_list;

    poz_x = global.window.margin;
    poz_y = global.window.margin + global.video.height + global.window.margin;

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

            poz_x += 60;
            poz_y += 52;
            size_y -= 16;
            size_x -= 60;

            maxStats(&t_max, &x_max, &y_max, &z_max);
            minStats(&t_min, &x_min, &y_min, &z_min);

            min = HUGE_VAL_F32;
            if (global.graph.show_x && min > x_min) min = x_min;
            if (global.graph.show_y && min > y_min) min = y_min;
            if ((global.graph.show_z && min > z_min) || min == HUGE_VAL_F32) min = z_min;

            max = (-1) * HUGE_VAL_F32;
            if (global.graph.show_x && max < x_max) max = x_max;
            if (global.graph.show_y && max < y_max) max = y_max;
            if ((global.graph.show_z && max < z_max) || max == (-1) * HUGE_VAL_F32) max = z_max;
            
            drawDecartesCoordinateSystem(draw_list, &poz_x, &poz_y, &size_x, &size_y, t_max, ImVec2(min, max), &y0);

            max = (min < 0 ? max - min : max);

            xc = ImColor(colors[6]);
            yc = ImColor(colors[7]);
            zc = ImColor(colors[8]);

            x2 = global.stats[0].x;
            y2 = global.stats[0].y;
            z2 = global.stats[0].z;
            t2 = global.stats[0].time;

            generalTransformCoordinates(&t2, t_max, size_x, poz_x);
            generalTransformCoordinates(&x2, max, size_y, poz_y, true);
            generalTransformCoordinates(&y2, max, size_y, poz_y, true);
            generalTransformCoordinates(&z2, max, size_y, poz_y, true);

            // it has to shift the curves with the distance of negative values (size_y - y0)
            x2 -= size_y - y0;
            y2 -= size_y - y0;
            z2 -= size_y - y0;

            t0 = t2;
            t_frame = global.current_frame * 100;
            generalTransformCoordinates(&t_frame, t_max, size_x, poz_x);
            j = -1;
            for (i = 1; i < global.N_stats; i++)
            {
                x1 = x2;
                y1 = y2;
                z1 = z2;
                tn = t1 = t2;

                x2 = global.stats[i].x;
                y2 = global.stats[i].y;
                z2 = global.stats[i].z;
                t2 = global.stats[i].time;

                generalTransformCoordinates(&t2, t_max, size_x, poz_x);
                generalTransformCoordinates(&x2, max, size_y, poz_y, true);
                generalTransformCoordinates(&y2, max, size_y, poz_y, true);
                generalTransformCoordinates(&z2, max, size_y, poz_y, true);

                x2 -= size_y - y0;
                y2 -= size_y - y0;
                z2 -= size_y - y0;

                if (global.graph.show_x) draw_list->AddLine(ImVec2(t1, (int) x1), ImVec2(t2, (int) x2), xc, 0.5);
                if (global.graph.show_y) draw_list->AddLine(ImVec2(t1, (int) y1), ImVec2(t2, (int) y2), yc, 0.5);
                if (global.graph.show_z) draw_list->AddLine(ImVec2(t1, (int) z1), ImVec2(t2, (int) z2), zc, 0.5);

                if (global.graph.show_x || global.graph.show_y || global.graph.show_z)
                    if (t_frame > t1 && t_frame <= t2)
                    {
                        j = i;  // t_frame value is between (i - 1) and i
                        draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                    }
            }
            if (global.graph.show_x || global.graph.show_y || global.graph.show_z)
                if (j == -1)
                {
                    if (t_frame <= t0)
                    {
                        j = 0;
                        t_frame += 2;
                        draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                    }

                    if (t_frame > tn)
                    {
                        j = global.N_stats - 1;
                        t_frame -= 2;
                        draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                    }
                }

        EndChild();
        calculateCoordinatesOnGraph(j);
    End();
}

void calculateCoordinatesOnGraph(int i)
{
    float x1, y1, z1, x2, y2, z2;
    float x_value, y_value, z_value;
    int t1, t2, t_frame;
    char *x_text, *y_text, *z_text;

    t_frame = global.current_frame * 100;
    x2 = global.stats[i].x;
    y2 = global.stats[i].y;
    z2 = global.stats[i].z;
    t2 = global.stats[i].time;

    if (i != 0)
    {
        x1 = global.stats[i - 1].x;
        y1 = global.stats[i - 1].y;
        z1 = global.stats[i - 1].z;
        t1 = global.stats[i - 1].time;
    } else {
        x1 = x2;
        y1 = y2;
        z1 = z2;
        t1 = t2;
    }
    
    if (t1 == t2 || t1 == t_frame) 
    {
        x_value = x1;
        y_value = y1;
        z_value = z1;
    } else {
        x_value = ((t_frame - t1) * (x2 - x1)) / (t2 - t1) + x1;
        y_value = ((t_frame - t1) * (y2 - y1)) / (t2 - t1) + y1;
        z_value = ((t_frame - t1) * (z2 - z1)) / (t2 - t1) + z1;
    }

    if (global.graph.show_x)
    {
        x_text = (char*) malloc(100);
        snprintf(x_text, 100, "x = %2.8f\t", x_value);
        TextColored(colors[6], "%s", x_text);
        SameLine();
        free(x_text);
    }

    if (global.graph.show_y)
    {
        y_text = (char*) malloc(100);
        snprintf(y_text, 100, "y = %2.8f\t", y_value);
        TextColored(colors[7], "%s", y_text);
        SameLine();
        free(y_text);
    }

    if (global.graph.show_z)
    {
        z_text = (char*) malloc(100);
        snprintf(z_text, 100, "z = %f2.8\t", z_value);
        TextColored(colors[8], "%s", z_text);
        SameLine();
        free(z_text);
    }

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
                read_statisticsfile_data();
                open_stats = false;
                global.settings.open = -1;
            } else if (global.settings.open == 0)
            {
                strncpy(global.moviefilename, dlg.getChosenPath(), length);
                global.moviefilename[length] = '\0';
                read_moviefile_data();
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
    
    if (CollapsingHeader("Movie"))
    {
        Text("Pinningsites");
        Checkbox("Show pinningsites", &global.movie.show_pinningsites);
        Checkbox("Show pinningsite grid lines", &global.movie.show_grid_lines);
        if (global.movie.show_grid_lines) {
            ColorEdit3("Grid line color", (float*)&global.movie.grid_color);
            DragFloat("Grid line width", &global.movie.grid_line_width, 0.05f, 0.1f, 5.0f, "%.2f");
        }
        Checkbox("Decreese pinningsite radius", &global.movie.show_just_center_pinningsites);
        Separator();

        Text("Particles");
        Checkbox("Show particles", &global.movie.show_particles);
        if (global.movie.show_particles) {
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
        }
        Separator();

        if (TreeNode("Zoom"))
        {
            TreePop();
            Separator();
        }
    }
    if (CollapsingHeader("Graph"))
    {
        if (TreeNode("Data shown"))
        {
            Checkbox("X", &global.graph.show_x);
            Checkbox("Y", &global.graph.show_y);
            Checkbox("Z", &global.graph.show_z);
            TreePop();
            Separator();
        }
    }
    
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

void maxStats(unsigned int *t_max, float *x_max, float *y_max, float *z_max)
{
    unsigned int i;

    *t_max = global.stats[0].time;
    *x_max = global.stats[0].x;
    *y_max = global.stats[0].y;
    *z_max = global.stats[0].z;
    for (i = 1; i < global.N_stats; i++)
    {
        if (global.stats[i].time > *t_max) *t_max = global.stats[i].time;
        if (global.stats[i].x > *x_max) *x_max = global.stats[i].x;
        if (global.stats[i].y > *y_max) *y_max = global.stats[i].y;
        if (global.stats[i].z > *z_max) *z_max = global.stats[i].z;
    }
}

void minStats(unsigned int *t_min, float *x_min, float *y_min, float *z_min)
{
    unsigned int i;

    *t_min = global.stats[0].time;
    *x_min = global.stats[0].x;
    *y_min = global.stats[0].y;
    *z_min = global.stats[0].z;
    for (i = 1; i < global.N_stats; i++)
    {
        if (global.stats[i].time < *t_min) *t_min = global.stats[i].time;
        if (global.stats[i].x < *x_min) *x_min = global.stats[i].x;
        if (global.stats[i].y < *y_min) *y_min = global.stats[i].y;
        if (global.stats[i].z < *z_min) *z_min = global.stats[i].z;
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
        printf("Could not load '%s'.", filename);
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
        ptr = (char *) malloc(255);
        snprintf(ptr, 255, "Could not open file! %s", filename);
    }
    PushTextWrapPos(0.0f);
    TextUnformatted(ptr);
    PopTextWrapPos();
    free(ptr);
}
