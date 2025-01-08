
run: build
	./main

build:
	g++ src/main.cpp src/Bmaths.cpp src/Circuit.cpp -g -o main

plot: run
	octave --no-gui plotData.m

clean:
	rm -f main test
