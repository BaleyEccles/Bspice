CPP_FILES = src/main.cpp src/Bmaths/Bmaths.cpp src/Bmaths/AlgebraicEquationSolver.cpp src/Bmaths/DifferentialEquationSolver.cpp src/Bmaths/DAESolve.cpp src/Circuit.cpp

run: build
	./main

build:
	g++ $(CPP_FILES) -g -o main

plot: run
	octave --no-gui plotData.m

clean:
	rm -f main test
