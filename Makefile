# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11 -pthread

# Make
main: lab2pt1.cpp lab2Pt2.cpp
	$(CC) $(CC_FLAGS) lab2pt1.cpp -o Part1
	$(CC) $(CC_FLAGS) lab2Pt2.cpp -o Part2
 
# Clean
clean:
	rm -f Part1 Part2 trace.json
