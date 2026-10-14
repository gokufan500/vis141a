
ifeq ($(MPI),1)
	CXX = mpic++
	CXXFLAGS_EXTRA = -DUSE_MPI
else 
	CXX = g++
	CXXFLAGS_EXTRA = 
endif

GLFW_LIB = ./glfw-binaries/libglfw3.a
GLFW_INC = ./glfw-binaries/include
LDFLAGS = -lGL -ldl -lpthread -lX11

TARGET = program
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude -I$(GLFW_INC) src/glad.c $(CXXFLAGS_EXTRA)

all: $(TARGET)

$(TARGET): $(wildcard *.cpp)
	$(CXX) -g $(CXXFLAGS) -o $@ $^ $(GLFW_LIB) $(LDFLAGS)

clean:
	$(RM) $(TARGET)

