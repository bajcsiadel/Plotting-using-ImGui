#ifndef drawing_h
#define drawing_h

#include "globaldata.h"

void AddFileLocation(const char*);
void calculateCoordinatesOnGraph(int);
int calculateLength(float);
void cleanup();
void generalTransformCoordinates(unsigned int*, unsigned int, unsigned int, int, bool = false);
void generalTransformCoordinates(float*, float, int, int, bool = false);
void initGraphWindow(bool*);
void initMovie(bool*);
void initSettingsMenuBar();
void initSettingsWindow(bool*);
void initVideoWindow(bool*);
int initWindow();
void maxStats(unsigned int*, float*);
void minStats(unsigned int*, float*);
void popDisable();
void pushDisable();
void readImage(GLuint*, const char*);
int setupGLFW();
bool showAtLeastOneStatData();
void ShowHelpMarker(const char*);
void startMainLoop();
void transformMovieCoordinates(float *x, float *y);
void transformDistance(float *r);

#endif