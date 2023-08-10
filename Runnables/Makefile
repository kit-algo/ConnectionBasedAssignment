CC=g++ -fopenmp
FLAGS=-std=c++17 -pipe
OPTIMIZATION=-march=native -O3
DEBUG=-rdynamic -Werror -Wpedantic -pedantic-errors -Wall -Wextra -Wparentheses -Wfatal-errors -D_GLIBCXX_DEBUG -g -fno-omit-frame-pointer
RELEASE=-ffast-math -ftree-vectorize -Wfatal-errors -DNDEBUG

all: AssignmentRelease

AssignmentDebug:
	$(CC) $(FLAGS) $(OPTIMIZATION) $(DEBUG) -o Assignment Assignment.cpp

AssignmentRelease:
	$(CC) $(FLAGS) $(OPTIMIZATION) $(RELEASE) -o Assignment Assignment.cpp
