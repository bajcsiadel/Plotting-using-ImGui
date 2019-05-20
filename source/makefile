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
# setting output directory
OUT_DIR = ../build/
DIRECTORIES = $(OUT_DIR) ../videos
# set objectfile names from source files
OBJS = $(addprefix $(OUT_DIR), $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
INCLUDES = $(IMGUI) $(IMGUI_IMPL) $(IMGUI)/imguifilesystem $(IMGUI)/sfml /usr/SFML
CXXFLAGS = $(addprefix -I, $(INCLUDES)) `pkg-config --static --cflags glfw3`
LIBS = `pkg-config --static --libs glfw3`

# if opencv is installed set OPENCV as c++ processor variable 
# 2> /dev/null redirecting output if opencv is not installed
opencv=$(shell pkg-config opencv --cflags 2> /dev/null)
ifneq (,$(findstring -I, $(opencv)))
	CXXFLAGS += `pkg-config --static --cflags opencv`
	CXXFLAGS += -DOPENCV
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

# creating needed object files to compile $(EXE)
$(OUT_DIR)%.o: $(OUT_DIR)%.cpp
	@$(CXX) $(CXXFLAGS) -c -o $@ $<
	@echo "Create \t $@"

# this will be executed if we write in terminal just make
all: clean create

create: directories copy_all $(EXE) delete_copies
	@if [ $(OK) != ok ]; then \
		echo "You have to install openCV3 separately if you want to use the save function"; \
	else \
		echo "openCV3 is set"; \
	fi
	@echo $(GREEN)"Build complete for $(ECHO_MESSAGE)\n"$(DEFAULT)

$(EXE): $(HEADERS) $(OBJS)
	@echo "\nCreate executable \t"$(GREEN)"$(EXE)"$(DEFAULT)
	@$(CXX) -o $(OUT_DIR)$@ $^ $(CXXFLAGS) $(LIBS)
	@echo ""

# delete existing binary- and object files
# $(OUT_DIR)$(EXE) - binary created on Linux or Apple
# $(OUT_DIR)$(EXE).exe - binary created on Windows
clean:
	@echo $(GREEN)"Remove existing binary files"$(DEFAULT)
	@rm -f $(OUT_DIR)$(EXE) $(OUT_DIR)$(EXE).exe $(OBJS)
	@echo ""

# copying source and header files to the target directory, because it is easier to write a rule to creating object from source file
copy_all:
	@for src in $(SOURCES) $(HEADERS); \
	do \
		cp $$src $(OUT_DIR)$$(basename $$src); \
	done

# we do not really need these copies just for creating the objects, so after that we delete them
delete_copies:
	@rm -f $(OUT_DIR)*.cpp $(OUT_DIR)*.h

# execute rule directories if needed (if directories does not exist)
.PHONY: directories

directories: $(DIRECTORIES)

# creating the given directory if does not exist 
$(DIRECTORIES):
	@echo $(GREEN)"Creating directories"$(DEFAULT)
	$(MKDIR_P) $(DIRECTORIES)
	@echo ""

# this will be called when typing to the terminal make install
install: install_libraries create

# this will install the dependent libraries besides compiling the code
install_libraries:
	sudo apt-get update
	sudo apt-get install pkg-config
	sudo apt-get install libglfw3
	sudo apt-get install libglfw3-dev
	@echo $(GREEN)"Libraries has been installed!\n"$(DEFAULT)

uninstall: clean
