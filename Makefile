CC = g++
CXX_FLAGS = -fcoroutines -std=c++23 -O3
INCLUDE_PATH = -I./include/
HEADERS = ./include/assistance.hpp ./include/awaiters.hpp ./include/coroutine_base.hpp ./include/Scheduler.hpp

all: test_main run

test_main: test_main.cpp $(HEADERS)
	g++ $@.cpp -o $@.exe $(CXX_FLAGS) $(INCLUDE_PATH)

run: test_main.exe
	./$^

clean:
	@powershell rm ./*.exe