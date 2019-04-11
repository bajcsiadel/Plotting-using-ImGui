EXE = plot
SOURCES = main.cpp globaldata.cpp drawing.cpp
SOURCES += ../imgui/examples/imgui_impl_glfw.cpp ../imgui/examples/imgui_impl_opengl2.cpp
SOURCES += ../imgui/sfml/imgui-SFML.cpp
SOURCES += ../imgui/imgui.cpp ../imgui/imgui_demo.cpp ../imgui/imgui_draw.cpp ../imgui/imgui_widgets.cpp ../imgui/imguifilesystem/imguifilesystem.cpp
DST_SRC = $(addprefix $(OUT_DIR), $(notdir $(SOURCES)))
HEADERS = globaldata.h drawing.h color.h
MKDIR_P = mkdir -p
OUT_DIR = ../build/
DIRECTORIES = $(OUT_DIR) ../videos
OBJS = $(addprefix $(OUT_DIR), $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
DIRECTORIES = $(OUT_DIR)
INCLUDES = ../imgui ../imgui/examples ../imgui/imguifilesystem ../imgui/sfml /usr/SFML
CXXFLAGS = $(addprefix -I, $(INCLUDES)) `pkg-config --cflags glfw3`
LIBS = `pkg-config --static --libs glfw3`

opencv=$(shell pkg-config opencv --cflags)
ifneq (,$(findstring -I, $(opencv)))
	CXXFLAGS += -DOPENCV
	CXXFLAGS += `pkg-config --cflags opencv`
	LIBS += `pkg-config --static --libs opencv`
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

all: clean directories copy_all $(EXE) delete_copies
	@echo $(GREEN)"Build complete for $(ECHO_MESSAGE)\n"$(DEFAULT)

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

directories: ${DIRECTORIES}

${DIRECTORIES}:
	@echo $(GREEN)"Creating directories"$(DEFAULT)
	${MKDIR_P} ${DIRECTORIES}
	@echo ""
