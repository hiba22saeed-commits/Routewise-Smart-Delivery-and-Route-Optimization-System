CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic -O2

all: RouteWise

RouteWise: main.cpp *.h
	$(CXX) $(CXXFLAGS) main.cpp -o RouteWise

run: RouteWise
	./RouteWise

demo: RouteWise
	./RouteWise < tests/demo_input.txt

test: tests/test_structures.cpp *.h
	$(CXX) $(CXXFLAGS) tests/test_structures.cpp -o test_structures
	./test_structures

clean:
	rm -f RouteWise test_structures
