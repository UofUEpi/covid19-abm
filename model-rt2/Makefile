main.o: main.cpp ../epiworld.hpp
	g++ -std=c++17 -g -fopenmp -Wall -pedantic -O2 main.cpp -o main.o

README.md: README.qmd main.cpp 
	quarto render README.qmd


