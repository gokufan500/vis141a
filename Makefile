ifeq ($(MPI),1)
	CXX = mpic++
	CXXFLAGS_EXTRA = -DUSE_MPI
else 
	CXX = g++
	CXXFLAGS_EXTRA = 
endif

LDFLAGS = -lGL -ldl -lpthread -lX11 -lglfw

TARGET = program
CXXFLAGS = -std=c++20 -Wall -O2 -Iinclude  $(CXXFLAGS_EXTRA)

SRCS = $(wildcard src/*.cpp)

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) -g $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET)

