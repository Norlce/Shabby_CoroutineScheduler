CC = g++
CXX_FLAGS = -fcoroutines -std=c++23 -O3
INCLUDE_PATH = -I./include/
HEADERS = ./include/assistance.hpp ./include/awaiters.hpp ./include/coroutine_base.hpp ./include/Scheduler.hpp

all: example run

example: example.cpp $(HEADERS)
	g++ $@.cpp -o $@ $(CXX_FLAGS) $(INCLUDE_PATH)

run: example
	./$^

clean:
	@rm ./*.exe