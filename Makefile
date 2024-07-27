CC = g++
CXX_FLAGS = -fcoroutines -std=c++23 -O3
HEADERS = assistance.hpp awaiters.hpp coroutine_base.hpp Scheduler.hpp

all: test_main test

test_main: test_main.cpp $(HEADERS)
	g++ $@.cpp -o $@.exe $(CXX_FLAGS)

test: test.cpp
	g++ $^ -o $@.exe $(CXX_FLAGS)

run:
	./$^

run_test: test.exe
	@powershell ./$^

clean:
	@powershell rm ./*.exe