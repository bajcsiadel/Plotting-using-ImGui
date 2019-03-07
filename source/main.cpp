// dear imgui: standalone example application for SDL2 + OpenGL
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_sdl_opengl3/ folder**
// See imgui_impl_sdl.cpp for details.

#include "globaldata.h"
#include "drawing.h"

#include <stdlib.h>

int main(int argc, char** argv)
{
    char *filename = (char *) malloc(255);
    if (argc == 2) memcpy(filename, argv[1], strlen(argv[1]));
    else filename[0] = '\0';    // zero length string
    initializeGlobalData(filename);
    free(filename);

    readMoviefileData();
    
    // write_frame_data_to_file();
    
    if (setupGLFW() == 0) {
        fprintf(stderr, "Glfw Error: could not initalize!\n");
        return 1;
    }

    if (initWindow() == 0) {
        fprintf(stderr, "Glfw Error: could not create window!\n");
        return 1;
    }

    startMainLoop();
    freeArrays();
    return 0;
}
