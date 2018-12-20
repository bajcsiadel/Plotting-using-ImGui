#ifndef drawing_h
#define drawing_h

#include "globaldata.h"

void AddFileLocation(const char*);
void calculateCoordinatesOnGraph(int);
void cleanup();
void generalTransformCoordinates(int*, int, int, int, bool = false);
void generalTransformCoordinates(float*, float, int, int, bool = false);
void initGraphWindow(bool*);
void initMovie(bool*);
void initSettingsMenuBar();
void initSettingsWindow(bool*);
void initVideoWindow(bool*);
void initWindow();
void maxStats(int*, float*, float*, float*);
void readImage(const char*, GLuint*);
void readImages();
void setupSDL();
void ShowHelpMarker(const char*);
void startMainLoop();
void transformMovieCoordinates(float *x, float *y);
void transformDistance(float *r);

#endif