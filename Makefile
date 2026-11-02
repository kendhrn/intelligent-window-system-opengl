CXX ?= g++
CXXFLAGS ?= -O2 -Wall

ifeq ($(OS),Windows_NT)
    TARGET := iws.exe
    CXXFLAGS += -DFREEGLUT_STATIC
    LDLIBS := -static -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32
    RM := cmd /C del /Q
else
    TARGET := iws
    RM := rm -f
    ifeq ($(shell uname -s),Darwin)
        CXXFLAGS += -Wno-deprecated-declarations
        LDLIBS := -framework GLUT -framework OpenGL
    else
        LDLIBS := -lglut -lGLU -lGL -lm
    endif
endif

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	-$(RM) $(TARGET)

.PHONY: all run clean
