run:
	cd build && cmake .. && cmake --build . --target run

plot: run
	cd build && octave --no-gui plotData.m

perf: run
	cd build && gprof main gmon.out | gprof2dot -o output.dot && dot -Tpng output.dot -o output.png


