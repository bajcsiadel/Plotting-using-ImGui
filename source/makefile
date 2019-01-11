EXE = plot
SOURCES = main.cpp globaldata.cpp drawing.cpp
SOURCES += ../imgui/examples/imgui_impl_glfw.cpp ../imgui/examples/imgui_impl_opengl2.cpp
SOURCES += ../imgui/imgui.cpp ../imgui/imgui_demo.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_widgets.cpp ../imgui/imguifilesystem/imguifilesystem.cpp
HEADERS = globaldata.h drawing.h  CImg.h
MKDIR_P = mkdir -p
OUT_DIR = ../build/
DIRECTORIES = $(OUT_DIR)
OBJS = $(addprefix $(OUT_DIR), $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
DIRECTORIES = $(OUT_DIR)
INCLUDES = ../imgui ../imgui/examples ../imgui/imguifilesystem
CXXFLAGS = $(addprefix -I, $(INCLUDES)) 

ifeq ($(OS),Windows_NT) #WINDOWS
	ECHO_MESSAGE = "Windows"
	LIBS = -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

	CXXFLAGS += `pkg-config --cflags glfw3`
	# CXXFLAGS += -I../libs/gl3w

	GREEN = "[42m"
	DEFAULT = "[0m"
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux) #LINUX
		ECHO_MESSAGE = "Linux"
		LIBS = -lGL `pkg-config --static --libs glfw3` -lpthread -L/usr/X11R6/lib -lm -lX11

		CXXFLAGS += `pkg-config --cflags glfw3`
	endif

	ifeq ($(UNAME_S), Darwin) #APPLE
		ECHO_MESSAGE = "Mac OS X"
		LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs`

		CXXFLAGS += -I/usr/local/include `pkg-config --cflags glfw3`
	endif
	GREEN = "\033[0;32m"
	DEFAULT = "\033[0m"
endif
CXXFLAGS += -Wall -Wformat -Wunused-variable
CFLAGS = $(CXXFLAGS)

$(OUT_DIR)%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT_DIR)%.o: ../imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT_DIR)%.o: ../imgui/examples/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUT_DIR)%.o: ../imgui/imguifilesystem/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean directories plot
	@echo $(GREEN)"Build complete for $(ECHO_MESSAGE)\n"$(DEFAULT)

$(EXE): $(HEADERS) $(OBJS)
	@echo ""
	$(CXX) -o $(OUT_DIR)$@ $^ $(CXXFLAGS) $(LIBS)
	@echo ""

clean:
	@echo $(GREEN)"Remove binary files"$(DEFAULT)
	rm -f $(OUT_DIR)$(EXE) $(OUT_DIR)$(EXE).exe $(OBJS)
	@echo ""

.PHONY: directories

directories: ${DIRECTORIES}

${DIRECTORIES}:
	@echo $(GREEN)"Creating directories"$(DEFAULT)
	${MKDIR_P} ${DIRECTORIES}
	@echo ""
