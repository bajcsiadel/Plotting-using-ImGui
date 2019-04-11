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
#include <GLFW/glfw3.h> // GLuint
#include <string>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iostream>

#ifdef OPENCV
    #include <opencv2/opencv.hpp>

    cv::Mat make_frame(int current_frame, int width, int height, int padding)
    {
        unsigned int i, n, c;
        int j;
        double x, y, r, x1, y1, x2, y2;
        cv::Mat frame(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
        ImVec2 draw_pos   = ImVec2(padding, padding),
            proportion = ImVec2((width - 2 * padding) / global.movie.zoom.width, (height - 2 * padding) / global.movie.zoom.height);
        
        // if (global.movie.show_grid_lines)
        //     draw_grid(draw_list);

        n = 0;
        if (global.objects != NULL)
        {
            for (i = 0; i < global.N_objects; i++)
            {

                x = global.objects[current_frame][i].x;
                y = global.objects[current_frame][i].y;

                if (!(x >= global.movie.zoom.corners[0].x) || !(x <= global.movie.zoom.corners[1].x) ||
                    !(y >= global.movie.zoom.corners[0].y) || !(y <= global.movie.zoom.corners[1].y))
                    // if the current particle/pinningsite is not in the zoomed area, then skip it and go to the next element
                    continue;

                r = global.objects[current_frame][i].R;
            
                transform_movie_coordinates(&x, &y, draw_pos, proportion);
                transform_distance(&r);
                c = global.objects[current_frame][i].color;
                if (c < 0 || c > 9) c = 0;
                cv::Scalar col32 = cv::Scalar(colors[c].z * 255, colors[c].y * 255, colors[c].x * 255);
                if (global.objects[current_frame][i].R == global.particle_r) 
                {
                    if (global.movie.show_particles)
                    {
                        if (global.movie.monocrome_particles) col32 = cv::Scalar(global.movie.particle_color.z * 255, global.movie.particle_color.y * 255, global.movie.particle_color.x * 255);
                        circle(frame, cv::Point((int)x, (int)y), r, cv::Scalar(colors[c].x, colors[c].y, colors[c].z), -1, CV_AA);
                        if (global.movie.trajectories_on)
                        {
                            if (n < global.movie.particles_tracked)
                            {
                                n ++;
                                for (j = 0; j < current_frame - 1; j++)
                                {
                                    x1 = global.objects[j][i].x;
                                    y1 = global.objects[j][i].y;

                                    x2 = global.objects[j + 1][i].x;
                                    y2 = global.objects[j + 1][i].y;
                                    if ((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) < 2.0)
                                    {
                                        transform_movie_coordinates(&x1, &y1, draw_pos, proportion);
                                        transform_movie_coordinates(&x2, &y2, draw_pos, proportion);
                                        line(frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(global.movie.traj_color.z * 255, global.movie.traj_color.y * 255, global.movie.traj_color.x * 255), global.movie.traj_width, CV_AA);
                                    }
                                }
                            }
                        }
                    }
                }
                else 
                    if (global.movie.show_pinningsites)
                    {
                        if (global.movie.monocrome_pinningsites) col32 = cv::Scalar(global.movie.particle_color.z * 255, global.movie.particle_color.y * 255, global.movie.particle_color.x * 255);
                        if (global.movie.show_just_center_pinningsites)
                            circle(frame, cv::Point((int)x, (int)y), r / 3, col32, 1, CV_AA);
                        else
                            circle(frame, cv::Point((int)x, (int)y), r, col32, 1, CV_AA);
                    }
            }
        }
        return frame;
    }

    void make_video(const char *videoname, int from, int to, int width, int height)
    {
        cv::VideoWriter video(videoname, CV_FOURCC('M','J','P','G'), 40, cv::Size(width, height));
        for (int i = from; i <= to; i++)
        {
            cv::Mat frame = make_frame(i, width, height, global.save.padding);
            video.write(frame);
        }
        video.release();
    }
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int setup_GLFW()
{
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 0;
    return 1;
}

int init_window()
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
    // glfwSetCharCallback(global.window.window, ImGui_ImplGlfw_CharCallback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO io = ImGui::GetIO(); (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(global.window.window, true);
    ImGui_ImplOpenGL2_Init();

    // Setup Style
    ImGui::StyleColorsLight();

    // window data
    global.window.background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    global.window.margin = 10;

    // video window data
    global.video.button_size = 28;

    global.video.height = global.video.width
        = global.Windowsize_y * 2 / 3 - 2 * global.window.margin;
    global.video.play = true;
    global.video.step = 1;

    set_video_buttons_location();

    // movie window data
    global.movie.width = global.video.width - 2 * global.window.margin;
    global.movie.height = global.video.height - 5 * global.window.margin - global.video.button_size - ceil((double) strlen(global.moviefilename) * 6 / (double) (global.movie.width - 150)) * 13.5 - 2; // filename height

    global.movie.draw_x = global.movie.poz_x = 24;
    global.movie.draw_y = global.movie.poz_y = 56;

    if ((double) global.movie.width / (double) global.movie.height < abs(global.SX / global.SY))
        // decresing height is needed so with this (global.movie.width * global.SY) / global.SX the rate would be equal, therefore this
        // value is lower than the given height
    {
        global.movie.draw_y += (global.movie.height - ((double) global.movie.width * global.SY) / global.SX) / 2;
        // padding
        global.movie.draw_width = -10;
    }
    else
        if ((double) global.movie.width / (double) global.movie.height > abs(global.SX / global.SY))
        // decresing width is needed so with this (global.movie.height * global.SX) / global.SY the rate would be equal, therefore this
        // value is lower than the given width
        {
            global.movie.draw_x += (global.movie.width - ((double) global.movie.height * global.SX) / global.SY) / 2;
            // padding 
            global.movie.draw_height = -10;
        }

    global.movie.draw_width  += global.movie.width  - 2 * (global.movie.draw_x - global.movie.poz_x);
    global.movie.draw_height += global.movie.height - 2 * (global.movie.draw_y - global.movie.poz_y);
    
    reset_zoom();

    global.movie.trajectories_on = false;
    global.movie.particles_tracked = 5;
    global.movie.traj_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);
    global.movie.traj_width = 0.5;

    global.movie.grid_line_width = 0.5;
    global.movie.show_grid_lines = false;
    global.movie.grid_color = colors[1];

    global.movie.monocrome_particles = true;
    global.movie.particle_color = ImVec4(0.0000, 0.0000, 0.0000, 1.0000);

    global.movie.monocrome_pinningsites = false;
    global.movie.pinningsite_color = ImVec4(1.0000, 0.0000, 0.0000, 1.0000);

    global.movie.show_particles = true;
    global.movie.show_pinningsites = true;

    // graph window data
    global.graph.width = global.video.width;
    global.graph.height = global.video.height / 2;

    // settings window data
    global.settings.width = global.Windowsize_x - global.video.width - 3 * global.window.margin;
    global.settings.height = global.Windowsize_y - 2 * global.window.margin;

    global.settings.poz_x = global.window.margin + global.video.width + global.window.margin;
    global.settings.poz_y = global.window.margin;

    global.settings.open = -1;

    global.save.current = global.save.from = 0;
    global.save.to = global.N_frames;
    global.save.started = false;
    global.save.padding = 10;

    global.save.filename_length = 255;
    global.save.filename = (char *) malloc(global.save.filename_length);
    snprintf(global.save.filename, global.save.filename_length, "../vedios/test.avi");

    return 1;
}

void set_video_buttons_location()
{
    global.path_length = 255;
    global.path = (char *) malloc(global.path_length);
    get_relative_path_to_project_root(global.path, global.path_length);

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

void reset_zoom()
{
    global.movie.zoom.i = 0;

    global.movie.zoom.corners[0] = ImVec2(0.0, 0.0);
    global.movie.zoom.corners[1] = ImVec2(global.movie.draw_width, global.movie.draw_height);

    global.movie.zoom.width  = (global.movie.zoom.corners[1].x - global.movie.zoom.corners[0].x) * global.SX / global.movie.draw_width;
    global.movie.zoom.height = (global.movie.zoom.corners[1].y - global.movie.zoom.corners[0].y) * global.SY / global.movie.draw_height;

    global.movie.proportion_x = (double) global.movie.draw_width  / global.movie.zoom.width;
    global.movie.proportion_y = (double) global.movie.draw_height / global.movie.zoom.height;
}

void init_movie(bool show_video_window)
{
    unsigned int i, n, c;
    int j;
    double x, y, r, x1, y1, x2, y2;
    ImDrawList *draw_list;
    ImVec2 draw_pos   = ImVec2(global.movie.draw_x, global.movie.draw_y),
           proportion = ImVec2(global.movie.proportion_x, global.movie.proportion_y);

    ImGui::BeginChild("##movieChild", ImVec2(global.movie.width, global.movie.height), true);
        global.movie.draw_list = draw_list = ImGui::GetWindowDrawList();
        // global.movie.draw_list->AddRect(ImVec2(global.movie.draw_x, global.movie.draw_y), ImVec2(global.movie.draw_x + global.movie.draw_width, global.movie.draw_y + global.movie.draw_height), (ImU32) ImColor(colors[0]));

        if (global.movie.show_grid_lines)
            draw_grid(draw_list);

        n = 0;
        if (global.objects != NULL)
        {
            for (i = 0; i < global.N_objects; i++)
            {
                x = global.objects[global.current_frame][i].x;
                y = global.objects[global.current_frame][i].y;

                if (!(x >= global.movie.zoom.corners[0].x) || !(x <= global.movie.zoom.corners[1].x) ||
                    !(y >= global.movie.zoom.corners[0].y) || !(y <= global.movie.zoom.corners[1].y))
                    // if the current particle/pinningsite is not in the zoomed area, then skip it and go to the next element
                    continue;

                r = global.objects[global.current_frame][i].R;
            
                transform_movie_coordinates(&x, &y, draw_pos, proportion);
                transform_distance(&r);
                c = global.objects[global.current_frame][i].color;
                if (c < 0 || c > 9) c = 0;
                ImU32 col32 = ImColor(colors[c]);
                if (global.objects[global.current_frame][i].R == global.particle_r) 
                {
                    if (global.movie.show_particles)
                    {
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
                                        transform_movie_coordinates(&x1, &y1, draw_pos, proportion);
                                        transform_movie_coordinates(&x2, &y2, draw_pos, proportion);
                                        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImColor(global.movie.traj_color), global.movie.traj_width);
                                    }
                                }
                            }
                        }
                    }
                }
                else 
                    if (global.movie.show_pinningsites)
                    {
                        if (global.movie.monocrome_pinningsites) col32 = ImColor(global.movie.pinningsite_color);
                        if (global.movie.show_just_center_pinningsites)
                            draw_list->AddCircle(ImVec2(x, y), r / 3, col32, 36, 1);
                        else
                            draw_list->AddCircle(ImVec2(x, y), r, col32, 36, 1);
                    }
            }
        }
        zoom();
    ImGui::EndChild();
}

void init_video_window(bool *show_video_window)
{
    ImGui::SetNextWindowPos(ImVec2(global.window.margin, global.window.margin));
    ImGui::SetNextWindowSize(ImVec2(global.video.width, global.video.height));
    ImGui::Begin("Video", show_video_window,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove);
        // add_file_location(global.moviefilename);
        ImGui::Text("%f x %f", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        init_movie(true);
        
        GLuint play_image, pause_image, back_image, next_image, rewind_image, fastforward_image;
        // https://www.flaticon.com/packs/music
        read_image(&play_image, global.video.play_img_location);
        read_image(&pause_image, global.video.pause_img_location);
        read_image(&back_image, global.video.back_img_location);
        read_image(&next_image, global.video.next_img_location);
        read_image(&rewind_image, global.video.rewind_img_location);
        read_image(&fastforward_image, global.video.fastforward_img_location);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0, 1.0, 1.0, 1.0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, global.window.background_color);

        ImGui::Separator();
        ImGui::ImageButton((void *) (intptr_t) back_image, ImVec2(global.video.button_size, global.video.button_size));
        if (ImGui::IsItemClicked())
            global.current_frame = 0;
        ImGui::SameLine();

        ImGui::ImageButton((void *) (intptr_t) rewind_image, ImVec2(global.video.button_size, global.video.button_size));
        if (ImGui::IsItemClicked())
            global.video.step = (global.video.step != 1 ? global.video.step / 2 : global.video.step);
        ImGui::SameLine();

        if (global.video.play) 
            ImGui::ImageButton((void *) (intptr_t) pause_image, ImVec2(global.video.button_size, global.video.button_size));
        else
            ImGui::ImageButton((void *) (intptr_t) play_image, ImVec2(global.video.button_size, global.video.button_size));
        if (ImGui::IsItemClicked())
            global.video.play = !global.video.play;
        ImGui::SameLine();

        ImGui::ImageButton((void *) (intptr_t) fastforward_image, ImVec2(global.video.button_size, global.video.button_size));
        if (ImGui::IsItemClicked())
            global.video.step = (global.video.step < 128 ? global.video.step * 2 : global.video.step);
        ImGui::SameLine();

        ImGui::ImageButton((void *) (intptr_t) next_image, ImVec2(global.video.button_size, global.video.button_size));
        if (ImGui::IsItemClicked())
            global.current_frame = global.N_frames - 1;
        ImGui::SameLine();
        ImGui::PopStyleColor(2);

        ImGui::PushItemWidth(-50);
        ImGui::SliderInt("Frames", &global.current_frame, 0, global.N_frames - 1);
    ImGui::End();
    if (global.video.play)
        global.current_frame = (global.current_frame < (int) global.N_frames - (int) global.video.step ? global.current_frame + global.video.step : 0);
}

void draw_grid(ImDrawList *draw_list)
{
    unsigned int N_rows, N_columns;
    unsigned int i;
    double dx, dy;
    ImU32 color;
    ImVec2 draw_pos   = ImVec2(global.movie.draw_x, global.movie.draw_y),
           proportion = ImVec2(global.movie.proportion_x, global.movie.proportion_y);

    N_columns = (int) (global.SX / (2 * global.pinningsite_r)) + 1;
    N_rows = (int) (global.SY / (2 * global.pinningsite_r)) + 1;

    dx = global.SX / N_columns;
    dy = global.SY / N_rows;

    color = ImColor(global.movie.grid_color);

    for (i = 1; i < N_rows; i++)
    {
        double y0 = i * dy, y1 = i * dy;
        double x0 = 0.0, x1 = global.SX;

        transform_movie_coordinates(&x0, &y0, draw_pos, proportion);
        transform_movie_coordinates(&x1, &y1, draw_pos, proportion);

        draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, global.movie.grid_line_width);
    }

    for (i = 1; i < N_columns; i++)
    {
        double x0 = i * dx, x1 = i * dx;
        double y0 = 0.0, y1 = global.SY;

        transform_movie_coordinates(&x0, &y0, draw_pos, proportion);
        transform_movie_coordinates(&x1, &y1, draw_pos, proportion);

        draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), color, global.movie.grid_line_width);
    }
}

int calculate_number_length(double x)
{
    int n, xi;
    n = 0;
    do {
        n++;
        x /= 10;
        xi = (int) x;
    } while (xi != 0);
    return n;
}

void draw_descartes_coordinate_system(ImDrawList *draw_list, 
    unsigned int *poz_x, unsigned int *poz_y, 
    unsigned int *size_x, unsigned int *size_y, 
    unsigned int x_max, ImVec2 y_lims, unsigned int *origins_y_pozition)
{
    // y_lims = ImVec2(min, max)
    unsigned int j, t_value, t_step, axis, tick_poz, last_big_tick_mark;
    double range, y_value, y_step, min;
    ImU32 black, gray;
    const unsigned int a = 7, n = calculate_number_length(y_lims.y) - 1; // length of the arrow
    const unsigned int x = *poz_x + n * 5,
        y = *poz_y,
        y2 = *poz_y + *size_y;
    const unsigned int x0 = *poz_x - 50,
        x01 = *poz_x + *size_x,
        y0 = (y_lims.x < 0 ? *poz_y + *size_y * y_lims.y / (y_lims.y - y_lims.x) : *poz_y + *size_y) - 2 * a;

    *size_x -= n * 5;
    gray    = ImColor(colors[2]);
    black   = ImColor(colors[3]);

    // place thicks on vertical axis
    range = y_lims.y - (y_lims.x  < 0.0 ? y_lims.x : 0.0f);
    y_step = range / 4 / 4;
    *size_y = *size_y - sqrt(3) * a / 2;
    axis = *size_y / 4 / 4;
    *poz_y += sqrt(3) * a / 2 + 2;
    min = (y_lims.x  < 0.0 ? y_lims.x : 0.0f);
    min += min / 100;
    for (y_value = y_lims.y, tick_poz = *poz_y, j = 0; y_value >= min; y_value -= y_step, tick_poz += axis, j++)
    {
        draw_list->AddLine(ImVec2(x, tick_poz), ImVec2(x01 * 2, tick_poz), gray);
        if (abs((int) tick_poz - (int) y0) > 5 && (j % 4 == 0  || y_value - y_step < min))
        {
            char *number = new char[10];
            size_t len = snprintf(number, 9, "%.3f", y_value);

            draw_list->AddLine(ImVec2(x - 7, tick_poz), ImVec2(x + 7, tick_poz), black);
            draw_list->AddText(ImVec2(x - len * 8, tick_poz - 7), black, number);
            last_big_tick_mark = tick_poz;
            delete[] number;
        }
        else
            draw_list->AddLine(ImVec2(x - 4, tick_poz), ImVec2(x + 4, tick_poz), black);
        *size_y = tick_poz - *poz_y;
    }

    // vertical axis
    draw_list->AddLine(ImVec2(x, y), ImVec2(x, y2), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x, y), ImVec2(x - a / 2, y + sqrt(3) * a / 2), black);
    draw_list->AddLine(ImVec2(x, y), ImVec2(x + a / 2, y + sqrt(3) * a / 2), black);

    // ticks on horisontal axis
    t_step = x_max / 5 / 4;
    axis = (x01 - sqrt(3) * a / 2 - x0 - 50) / 20;
    for (t_value = t_step, tick_poz = *poz_x + axis, j = 1; t_value <= x_max; t_value += t_step, tick_poz += axis, j++)
    {
        draw_list->AddLine(ImVec2(tick_poz, 0), ImVec2(tick_poz, 2 * y2), gray);
        if (j % 5 == 0)
        {
            char *number = new char[10];
            size_t len = snprintf(number, 9, "%d", t_value);

            draw_list->AddLine(ImVec2(tick_poz, y0 - 7), ImVec2(tick_poz, y0 + 7), black);
            draw_list->AddText(ImVec2(tick_poz - len * 4, y0 + 7), black, number);
            *size_x = tick_poz - *poz_x;

            delete[] number;
        }
        else
            draw_list->AddLine(ImVec2(tick_poz, y0 - 4), ImVec2(tick_poz, y0 + 4), black);
    }

    // horizontal axis
    draw_list->AddLine(ImVec2(x0, y0), ImVec2(x01, y0), black);
    // arrow at it's end
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 - a / 2), black);
    draw_list->AddLine(ImVec2(x01, y0), ImVec2(x01 - sqrt(3) * a / 2, y0 + a / 2), black);

    if (abs((int) last_big_tick_mark - (int) y0) > 12) draw_list->AddText(ImVec2(x - 8, y0), black, "0");

    *poz_x = x;
    *origins_y_pozition = (unsigned int)y0 - *poz_y;
}

bool show_at_least_one_stat_data()
{
    bool show = false;
    for (size_t j = 0; j < global.number_of_columns; j++)
        show |= global.graph.show[j];
    return show;
}

size_t estimated_label_row_number()
{
    size_t line, len;

    if (!show_at_least_one_stat_data()) return 0;

    line = 1;
    len = 0;
    for (size_t j = 0; j < global.number_of_columns; j++)
        if (global.graph.show[j])
        {
            len += 10 + strlen(global.stat_names[j]) + 13; // space between the words + length of label name + '=' chackter +  value for the label
            if (len * 7.5 > global.graph.width)
            {
                len = 0;   
                line ++;
            }
        }
    return line;
}

void init_graph_window(bool *show)
{
    unsigned int size_x, size_y;
    unsigned int poz_x, poz_y;
    unsigned int i, y0, len;
    int time_index;
    unsigned int t1, t2, t_max, t_min, t_frame, t0, tn;
    double *data1, *data2;
    double *data_min, *data_max;
    double min, max;
    ImDrawList *draw_list;

    poz_x = global.window.margin;
    poz_y = global.window.margin + global.video.height + global.window.margin;

    data1 = (double *) malloc(global.number_of_columns * sizeof(double));
    data2 = (double *) malloc(global.number_of_columns * sizeof(double));
    
    ImGui::SetNextWindowPos(ImVec2(poz_x, poz_y));
    ImGui::SetNextWindowSize(ImVec2(global.graph.width, global.graph.height));
    ImGui::Begin("Graph", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoMove);
        len = add_file_location(global.statfilename);
        size_x = global.graph.width - 2 * global.window.margin;
        // substracting the height of the header and the margin belowe it
        size_y = global.graph.height - 5 * global.window.margin;
        // substricting the filenames height
        size_y -= round(((double) len * 10) / (size_x - 2 * global.window.margin)) * 13.5;
        // substracting the labels height
        size_y -= estimated_label_row_number() * 13.5;
        draw_list = ImGui::GetWindowDrawList();
        ImGui::BeginChild("##diagram", ImVec2(size_x, size_y), true);
            draw_list = ImGui::GetWindowDrawList();
            
            if (global.stats != NULL)
            {
                // poz_x += 60;
                // poz_y += 52;
                poz_x = ImGui::GetWindowPos().x + 50;
                poz_y = ImGui::GetWindowPos().y + global.window.margin / 2;
                size_y -= global.window.margin;
                size_x -= 60;
                
                data_max = (double *) malloc(global.number_of_columns * sizeof(double));
                data_min = (double *) malloc(global.number_of_columns * sizeof(double));

                max_stats(&t_max, data_max);
                min_stats(&t_min, data_min);
                
                min = HUGE_VAL_F32;
                for (size_t j = 0; j < global.number_of_columns - 1; j++)
                    if (global.graph.show[j] && min > data_min[j]) min = data_min[j];
                if ((global.graph.show[global.number_of_columns - 1] && min > data_min[global.number_of_columns - 1]) || min == HUGE_VAL_F32) min = data_min[global.number_of_columns - 1];

                max = (-1) * HUGE_VAL_F32;
                for (size_t j = 0; j < global.number_of_columns - 1; j++)
                    if (global.graph.show[j] && max < data_max[j]) max = data_max[j];
                if ((global.graph.show[global.number_of_columns - 1] && max < data_max[global.number_of_columns - 1]) || max == (-1) * HUGE_VAL_F32) max = data_max[global.number_of_columns - 1];
                
                if (min == max) max += 0.0125;

                free(data_max);
                free(data_min);

                draw_descartes_coordinate_system(draw_list, &poz_x, &poz_y, &size_x, &size_y, t_max, ImVec2(min, max), &y0);
                max = (min < 0.0 ? max - min : max);

                memcpy(data2, global.stats[0].data, global.number_of_columns * sizeof(double));
                t2 = global.stats[0].time;

                general_transform_coordinates(&t2, t_max, size_x, poz_x);
                for (size_t j = 0; j < global.number_of_columns; j++)
                {
                    general_transform_coordinates(&data2[j], max, size_y, poz_y, true);
                    // it has to shift the curves with the distance of negative values (size_y - y0)
                    data2[j] -= size_y - y0;
                }

                t0 = t2;
                t_frame = global.current_frame * 100;
                general_transform_coordinates(&t_frame, t_max, size_x, poz_x);
                time_index = -1;

                if (show_at_least_one_stat_data())
                    for (i = 1; i < global.N_stats; i++)
                    {
                        memcpy(data1, data2, global.number_of_columns * sizeof(double));
                        tn = t1 = t2;

                        memcpy(data2, global.stats[i].data, global.number_of_columns * sizeof(double));
                        t2 = global.stats[i].time;

                        general_transform_coordinates(&t2, t_max, size_x, poz_x);
                        for (size_t j = 0; j < global.number_of_columns; j++)
                        {
                            general_transform_coordinates(&data2[j], max, size_y, poz_y, true);
                            // it has to shift the curves with the distance of negative values (size_y - y0)
                            data2[j] -= size_y - y0;
                            if (global.graph.show[j]) draw_list->AddLine(ImVec2(t1, (int) data1[j]), ImVec2(t2, (int) data2[j]), ImColor(global.graph.line_colors[j]), 0.5);
                        }

                        if (t_frame > t1 && t_frame <= t2)
                        {
                            time_index = i;  // t_frame value is between (i - 1) and i
                            draw_list->AddLine(ImVec2(t_frame, poz_y - 15), ImVec2(t_frame, poz_y + size_y + 15), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }
                    }

                free(data1);
                free(data2);

                if (show_at_least_one_stat_data())
                    if (time_index == -1)
                    {
                        if (t_frame <= t0)
                        {
                            time_index = 0;
                            t_frame += 2;
                            draw_list->AddLine(ImVec2(t_frame, poz_y - 15), ImVec2(t_frame, poz_y + size_y + 15), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }

                        if (t_frame > tn)
                        {
                            time_index = global.N_stats - 1;
                            t_frame -= 2;
                            draw_list->AddLine(ImVec2(t_frame, poz_y - 15), ImVec2(t_frame, poz_y + size_y + 15), ImColor(ImVec4(0.2705, 0.9568, 0.2588, 1.0)), 1.5);
                        }
                    }
            }
        ImGui::EndChild();
        if (global.stats != NULL && time_index != -1) calculate_coordinates_on_graph(time_index);
    ImGui::End();
}

void calculate_coordinates_on_graph(int i)
{
    double *data1, *data2;
    double *values;
    int t1, t2, t_frame;
    char *text;
    size_t len;

    data1 = (double *) malloc(global.number_of_columns * sizeof(double));
    data2 = (double *) malloc(global.number_of_columns * sizeof(double));
    values = (double *) malloc(global.number_of_columns * sizeof(double));

    t_frame = global.current_frame * 100;
    memcpy(data2, global.stats[i].data, global.number_of_columns * sizeof(double));
    t2 = global.stats[i].time;

    if (i != 0)
    {
        memcpy(data1, global.stats[i].data, global.number_of_columns * sizeof(double));
        t1 = global.stats[i - 1].time;
    }
    else
    {
        memcpy(data1, data2, global.number_of_columns * sizeof(double));
        t1 = t2;
    }
    
    if (t1 == t2 || t1 == t_frame) 
        memcpy(values, data1, global.number_of_columns * sizeof(double));
    else
        for (size_t j = 0; j < global.number_of_columns; j++)
            values[j] = ((t_frame - t1) * (data2[j] - data1[j])) / (t2 - t1) + data1[j];

    len = global.graph.width;
    for (size_t j = 0; j < global.number_of_columns; j++)
        if (global.graph.show[j])
        {
            text = (char*) malloc(100);
            len += snprintf(text, 100, "%s = %2.8f\t", global.stat_names[j], values[j]);
            len += 10; // space between the words
            if (len * 7.5 < global.graph.width)
                ImGui::SameLine();
            else
                len = 0;
            ImGui::TextColored(global.graph.line_colors[j], "%s", text);
                        
            free(text);
        }

    free(values);
    free(data2);
    free(data1);
}

void save_video(bool *save_movie)
{
#ifdef OPENCV
    static int option = 0;
    static ImGuiFs::Dialog dlg;
    size_t length;
    static bool checked = false;

    ImGui::SetNextWindowSize(ImVec2(400, 210));
    if (ImGui::BeginPopupModal("Save video", save_movie, ImGuiWindowFlags_NoResize)) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushItemWidth(275);
        ImGui::InputText("##videofilename", global.save.filename, global.save.filename_length);
        ImGui::PopItemWidth();
        ImGui::PopItemFlag();
        ImGui::SameLine();
        const bool choose_file = ImGui::Button("Choose file");
        if (ImGui::IsItemDeactivated())
            checked = false;

        dlg.saveFileDialog(choose_file, "../videos", ".avi");
        if ((length = strlen(dlg.getChosenPath())) > 0 && !checked)
        {
            if (length > global.save.filename_length)
            {
                global.save.filename_length = ++ length;
                global.save.filename = (char *) realloc(global.save.filename, length);
            }
            strncpy(global.save.filename, check_path(dlg.getChosenPath()), length);
            global.save.filename[length - 1] = '\0';
            checked = true;
        }

        ImGui::Text("Padding:");
        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        ImGui::InputInt("##videopadding", &global.save.padding);
        ImGui::PopItemWidth();

        ImGui::Text("Choose one option:");
        ImGui::RadioButton("All", &option, 0);

        ImGui::RadioButton("From: ", &option, 1);
        if (option == 1)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            ImGui::InputInt("##from", &global.save.from);
            ImGui::PopItemWidth();
        }

        ImGui::RadioButton("Till: ", &option, 2);
        if (option == 2)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            ImGui::InputInt("##to", &global.save.to);
            ImGui::PopItemWidth();
        }

        ImGui::RadioButton("Intervall: ", &option, 3);
        if (option == 3)
        {
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputInt("##from", &global.save.from);
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text(" - ");
            ImGui::SameLine();
            ImGui::PushItemWidth(80);
            ImGui::InputInt("##to", &global.save.to);
            ImGui::PopItemWidth();
        }

        if (ImGui::Button("Save##Video"))
        {
            global.save.started = true;
            global.save.current = global.save.from;
            ImGui::OpenPopup("Saving...##Modal");
            // make_video("proba.avi", 10, 1001, global.movie.width, global.movie.height);
            // *save_movie = false;
        }
        ImGui::SetNextWindowSize(ImVec2(200, 80));
        if(ImGui::BeginPopupModal("Saving...##Modal", &global.save.started, ImGuiWindowFlags_NoResize))
        {
            ImGui::ProgressBar((double) (global.save.current - global.save.from) / (double) (global.save.to - global.save.from), ImVec2(-1.0, 0.0));
            // write to video file I choose
            static cv::VideoWriter video(global.save.filename, CV_FOURCC('M','J','P','G'), 45, cv::Size(global.movie.width, global.movie.height));
            cv::Mat frame = make_frame(global.save.current, global.movie.width, global.movie.height, global.save.padding);
            video.write(frame);
            global.save.current ++;
            if (global.save.current > global.save.to || ImGui::Button("Cancel##Progress"))
            {
                video.release();
                global.video.play = true;
                *save_movie = false;
                global.save.started = false;
                COLOR_NOTE;
                printf("File saved to %s\n", global.save.filename);
                COLOR_DEFAULT;
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine(); 
        if (ImGui::Button("Cancel##SetSaveDataPopup"))
        {
            global.video.play = true;
            *save_movie = false;
        }
        ImGui::EndPopup();
    }
#endif
}

void init_settings_menubar()
{
    size_t length;
    static ImGuiFs::Dialog dlg;
    bool open_movie, open_stats;
    static bool save_movie = false;/*, save_stats;*/

    open_movie = false;
    open_stats = false;
    
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::BeginMenu("Open"))
            {
                if (ImGui::MenuItem("Movie file", "CTRL+M")) 
                {
                    open_movie = true;
                    global.settings.open = 0;
                }
                
                if (ImGui::MenuItem("Statistics file", "CTRL+T"))
                {
                    open_stats = true;
                    global.settings.open = 1;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Save"))
            {
#ifndef OPENCV
                push_disable();
#endif
                if (ImGui::MenuItem("Movie to avi"))
                    save_movie = true;

#ifndef OPENCV
                pop_disable();
#endif
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

    if (open_movie || open_stats || save_movie) global.video.play = false;

    if (save_movie)
        ImGui::OpenPopup("Save video");

    save_video(&save_movie);

    // setting extension depeding on which file do we want to open
    static std::string extension;
    if (open_movie) extension = ".mvi";
    else if (open_stats) extension = ".txt";

    dlg.chooseFileDialog(open_movie || open_stats, "../../Time-Crystals/results", extension.c_str());
    if ((length = strlen(dlg.getChosenPath())) > 0)
    {
        if (strncmp(dlg.getChosenPath(), global.moviefilename, (length < global.length ? length : global.length)) != 0 &&
            strncmp(dlg.getChosenPath(), global.statfilename,  (length < global.length ? length : global.length)) != 0)
        {
            if (length > global.length)
            {
                // reallocating memory if the new filename is longer then the previous
                // length calculates just the 'normal' characters so we have to increment by one because of the null-character
                global.length = ++length;
                global.moviefilename = (char *) realloc(global.moviefilename, length);
                global.statfilename  = (char *) realloc(global.statfilename,  length);
            }

            if (global.settings.open == 1)
            {
                strncpy(global.statfilename, dlg.getChosenPath(), length);
                global.statfilename[length - 1] = '\0';
                read_statisticsfile_data();
                open_stats = false;
            }
            else if (global.settings.open == 0)
            {
                strncpy(global.moviefilename, dlg.getChosenPath(), length);
                global.moviefilename[length - 1] = '\0';
                read_moviefile_data();
                open_movie = false;
            }
            reset_zoom();
            global.movie.height = global.video.height - 5 * global.window.margin - global.video.button_size - ceil((double) strlen(global.moviefilename) * 6 / (double) (global.movie.width - 150)) * 13.5 - 2; // filename height
            global.current_frame = 0;
        }
        global.video.play = true;
        global.settings.open = -1;
    } 
    // else
    //     if (global.settings.open > -1)
    //     {
    //         global.movie.height = global.video.height - 5 * global.window.margin - global.video.button_size - ceil((double) strlen(global.moviefilename) * 6 / (double) (global.movie.width - 150)) * 13.5 - 2; // filename height
    //         global.current_frame = 0;
    //         global.video.play = true;
    //         global.settings.open = -1;
    //     }
}

void zoom()
{
    unsigned int w, h, len;
    char *str;

    static ImVec2 mouse_pos[2];
    ImVec2 second, text_pos;
    const double difference = 1.5;
    // right button on the mouse is pressed
    if (ImGui::IsMouseClicked(0))
    {
        if (ImGui::IsMousePosValid() && global.movie.zoom.i < 3 && global.settings.open == -1)
        // if click happend in video window
        {
            // checking if the click happend in the movie window
            ImVec2 click = ImGui::GetIO().MousePos;
            if ((click.x >= global.movie.poz_x && click.x <= global.movie.poz_x + global.movie.width ) &&
                (click.y >= global.movie.poz_y && click.y <= global.movie.poz_y + global.movie.height))
            {
                // save position where the click occured
                mouse_pos[global.movie.zoom.i] = ImGui::GetIO().MousePos;
                global.movie.zoom.i ++;
            }
        }
    }
    else
        // left mouse on the mouse is pressed
        // reset zoom
        if (ImGui::IsMouseClicked(1))
            reset_zoom();
    

    if (global.movie.zoom.i == 1 && global.settings.open == -1)
        if (ImGui::IsMousePosValid())
        {
            second = ImGui::GetIO().MousePos;
            w = abs(mouse_pos[0].x - second.x);
            h = abs(mouse_pos[0].y - second.y);
            if (h > difference * w || w > difference * h)
            // the given area will be distorted too much => do not apply the zoom and give the chance to the user to choose another second point
            {
                len = snprintf(NULL, 0, "Too big differece between width and height!");
                str = (char *) malloc(len + 1);
                snprintf(str, len + 1, "Too big differece between width and height!");
            }
            else
            {
                len = snprintf(NULL, 0, "%d x %d", w, h);
                str = (char *) malloc(len + 1);
                snprintf(str, len + 1, "%d x %d", w, h);
            }

            text_pos = second;
            if (mouse_pos[0].x < second.x)
            {
                if (mouse_pos[0].y < second.y) text_pos.x -= 7 * len;
                else 
                    if (mouse_pos[0].y > second.y) 
                    {
                        text_pos.x -= 7 * len;
                        text_pos.y = mouse_pos[0].y;
                    }
            }
            else
                if (mouse_pos[0].x > second.x)
                {
                    if (mouse_pos[0].y < second.y)
                        text_pos.x = mouse_pos[0].x - 7 * len;
                    else 
                        if (mouse_pos[0].y > second.y)
                        {
                            text_pos.x = mouse_pos[0].x - 7 * len;
                            text_pos.y = mouse_pos[0].y;
                        }
                }


            global.movie.draw_list->AddRect(mouse_pos[0], second, (ImU32) ImColor(colors[0]), 0.0, 15, 2.0);
            global.movie.draw_list->AddText(text_pos, (ImU32) ImColor(colors[0]), str);

            free(str);
        }

    if (global.movie.zoom.i == 2)
    {
        w = abs(mouse_pos[0].x - mouse_pos[1].x);
        h = abs(mouse_pos[0].y - mouse_pos[1].y);

        if (h > difference * w || w > difference * h || h < 5 || w < 5)
            // the given area will be distorted too much => do not apply the zoom and give the chance to the user to choose another second point
            // or the selected area is too small or the two selected points are the same
        {
            global.movie.zoom.i --;
            return;
        }

        for (size_t j = 0; j < global.movie.zoom.i; j++)
            global.movie.zoom.corners[j] = mouse_pos[j];

        if (mouse_pos[0].x < mouse_pos[1].x && mouse_pos[0].y < mouse_pos[1].y)
        {
            global.movie.zoom.corners[0] = mouse_pos[0];
            global.movie.zoom.corners[1] = mouse_pos[1];
        }
        else if (mouse_pos[0].x > mouse_pos[1].x && mouse_pos[0].y > mouse_pos[1].y)
        {
            global.movie.zoom.corners[0] = mouse_pos[1];
            global.movie.zoom.corners[1] = mouse_pos[0];  
        }
        else if (mouse_pos[0].x < mouse_pos[1].x && mouse_pos[0].y > mouse_pos[1].y)
        {
            global.movie.zoom.corners[0].x = mouse_pos[0].x;
            global.movie.zoom.corners[0].y = mouse_pos[1].y;
            global.movie.zoom.corners[1].x = mouse_pos[1].x;
            global.movie.zoom.corners[1].y = mouse_pos[0].y;  
        }
        else if (mouse_pos[0].x > mouse_pos[1].x && mouse_pos[0].y < mouse_pos[1].y)
        {
            global.movie.zoom.corners[0].x = mouse_pos[1].x;
            global.movie.zoom.corners[0].y = mouse_pos[0].y;
            global.movie.zoom.corners[1].x = mouse_pos[0].x;
            global.movie.zoom.corners[1].y = mouse_pos[1].y; 
        }

        // converting selected pixel values into measurment in the system
        for (size_t j = 0; j < global.movie.zoom.i; j++)
        {
            global.movie.zoom.corners[j].x = (global.movie.zoom.corners[j].x - global.movie.draw_x) / global.movie.proportion_x;
            global.movie.zoom.corners[j].y = (global.movie.zoom.corners[j].y - global.movie.draw_y) / global.movie.proportion_y;
        }

        // calculating zoomed area's width and height
        global.movie.zoom.width  = global.movie.zoom.corners[1].x - global.movie.zoom.corners[0].x;
        global.movie.zoom.height = global.movie.zoom.corners[1].y - global.movie.zoom.corners[0].y;

        // calculating new movie proportion
        global.movie.proportion_x = (double) global.movie.draw_width  / global.movie.zoom.width;
        global.movie.proportion_y = (double) global.movie.draw_height / global.movie.zoom.height;
        
        // if i = 3 we do not allow the zoom's usage till it is not reseted
        global.movie.zoom.i ++;
    }
}

void init_settings_window(bool *show)
{
    ImGui::SetNextWindowPos(ImVec2(global.settings.poz_x, global.settings.poz_y));
    ImGui::SetNextWindowSize(ImVec2(global.settings.width, global.settings.height));
    ImGui::Begin("Settings", show,  
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_MenuBar);

        init_settings_menubar();

        if (global.objects == NULL) push_disable();
        if (ImGui::CollapsingHeader("Movie"))
        {
            ImGui::Text("Pinningsites");
            if (global.N_pinningsites == 0) push_disable();
            ImGui::Checkbox("Show pinningsites", &global.movie.show_pinningsites);
            if (!global.movie.show_pinningsites) push_disable();
            ImGui::Checkbox("Show pinningsite grid lines", &global.movie.show_grid_lines);
            if (global.movie.show_grid_lines)
            {
                ImGui::ColorEdit3("Grid line color", (float *)&global.movie.grid_color);
                ImGui::DragFloat("Grid line width", &global.movie.grid_line_width, 0.05f, 0.1f, 5.0f, "%.2f");
            }
            ImGui::Checkbox("Decreese pinningsite radius", &global.movie.show_just_center_pinningsites);
            ImGui::Checkbox("Monocrome pinningsites", &global.movie.monocrome_pinningsites);
            if (global.movie.monocrome_pinningsites)
                ImGui::ColorEdit3("Pinningsite color", (float *)&global.movie.pinningsite_color);
            ImGui::Separator();
            if (!global.movie.show_pinningsites) pop_disable();
            if (global.N_pinningsites == 0) pop_disable();

            ImGui::Text("Particles");
            if (global.N_particles == 0) push_disable();
            ImGui::Checkbox("Show particles", &global.movie.show_particles);
            if (!global.movie.show_particles) push_disable();
            ImGui::Checkbox("Toggle trajectory", &global.movie.trajectories_on);
            if (global.movie.trajectories_on)
            {
                double spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                if (ImGui::ArrowButton("##left", ImGuiDir_Left)) { if (global.movie.particles_tracked > 1) global.movie.particles_tracked--; }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##right", ImGuiDir_Right)) { if (global.movie.particles_tracked < global.N_objects/4) global.movie.particles_tracked++; }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::Text("%d", global.movie.particles_tracked);

                ImGui::ColorEdit3("Trajectory color", (float *)&global.movie.traj_color);
                ImGui::SameLine(); 
                show_help_marker("Click on the colored square to open a color picker.\nClick and hold to use drag and drop.\nRight-click on the colored square to show options.\nCTRL+click on individual component to input value.\n");
                ImGui::DragFloat("Trajectory width", &global.movie.traj_width, 0.05f, 0.1f, 5.0f, "%.2f");
                show_help_marker("Click and drag to change the value");
            }
            ImGui::Checkbox("Monocrome particles", &global.movie.monocrome_particles);
            if (global.movie.monocrome_particles)
                ImGui::ColorEdit3("Particle color", (float *)&global.movie.particle_color);
            if (!global.movie.show_particles) pop_disable();
            ImGui::Separator();
            if (global.N_particles == 0) pop_disable();

            if (abs(global.SX - global.movie.zoom.width) < 0.05  && abs(global.SY - global.movie.zoom.height) < 0.05) push_disable();
            if (ImGui::TreeNode("Zoom"))
            {
                ImGui::TreePop();
                ImGui::Separator();
            }
            if (abs(global.SX - global.movie.zoom.width) < 0.05  && abs(global.SY - global.movie.zoom.height) < 0.05) pop_disable();
        }
        if (global.objects == NULL) pop_disable();

        if (global.stats == NULL) push_disable();
        if (ImGui::CollapsingHeader("Graph"))
        {
            if (ImGui::Checkbox("Show all data", &global.graph.show_all))
            {
                if (global.graph.show_all)
                    for (size_t j = 0; j < global.number_of_columns; j++)
                        global.graph.show[j] = true;
                else
                    for (size_t j = 0; j < global.number_of_columns; j++)
                        global.graph.show[j] = false;
            }

            if (ImGui::TreeNode("Data shown"))
            {
                bool all = true;
                for (size_t j = 0; j < global.number_of_columns; j++) 
                {
                    ImGui::Checkbox(global.stat_names[j], &global.graph.show[j]);
                    if (global.graph.show[j])
                    {
                        char* str = (char *) malloc(global.number_of_columns + 4);
                        snprintf(str, global.number_of_columns + 4, "##lc%zu", j);
                        ImGui::ColorEdit3(str, (float *)&global.graph.line_colors[j]);
                        free(str);
                    }
                    else
                        all = false;
                }

                global.graph.show_all = all;
                ImGui::TreePop();
                ImGui::Separator();
            }
        }
        if (global.stats == NULL) pop_disable();

        if (ImGui::CollapsingHeader("Help"))
        {
            // 62 characters per row
            if (ImGui::TreeNode("General"))
            {
                ImGui::BulletText("Double-click on title bar to collapse window.");
                ImGui::BulletText("CTRL+Click on a slider to input value as text.");
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Zoom"))
            {
                ImGui::BulletText("For zoom click in the movie just select two points in the\n movie which define the corners of the rectangle.");
                ImGui::BulletText("To reset zoom left click on the movie.");
                ImGui::TreePop();
            }
        }
    
    ImGui::End();
}
    
void start_main_loop()
{
    // Main loop
    while (!glfwWindowShouldClose(global.window.window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        init_video_window(NULL);
        init_graph_window(NULL);
        init_settings_window(NULL);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(global.window.window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(global.window.background_color.x, global.window.background_color.y, global.window.background_color.z, global.window.background_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
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
    ImGui::DestroyContext();

    glfwDestroyWindow(global.window.window);
    glfwTerminate();
}

void max_stats(unsigned int *t_max, double *data_max)
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
    }
    else
    {
        COLOR_WARNING;
        printf("WARNING (%s: line %d)\n\tNo statistics data found!\n", strrchr(__FILE__, '/') + 1, __LINE__);
        COLOR_DEFAULT;
    }
}

void min_stats(unsigned int *t_min, double *data_min)
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
    }
    else
    {
        COLOR_WARNING;
        printf("WARNING (%s: line %d)\n\tNo statistics data found!\n", strrchr(__FILE__, '/') + 1, __LINE__);
        COLOR_DEFAULT;
    }
}

void general_transform_coordinates(double *x, double x_max, int x_size, int distance_from_origin, bool flip)
{
    if (flip) *x = x_max - *x;
    *x = *x * (double) x_size / x_max + distance_from_origin;
}

void general_transform_coordinates(unsigned int *x, unsigned int x_max, unsigned int x_size, int distance_from_origin, bool flip)
{
    if (flip) *x = x_max - *x;
    *x = *x * x_size / x_max + distance_from_origin;
}

void transform_movie_coordinates(double *x, double *y, ImVec2 draw_pos, ImVec2 proportion)
{
    // translate point to the origin
    *x -= global.movie.zoom.corners[0].x;
    *y -= global.movie.zoom.corners[0].y;
    // flipping near y axis
    // global.movie.draw_height / global.movie.proportion_y --> the shown system height
    *y = global.movie.draw_height / global.movie.proportion_y - *y;

    *x = *x * proportion.x + draw_pos.x;
    *y = *y * proportion.y + draw_pos.y;
}

//transforms from simulation unit distances to Opengl units
void transform_distance(double *r)
{
    *r = *r * global.movie.proportion_x;
}

void read_image(GLuint *texture, const char *filename)
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

void show_help_marker(const char *desc)
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

unsigned int add_file_location(const char *filename)
{
    char *ptr;
    unsigned int length;
    ptr = realpath(filename, NULL);
    if (ptr == NULL)
    {
        size_t len = strlen(filename);
        ptr = (char *) malloc(len + 1);
        // copying len + 1 characters to get null pointer at the end as well
        memcpy(ptr, filename, len + 1);
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(ptr);
    ImGui::PopTextWrapPos();
    length = strlen(ptr);
    free(ptr);

    return length;
}

void push_disable()
{
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void pop_disable()
{
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

char *check_path(const char *filename)
{
    time_t now = time(NULL);
    char buff[16];
    strftime(buff, 16, "%Y%m%d_%H%M%S", localtime(&now));

    char *name, *project;
    name = (char *) malloc(global.save.filename_length);
    std::string default_filename = "../videos/test.avi";

    if (strstr(filename, "../") != NULL)
    // if in the selectd path there is step to the parent directory we use the default filename, 
    // because we want to prevent enabling writing in operating systems root
    {
        strncpy(name, default_filename.c_str(), default_filename.length());
        name[default_filename.length()] = '\0';
    }

    if ((project = const_cast<char*>(strstr(filename, global.project_name.c_str()))) == NULL)
    // the selected path is above the project root, we do not allow that
    {
        strncpy(name, default_filename.c_str(), default_filename.length());
        name[default_filename.length()] = '\0';
    }
    else
    // the selected path is somewhere in the project root
    {
        snprintf(name, global.save.filename_length, "%s%s", global.path, project + global.project_name.length() + 1);
    }

    char *extension = get_extension(name);
    if (extension == NULL)
    {
        snprintf(&name[strlen(name)], 5, ".avi");
        extension = get_extension(name);
    }
    else
    {
        if (strstr(".avi.mp4.mkv.mov.amv.m4p.gif", extension) == NULL)
        // if selected file does not have one of the extensions mentioned above we use the default filename
        {
            strncpy(name, default_filename.c_str(), default_filename.length());
            name[default_filename.length()] = '\0';
            extension = get_extension(name);
        }
    }

    if (file_exists(name))
    // if the selected file exists we change it's name to not overwrite the other file
    {
        name = remove_extension(name);
        size_t len = snprintf(NULL, 0, "%s_%s", name, buff);
        name = (char *) realloc(name, len + 1);
        snprintf(&name[strlen(name)], 16 + 5, "_%s%s", buff, extension);
    }

    return name;    
}
