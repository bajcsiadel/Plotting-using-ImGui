# https://stackoverflow.com/questions/5618615/check-if-a-program-exists-from-a-makefile
# https://stackoverflow.com/questions/3774568/makefile-issue-smart-way-to-scan-directory-tree-for-c-files
EXE = plot
IMGUI = ../imgui
IMGUI_IMPL = $(IMGUI)/examples
# selecting .cpp files from current folder, which do not contain the word test
SOURCES = $(shell ls | grep -P '^(?!.*?test).*\.cpp')
SOURCES += $(IMGUI_IMPL)/imgui_impl_glfw.cpp $(IMGUI_IMPL)/imgui_impl_opengl2.cpp
SOURCES += $(IMGUI)/sfml/imgui-SFML.cpp
SOURCES += $(IMGUI)/imgui.cpp $(IMGUI)/imgui_demo.cpp $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_widgets.cpp $(IMGUI)/imguifilesystem/imguifilesystem.cpp
DST_SRC = $(addprefix $(OUT_DIR), $(notdir $(SOURCES)))
# getting files with .h extension form current folder
HEADERS = $(wildcard *.h)
MKDIR_P = mkdir -p
OUT_DIR = ../build/
DIRECTORIES = $(OUT_DIR) ../videos
OBJS = $(addprefix $(OUT_DIR), $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
DIRECTORIES = $(OUT_DIR)
INCLUDES = $(IMGUI) $(IMGUI_IMPL) $(IMGUI)/imguifilesystem $(IMGUI)/sfml /usr/SFML
CXXFLAGS = $(addprefix -I, $(INCLUDES)) `pkg-config --cflags glfw3`
LIBS = `pkg-config --static --libs glfw3`

# if opencv is installed set OPENCV as c++ processor variable 
opencv=$(shell pkg-config opencv --cflags 2> /dev/null)
ifneq (,$(findstring -I, $(opencv)))
	CXXFLAGS += -DOPENCV
	CXXFLAGS += `pkg-config --cflags opencv`
	LIBS += `pkg-config --static --libs opencv`
	OK = ok
else
	OK = not 
endif

ifeq ($(OS),Windows_NT) #WINDOWS
	ECHO_MESSAGE = "Windows"
	LIBS += -lgdi32 -lopengl32 -limm32

	# CXXFLAGS += -I../libs/gl3w

	GREEN = "[42m"
	DEFAULT = "[0m"
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux) #LINUX
		ECHO_MESSAGE = "Linux"
		LIBS += -lGL -lpthread -L/usr/X11R6/lib -lm -lX11 -lsfml-graphics -lsfml-window -lsfml-system
	endif

	ifeq ($(UNAME_S), Darwin) #APPLE
		ECHO_MESSAGE = "Mac OS X"
		LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

		CXXFLAGS += -I/usr/local/include
	endif

	GREEN = "\033[0;32m"
	DEFAULT = "\033[0m"
endif

CXXFLAGS += -Wall -Wformat -Wunused-variable -std=c++11
CFLAGS = $(CXXFLAGS)

$(OUT_DIR)%.o: $(OUT_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: clean create

create: directories copy_all $(EXE) delete_copies
	@echo $(GREEN)"Build complete for $(ECHO_MESSAGE)\n"$(DEFAULT)
	@if [ $(OK) != ok ]; then \
		echo "You have to install openCV3 separately if you want to use the save function\n"; \
	fi

$(EXE): $(HEADERS) $(OBJS)
	@echo ""
	$(CXX) -o $(OUT_DIR)$@ $^ $(CXXFLAGS) $(LIBS)
	@echo ""

clean:
	@echo $(GREEN)"Remove binary files"$(DEFAULT)
	rm -f $(OUT_DIR)$(EXE) $(OUT_DIR)$(EXE).exe $(OBJS)
	@echo ""

copy_all:
	@for src in $(SOURCES) $(HEADERS); \
	do \
		cp $$src $(OUT_DIR)$$(basename $$src); \
	done

delete_copies:
	@rm -f $(OUT_DIR)*.cpp $(OUT_DIR)*.h

.PHONY: directories

directories: $(DIRECTORIES)

$(DIRECTORIES):
	@echo $(GREEN)"Creating directories"$(DEFAULT)
	$(MKDIR_P} $(DIRECTORIES)
	@echo ""

install: install_libraries create

install_libraries:
	sudo apt-get update
	sudo apt-get install pkg-config
	sudo apt-get install libglfw3
	sudo apt-get install libglfw3-dev
	@echo $(GREEN)"Libraried has been installed!\n"$(DEFAULT)

uninstall: clean
