test: test.cpp
	g++ -o test test.cpp `pkg-config --libs icu-uc` -g

