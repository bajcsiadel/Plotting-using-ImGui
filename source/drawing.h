#ifndef drawing_h
#define drawing_h

#include "globaldata.h"

void AddFileLocation(const char*);
void calculateCoordinatesOnGraph(int);
int calculateNumberLength(float);
void cleanup();
void drawGrid(ImDrawList*);
size_t estimatedLabelRowNumber();
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
void resetZoom();
int setupGLFW();
void setVideoButtonsLocation();
bool showAtLeastOneStatData();
void ShowHelpMarker(const char*);
void startMainLoop();
void transformMovieCoordinates(float *x, float *y);
void transformDistance(float *r);
void zoom();

#endif /* drawing_h */