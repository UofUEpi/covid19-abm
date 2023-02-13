main.o: main.cpp ../epiworld.hpp
	g++ -std=c++17 -g -fopenmp -Wall -pedantic -O2 main.cpp -o main.o

README.md: README.Rmd main.o population.txt
	Rscript --vanilla --verbose -e 'rmarkdown::render("README.Rmd")'

population.txt: population.R
	Rscript --vanilla --verbose population.R 
