#include "drawing.h"
#include "globaldata.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "imguifilesystem.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "CImg.h"

using namespace cimg_library;
using namespace ImGui;
using namespace ImGuiFs;

const ImVec4 colors[] = {
    ImVec4(1.0000, 0.0000, 0.0000, 1.0000), // 0
    ImVec4(0.7000, 0.7000, 0.7000, 1.0000), // 1
    ImVec4(0.7000, 0.7000, 0.7000, 1.0000), // 2
    ImVec4(0.0000, 0.0000, 0.0000, 1.0000), // 3
    ImVec4(0.8941, 0.1019, 0.1098, 1.0000), // 4
    ImVec4(0.4941, 0.6117, 0.9215, 1.0000), // 5
    ImVec4(0.3019, 0.6862, 0.2901, 1.0000), // 6
    ImVec4(0.8900, 0.6120 ,0.2160, 1.0000), // 7
    ImVec4(0.8500, 0.1290, 0.1250, 1.0000), // 8
};

void setupSDL()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void initWindow()
{
    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GetCurrentDisplayMode(0, &global.window.current);

    global.window.window = SDL_CreateWindow("Plot", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, global.Windowsize_x, global.Windowsize_y, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    global.window.gl_context = SDL_GL_CreateContext(global.window.window);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    CreateContext();
    global.window.io = GetIO(); (void)global.window.io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(global.window.window, global.window.gl_context);
    ImGui_ImplOpenGL2_Init();

    // Setup Style
    // StyleColorsDark();
    StyleColorsLight();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    global.window.clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    global.window.margin = 10;
    global.video.button_size = 28;

    global.video.height = global.video.width 
        = global.Windowsize_y - 2 * global.window.margin;
    global.video.play = true;

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
        ImGuiWindowFlags_NoResize/* | ImGuiWindowFlags_NoCollapse */| 
        ImGuiWindowFlags_NoMove);
    AddFileLocation(global.moviefilename);
    initMovie(true);
    
    Separator();
    if (ImageButton((void*) (intptr_t)global.video.back_image, ImVec2(global.video.button_size, global.video.button_size)))
        global.current_frame = 0;
    SameLine();

    if (ImageButton((void*) (intptr_t)global.video.rewind_image, ImVec2(global.video.button_size, global.video.button_size)))
        {}
    SameLine();

    if (global.video.play)
        ImageButton((void*) (intptr_t)global.video.pause_image, ImVec2(global.video.button_size, global.video.button_size));
    else
        ImageButton((void*) (intptr_t)global.video.play_image, ImVec2(global.video.button_size, global.video.button_size));
    if (IsItemClicked())
        global.video.play = !global.video.play;
    SameLine();

    if (ImageButton((void*) (intptr_t)global.video.fastforward_image, ImVec2(global.video.button_size, global.video.button_size)))
        {}
    SameLine();

    if (ImageButton((void*) (intptr_t)global.video.next_image, ImVec2(global.video.button_size, global.video.button_size)))
        global.current_frame = global.N_frames - 1;
    SameLine();

    PushItemWidth(-50);
    SliderInt("Frames", &global.current_frame, 0, global.N_frames - 1);
    End();
    if (global.video.play)
        global.current_frame = (global.current_frame < global.N_frames - 1 ? global.current_frame + 1 : 0);
}

void initGraphWindow(bool *show)
{
    int x_size, y_size;
    int poz_x, poz_y;
    int i, j;
    int t1, t2, t_max, t_frame, t0, tn;
    float x1, y1, z1;
    float x2, y2, z2;
    float x_max, y_max, z_max;
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
    // SetNextWindowPos(ImVec2(2 * global.window.margin, global.window.margin));

    AddFileLocation(global.statfilename);

    x_size = global.graph.width - 2.5 * global.window.margin;
    y_size = global.graph.height - 12 * global.window.margin;
    BeginChild("", ImVec2(x_size, y_size), true);
    draw_list = GetWindowDrawList();

    poz_x += 10;
    x_size -= 7;
    poz_y += 45;

    xc = ImColor(colors[6]);
    yc = ImColor(colors[7]);
    zc = ImColor(colors[8]);

    maxStats(&t_max, &x_max, &y_max, &z_max);

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
    dlg.chooseFileDialog(open_movie || open_stats);
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
    readImages();
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(global.window.window);
        NewFrame();

        initVideoWindow(NULL);
        initGraphWindow(NULL);
        initSettingsWindow(NULL);

        // Rendering
        Render();
        glViewport(0, 0, (int)global.window.io.DisplaySize.x, (int)global.window.io.DisplaySize.y);
        glClearColor(global.window.clear_color.x, global.window.clear_color.y, global.window.clear_color.z, global.window.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL2_RenderDrawData(GetDrawData());
        SDL_GL_SwapWindow(global.window.window);

    }
    cleanup();
}

void cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    DestroyContext();

    SDL_GL_DeleteContext(global.window.gl_context);
    SDL_DestroyWindow(global.window.window);
    SDL_Quit();
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

void readImage(const char *imagePath, GLuint *renderedTexture)
{  
    glGenTextures(1, renderedTexture);
    glBindTexture(GL_TEXTURE_2D, *renderedTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    CImg<unsigned char> image(imagePath);
    int width = image.width();
    int height = image.height();

    unsigned char *img = new unsigned char[width * height * 3];
    for (int i = 0; i < height; i++)
    {  
        for (int j = 0; j < width; j++)
        {  
            img[ 3 * (i * width + j) + 0 ] = (unsigned char)*(image.data(j, i, 0, 0));
            img[ 3 * (i * width + j) + 1 ] = (unsigned char)*(image.data(j, i, 0, 1));
            img[ 3 * (i * width + j) + 2 ] = (unsigned char)*(image.data(j, i, 0, 2));
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
              GL_UNSIGNED_BYTE, img);
    delete[] img;
}

void readImages()
{
    // https://www.flaticon.com/packs/music
    readImage("../img/play.svg", &global.video.play_image);
    readImage("../img/pause.svg", &global.video.pause_image);
    readImage("../img/rewind.svg", &global.video.rewind_image);
    readImage("../img/fast-forward.svg", &global.video.fastforward_image);
    readImage("../img/next.svg", &global.video.next_image);
    readImage("../img/back.svg", &global.video.back_image);
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
