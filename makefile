
run: build
	./main

build:
	g++ src/main.cpp src/Bmaths.cpp -g -o main

plot: run
	octave plotData.m

clean:
	rm -f main test
