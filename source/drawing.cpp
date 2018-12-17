#include "drawing.h"
#include "globaldata.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "CImg.h"

using namespace cimg_library;

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
    ImGui::CreateContext();
    global.window.io = ImGui::GetIO(); (void)global.window.io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(global.window.window, global.window.gl_context);
    ImGui_ImplOpenGL2_Init();

    // Setup Style
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
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
    global.window.button_size = 28;
}

void initMovie(bool show_video_window)
{
    int i, j, n, c;
    float x, y, r, x1, y1, x2, y2;
    ImDrawList *draw_list;

    global.window.movie_window_width = global.window.video_window_width - 2 * global.window.margin;
    global.window.movie_window_height = global.window.video_window_height - 5 * global.window.margin - global.window.button_size - 20; // filenamr height
    
    global.movie_proportion_x = ((double) global.window.movie_window_width - 20) / global.SX;
    global.movie_proportion_y = ((double) global.window.movie_window_height - 20) / global.SY;

    ImGui::BeginChild("Test", ImVec2(global.window.movie_window_width, global.window.movie_window_height), true);
    draw_list = ImGui::GetWindowDrawList();
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
    ImGui::EndChild();
}

void initVideoWindow(bool* show_video_window)
{
    global.window.video_window_height = global.window.video_window_width 
        = global.Windowsize_y - 2 * global.window.margin;

    static bool play = true;
    ImGui::SetNextWindowPos(ImVec2(global.window.margin, global.window.margin));
    ImGui::SetNextWindowSize(ImVec2(global.window.video_window_width, global.window.video_window_height));
    ImGui::Begin("Video", show_video_window,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    AddFileLocation(global.moviefilename);
    initMovie(true);
    
    ImGui::Separator();
    if (ImGui::ImageButton((void*) (intptr_t)global.window.back_image, ImVec2(global.window.button_size, global.window.button_size)))
        global.current_frame = 0;
    ImGui::SameLine();

    if (ImGui::ImageButton((void*) (intptr_t)global.window.rewind_image, ImVec2(global.window.button_size, global.window.button_size)))
        {}
    ImGui::SameLine();

    if (play)
        ImGui::ImageButton((void*) (intptr_t)global.window.pause_image, ImVec2(global.window.button_size, global.window.button_size));
    else
        ImGui::ImageButton((void*) (intptr_t)global.window.play_image, ImVec2(global.window.button_size, global.window.button_size));
    if (ImGui::IsItemClicked())
        play = !play;
    ImGui::SameLine();

    if (ImGui::ImageButton((void*) (intptr_t)global.window.fastforward_image, ImVec2(global.window.button_size, global.window.button_size)))
        {}
    ImGui::SameLine();

    if (ImGui::ImageButton((void*) (intptr_t)global.window.next_image, ImVec2(global.window.button_size, global.window.button_size)))
        global.current_frame = global.N_frames - 1;
    ImGui::SameLine();

    ImGui::PushItemWidth(-50);
    ImGui::SliderInt("Frames", &global.current_frame, 0, global.N_frames - 1);
    ImGui::End();
    if (play)
        global.current_frame = (global.current_frame < global.N_frames - 1 ? global.current_frame + 1 : 0);
}

void initGraphWindow(bool* show)
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

    ImGui::ShowDemoWindow();
    global.window.graph_window_width = global.Windowsize_x - 3 * global.window.margin - global.window.video_window_width;
    global.window.graph_window_height = global.window.video_window_height / 2;

    poz_x = global.window.margin + global.window.video_window_width + global.window.margin;
    poz_y = global.window.margin;

    ImGui::SetNextWindowPos(ImVec2(poz_x, poz_y));
    ImGui::SetNextWindowSize(ImVec2(global.window.graph_window_width, global.window.graph_window_height));
    ImGui::Begin("Graph", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);
    // ImGui::SetNextWindowPos(ImVec2(2 * global.window.margin, global.window.margin));

    AddFileLocation(global.statfilename);

    x_size = global.window.graph_window_width - 2.5 * global.window.margin;
    y_size = global.window.graph_window_height - 12 * global.window.margin;
    ImGui::BeginChild("", ImVec2(x_size, y_size), true);
    draw_list = ImGui::GetWindowDrawList();

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
            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.window.graph_window_height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
        }
    }

    if (j == -1)
    {
        if (t_frame < t0)
        {
            j = 0;
            t_frame += 2;
            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.window.graph_window_height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
        }

        if (t_frame > tn)
        {
            j = global.N_stats - 1;
            t_frame -= 2;
            draw_list->AddLine(ImVec2(t_frame, 0), ImVec2(t_frame, poz_y + global.window.graph_window_height), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
        }
    }

    ImGui::EndChild();
    calculateCoordinatesOnGraph(j);
    ImGui::End();
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
        ImGui::TextColored(colors[6], "%s", x_text);
        ImGui::SameLine();
        free(x_text);
    }

    if (global.show_y)
    {
        y_text = (char*) malloc(100);
        snprintf(y_text, 100, "y = %2.8f\t", y_value);
        ImGui::TextColored(colors[7], "%s", y_text);
        ImGui::SameLine();
        free(y_text);
    }

    if (global.show_z)
    {
        z_text = (char*) malloc(100);
        snprintf(z_text, 100, "z = %f2.8\t", z_value);
        ImGui::TextColored(colors[8], "%s", z_text);
        ImGui::SameLine();
        free(z_text);
    }

}

void initSettingsWindow(bool* show)
{
    global.window.settings_window_width = global.window.graph_window_width;
    global.window.settings_window_height = global.Windowsize_y - 3 * global.window.margin - global.window.graph_window_height;
    ImGui::SetNextWindowPos(ImVec2(global.window.margin + global.window.video_window_width + global.window.margin, 2 * global.window.margin + global.window.graph_window_height));
    ImGui::SetNextWindowSize(ImVec2(global.window.settings_window_width, global.window.settings_window_height));
    ImGui::Begin("Settings", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_MenuBar);

    static bool load_movie = false;
    static bool load_stat = false;
    static bool save_movie = false;
    static bool save_graph = false;
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Load"))
            {
                ImGui::MenuItem("Movie file", "CTRL+M");
                ImGui::MenuItem("Statistics file", "CTRL+T");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Save"))
            {
                ImGui::MenuItem("Movie to avi");
                ImGui::MenuItem("Graph");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::BulletText("Double-click on title bar to collapse window.");
        ImGui::BulletText("CTRL+Click on a slider to input value as text.");
    }
    
    if (ImGui::CollapsingHeader("Movie"))
    {
        ImGui::Checkbox("Toggle trajectory", &global.trajectories_on);
        if (global.trajectories_on)
        {
            float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::PushButtonRepeat(true);
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { if (global.particles_tracked > 1) global.particles_tracked--; }
            ImGui::SameLine(0.0f, spacing);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { if (global.particles_tracked < global.N_objects/4) global.particles_tracked++; }
            ImGui::PopButtonRepeat();
            ImGui::SameLine();
            ImGui::Text("%d", global.particles_tracked);

            ImGui::ColorEdit3("Trajectory color", (float*)&global.traj_color);
            ImGui::SameLine(); 
            ShowHelpMarker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");
            ImGui::DragFloat("Trajectory width", &global.traj_width, 0.05f, 0.1f, 5.0f, "%.2f");
        }
        // if (ImGui::TreeNode("Zoom"))
        // {
        //     ImGui::TreePop();
        //     ImGui::Separator();
        // }
    }
    if (ImGui::CollapsingHeader("Graph"))
    {
        if (ImGui::TreeNode("Data shown"))
        {
            ImGui::Checkbox("X", &global.show_x);
            ImGui::Checkbox("Y", &global.show_y);
            ImGui::Checkbox("Z", &global.show_z);
            ImGui::TreePop();
            ImGui::Separator();
        }
    }
    
    ImGui::End();
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
        ImGui::NewFrame();

        initVideoWindow(NULL);
        initGraphWindow(NULL);
        initSettingsWindow(NULL);

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)global.window.io.DisplaySize.x, (int)global.window.io.DisplaySize.y);
        glClearColor(global.window.clear_color.x, global.window.clear_color.y, global.window.clear_color.z, global.window.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(global.window.window);

    }
    cleanup();
}

void cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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

void readImage(const char* imagePath, GLuint* renderedTexture)
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

    unsigned char* img = new unsigned char[width * height * 3];
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
    readImage("../img/play.png", &global.window.play_image);
    readImage("../img/pause.svg", &global.window.pause_image);
    readImage("../img/rewind.svg", &global.window.rewind_image);
    readImage("../img/fast-forward.svg", &global.window.fastforward_image);
    readImage("../img/next.svg", &global.window.next_image);
    readImage("../img/back.svg", &global.window.back_image);
}

void ShowHelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void AddFileLocation(const char* filename)
{
    char *ptr;
    ptr = realpath(global.statfilename, NULL);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(ptr);
    ImGui::PopTextWrapPos();
}
