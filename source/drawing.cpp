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

    global.window.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    global.window.margin = 10;
    global.video.button_size = 28;

    global.video.height = global.video.width 
        = global.Windowsize_y - 2 * global.window.margin;
    global.video.play = true;
    global.video.step = 1;

    global.movie.width = global.video.width - 2 * global.window.margin;
    global.movie.height = global.video.height - 5 * global.window.margin - global.video.button_size - 20; // filenamr height
    
    global.movie_proportion_x = ((double) global.movie.width - 20) / global.SX;
    global.movie_proportion_y = ((double) global.movie.height - 20) / global.SY;

    global.graph.width = global.Windowsize_x - 3 * global.window.margin - global.video.width;
    global.graph.height = global.video.height / 2;

    global.settings.width = global.graph.width;
    global.settings.height = global.Windowsize_y - 3 * global.window.margin - global.graph.height;

    global.settings.poz_x = global.window.margin + global.video.width + global.window.margin;
    global.settings.poz_y = 2 * global.window.margin + global.graph.height;

    return 1;
}

void initMovie(bool show_video_window)
{
    int i, j, n, c;
    float x, y, r, x1, y1, x2, y2;
    ImDrawList *draw_list;

    BeginChild("Test", ImVec2(global.movie.width, global.movie.height), true);
    draw_list = GetWindowDrawList();
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
        if (c == 2 || c == 3) 
        {
            draw_list->AddCircleFilled(ImVec2((int)x, (int)y), r, col32, 36);
            if (global.trajectories_on)
            {
                if (n < global.particles_tracked)
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
                            draw_list->AddLine(ImVec2((int)x1, (int)y1), ImVec2((int)x2, (int)y2), ImColor(global.traj_color), global.traj_width);
                        }
                    }
                }
            }
        }
        else draw_list->AddCircle(ImVec2((int)x, (int)y), r, col32, 36, 1);
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
        global.current_frame = (global.current_frame < global.N_frames - (int) global.video.step ? global.current_frame + global.video.step : 0);
}

void drawDecartesCoordinateSystem(ImDrawList *draw_list, ImVec2 poz, ImVec2 size, int x_max, ImVec2 y_lims)
{
    // x_lims = ImVec2(t_min, t_max)
    // y_lims = ImVec2(min, max)
    // y_lims.x = -1.5f;
    int i, j, t_value, step, axis, tick_poz;
    ImU32 black, gray;
    const unsigned int a = 7;
    const unsigned int x = poz.x,
        y = poz.y,
        y2 = poz.y + size.y;
    const unsigned int x0 = poz.x - 30,
        x01 = poz.x + size.x,
        y0 = (y_lims.x < 0 ? poz.y + size.y * y_lims.y / (y_lims.y - y_lims.x) : poz.y + size.y - 7);

    gray    = ImColor(colors[2]);
    black   = ImColor(colors[3]);

    // vertical axis
    draw_list->AddLine(ImVec2(x, y), ImVec2(x, y2), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x, y), ImVec2(x - a / 2, y + sqrt(3) * a / 2), black);
    draw_list->AddLine(ImVec2(x, y), ImVec2(x + a / 2, y + sqrt(3) * a / 2), black);
    draw_list->AddText(ImVec2(x - 9, y + 5), black, "N");

    // vertical tick lines
    step = x_max / 5 / 4;
    axis = (x01 - sqrt(3) * a / 2 - x0 - 30) / 20;
    // printf("%d\n", x_max);
    for (i = 0, tick_poz = poz.x + axis, j = 1; i < x_max; i += step, tick_poz += axis, j++) {
        draw_list->AddLine(ImVec2(tick_poz, y), ImVec2(tick_poz, y2), gray);
        if (j % 5 == 0) {
            draw_list->AddLine(ImVec2(tick_poz, y0 - 7), ImVec2(tick_poz, y0 + 7), black);
            draw_list->AddText(ImVec2(tick_poz - 15, y0 + 7), black, "1000");
        } else {
            draw_list->AddLine(ImVec2(tick_poz, y0 - 4), ImVec2(tick_poz, y0 + 4), black);
        }
    }

    // horizontal axis
    draw_list->AddLine(ImVec2(x0, y0), ImVec2(x01, y0), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 - a / 2), black);
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 + a / 2), black);
    draw_list->AddText(ImVec2(x01 - 20, y0 + a / 2), black, "t");

    draw_list->AddText(ImVec2(x0 + 2, y0), black, "0");
}

void initGraphWindow(bool *show)
{
    int x_size, y_size;
    int poz_x, poz_y;
    int i, j;
    int t1, t2, t_max, t_min, t_frame, t0, tn;
    float x1, y1, z1;
    float x2, y2, z2;
    float x_max, y_max, z_max, max;
    float x_min, y_min, z_min, min;
    ImU32 xc, yc, zc;
    ImDrawList *draw_list;

    ShowDemoWindow();
    poz_x = global.window.margin + global.video.width + global.window.margin;
    poz_y = global.window.margin;

    SetNextWindowPos(ImVec2(poz_x, poz_y));
    SetNextWindowSize(ImVec2(global.graph.width, global.graph.height));
    Begin("Graph", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove);

    AddFileLocation(global.statfilename);

    x_size = global.graph.width - 2.5 * global.window.margin;
    y_size = global.graph.height - 8.2 * global.window.margin;
    BeginChild("", ImVec2(x_size, y_size), true);
        draw_list = GetWindowDrawList();

        poz_x += 40;
        poz_y += 67;
        y_size -= 20;
        x_size -= 40;

        maxStats(&t_max, &x_max, &y_max, &z_max);
        minStats(&t_min, &x_min, &y_min, &z_min);

        min = x_min;
        if (min > y_min) min = y_min;
        if (min > z_min) min = z_min;

        max = x_max;
        if (max > y_max) max = y_max;
        if (max > z_max) max = z_max;
        
        drawDecartesCoordinateSystem(draw_list, ImVec2(poz_x, poz_y), ImVec2(x_size, y_size), t_max, ImVec2(min, max));

        y_size -= 7;

        xc = ImColor(colors[6]);
        yc = ImColor(colors[7]);
        zc = ImColor(colors[8]);


        x2 = global.stats[0].x;
        y2 = global.stats[0].y;
        z2 = global.stats[0].z;
        t2 = global.stats[0].time;

        generalTransformCoordinates(&t2, t_max, x_size, poz_x);
        generalTransformCoordinates(&x2, x_max, y_size, poz_y, true);
        generalTransformCoordinates(&y2, y_max, y_size, poz_y, true);
        generalTransformCoordinates(&z2, z_max, y_size, poz_y, true);

        t0 = t2;
        t_frame = global.current_frame * 100;
        generalTransformCoordinates(&t_frame, t_max, x_size, poz_x);
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


            generalTransformCoordinates(&t2, t_max, x_size, poz_x);
            generalTransformCoordinates(&x2, x_max, y_size, poz_y, true);
            generalTransformCoordinates(&y2, y_max, y_size, poz_y, true);
            generalTransformCoordinates(&z2, z_max, y_size, poz_y, true);

            if (global.show_x) draw_list->AddLine(ImVec2(t1, (int) x1), ImVec2(t2, (int) x2), xc, 0.5);
            if (global.show_y) draw_list->AddLine(ImVec2(t1, (int) y1), ImVec2(t2, (int) y2), yc, 0.5);
            if (global.show_z) draw_list->AddLine(ImVec2(t1, (int) z1), ImVec2(t2, (int) z2), zc, 0.5);

            if (t_frame >= t1 && t_frame <= t2)
            {
                j = i;  // t_frame value is between (i - 1) and i
                draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.graph.height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
            }
        }

        if (j == -1)
        {
            if (t_frame < t0)
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
    j = 1;
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

    if (i != 0) {
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

    if (t1 == t2) 
    {
        x_value = x1;
        y_value = y1;
        z_value = z1;
    } else {
        x_value = (1 - t1 * x2 + t2 * x1 - t_frame * (x1 - x2)) / (t2 - t1);
        y_value = (1 - t1 * y2 + t2 * y1 - t_frame * (y1 - y2)) / (t2 - t1);
        z_value = (1 - t1 * z2 + t2 * z1 - t_frame * (z1 - z2)) / (t2 - t1);
    }

    if (global.show_x)
    {
        x_text = (char*) malloc(100);
        snprintf(x_text, 100, "x = %2.8f\t", x_value);
        TextColored(colors[6], "%s", x_text);
        SameLine();
        free(x_text);
    }

    if (global.show_y)
    {
        y_text = (char*) malloc(100);
        snprintf(y_text, 100, "y = %2.8f\t", y_value);
        TextColored(colors[7], "%s", y_text);
        SameLine();
        free(y_text);
    }

    if (global.show_z)
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
    int length;
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
    dlg.chooseFileDialog(open_movie || open_stats, "..", ".mvi;.txt");
    if ((length = strlen(dlg.getChosenPath())) > 0)
    {
        if (strncmp(dlg.getChosenPath(), global.moviefilename, (length < global.length ? length : global.length)) != 0)
        {
            if (length > global.length)
            {
                global.length = length;
                global.moviefilename = (char *) realloc(global.moviefilename, length);
                global.statfilename = (char *) realloc(global.statfilename, length);
            }
            if (global.settings.open)
            {
                strncpy(global.statfilename, dlg.getChosenPath(), length);
                global.statfilename[length] = '\0';
                read_statisticsfile_data();
                open_stats = false;
            } else {
                strncpy(global.moviefilename, dlg.getChosenPath(), length);
                global.moviefilename[length] = '\0';
                read_moviefile_data();
                open_movie = false;
            }
            global.current_frame = 0;
        }
        global.video.play = true;
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
        Checkbox("Toggle trajectory", &global.trajectories_on);
        if (global.trajectories_on)
        {
            float spacing = GetStyle().ItemInnerSpacing.x;
            PushButtonRepeat(true);
            if (ArrowButton("##left", ImGuiDir_Left)) { if (global.particles_tracked > 1) global.particles_tracked--; }
            SameLine(0.0f, spacing);
            if (ArrowButton("##right", ImGuiDir_Right)) { if (global.particles_tracked < global.N_objects/4) global.particles_tracked++; }
            PopButtonRepeat();
            SameLine();
            Text("%d", global.particles_tracked);

            ColorEdit3("Trajectory color", (float*)&global.traj_color);
            SameLine(); 
            ShowHelpMarker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");
            DragFloat("Trajectory width", &global.traj_width, 0.05f, 0.1f, 5.0f, "%.2f");
            ShowHelpMarker("Click and drag to change the value");
        }
        // if (TreeNode("Zoom"))
        // {
        //     TreePop();
        //     Separator();
        // }
    }
    if (CollapsingHeader("Graph"))
    {
        if (TreeNode("Data shown"))
        {
            Checkbox("X", &global.show_x);
            Checkbox("Y", &global.show_y);
            Checkbox("Z", &global.show_z);
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

void maxStats(int *t_max, float *x_max, float *y_max, float *z_max)
{
    int i;

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

void minStats(int *t_min, float *x_min, float *y_min, float *z_min)
{
    int i;

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

void generalTransformCoordinates(int *x, int x_max, int x_size, int distance_from_origin, bool flip)
{
    if (flip) *x = x_max - *x;
    *x = *x * x_size / x_max + distance_from_origin;
}

void transformMovieCoordinates(float *x, float *y)
{
    *x = *x - global.zoom_x0;
    *y = *y - global.zoom_y0;
    *y = global.SY - *y;

    *x = *x / global.zoom_deltax * global.movie_proportion_x * global.SX + 3 * global.window.margin;
    *y = *y / global.zoom_deltay * global.movie_proportion_y * global.SY + 11 * global.window.margin - 90;
}

//transforms from simulation unit distances to Opengl units
void transformDistance(float *r)
{
    *r = *r * global.movie_proportion_x;
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
    PushTextWrapPos(0.0f);
    TextUnformatted(ptr);
    PopTextWrapPos();
}
