# =============================================================================
#  Makefile - 3D Cricket Stadium Simulation (OpenGL / FreeGLUT)
#  Usage:   make          build
#           make run      build and run
#           make clean    remove objects and the binary
# =============================================================================

CXX      := g++
CXXFLAGS := -O2 -Wall -Wextra -std=c++98
TARGET   := CricketStadium
SRCS     := main.cpp camera.cpp stadium.cpp animation.cpp \
            lighting.cpp texture.cpp input.cpp
OBJS     := $(SRCS:.cpp=.o)

UNAME := $(shell uname -s 2>/dev/null || echo Windows)

ifeq ($(UNAME),Darwin)
    # macOS - uses the built-in GLUT framework
    LIBS := -framework OpenGL -framework GLUT
    CXXFLAGS += -Wno-deprecated-declarations
else ifeq ($(UNAME),Windows)
    # Windows + MinGW
    LIBS := -lfreeglut -lopengl32 -lglu32
    TARGET := CricketStadium.exe
else
    # Linux / WSL
    LIBS := -lglut -lGLU -lGL -lm
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# header dependencies
main.o      : main.cpp common.h camera.h stadium.h lighting.h texture.h animation.h input.h
camera.o    : camera.cpp camera.h common.h
stadium.o   : stadium.cpp stadium.h common.h texture.h lighting.h animation.h
animation.o : animation.cpp animation.h common.h lighting.h
lighting.o  : lighting.cpp lighting.h common.h
texture.o   : texture.cpp texture.h common.h
input.o     : input.cpp input.h camera.h animation.h texture.h lighting.h common.h

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) CricketStadium.exe

.PHONY: all run clean
