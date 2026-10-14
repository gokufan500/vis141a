CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude

GLFW_LIB = ./glfw-binaries/libglfw3.a
LDFLAGS = -lGL -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

TARGET = program

all: $(TARGET)

$(TARGET): $(wildcard *.cpp)
	$(CXX) -g $(CXXFLAGS) -o $@ $^ $(GLFW_LIB) $(LDFLAGS)

clean:
	$(RM) $(TARGET)

