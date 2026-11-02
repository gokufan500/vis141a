ifeq ($(MPI),1)
	CXX = mpic++
	CXXFLAGS_EXTRA = -DUSE_MPI
else 
	CXX = ccache g++
	CXXFLAGS_EXTRA = 
endif

LDFLAGS = -lGL -ldl -lpthread -lX11 /usr/local/lib/libglfw3.a

TARGET = program
CXXFLAGS = -MMD -MP -std=c++20 -Wall -O2 -Iinclude -Iinclude/imgui $(CXXFLAGS_EXTRA)

-include $(OBJS:.0=.d)

SRCS = $(wildcard src/*.cpp src/*.c src/imgui/*.cpp)

all: $(TARGET)

OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	ccache $(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	ccache $(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	$(RM) $(TARGET)

