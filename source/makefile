CXX = g++
SOURCES = main.cpp globaldata.cpp drawing.cpp ../imgui/examples/imgui_impl_sdl.cpp ../imgui/examples/imgui_impl_opengl2.cpp
SOURCES += ../imgui/imgui.cpp ../imgui/imgui_demo.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_widgets.cpp
OBJS = $(addprefix ../build/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
HEADERS = globaldata.h drawing.h  CImg.h
UNAME_S := $(shell uname -s)
MKDIR_P = mkdir -p
DIRECTORIES = ../results/movies ../results/stats ../results/outs

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS = -lGL -ldl `sdl2-config --libs` -L/usr/X11R6/lib -lm -lpthread -lX11

	CXXFLAGS = -I ../imgui -I../imgui/examples  `sdl2-config --cflags` 
	CXXFLAGS += -Wall -Wformat -Wunused-variable
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`

	CXXFLAGS = -I ../imgui -I../imgui/examples -I/usr/local/include `sdl2-config --cflags`
	CXXFLAGS += -Wall -Wformat -Wunused-variable
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
	ECHO_MESSAGE = "Windows"
	if not exist "../results" mkdir ../results/movies ../results/stats ../results/outs
	LIBS = -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

	CXXFLAGS = -I ../imgui -I../imgui/examples `pkg-config --cflags sdl2`
	CXXFLAGS += -Wall -Wformat -Wunused-variable
endif

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

../build/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

../build/%.o: ../imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

../build/%.o: ../imgui/examples/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean plot
	@echo "\033[0;32mBuild complete for $(ECHO_MESSAGE)\033[0;37m\n"

plot: $(HEADERS) $(OBJS)
	@echo ""
	$(CXX) -o ../build/$@ $^ $(CXXFLAGS) $(LIBS)
	@echo ""

clean:
	rm -f ../build/plot $(OBJS)
	@echo ""
