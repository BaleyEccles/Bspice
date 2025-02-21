INPUT_FILE = NULL
# use somthing like:
# cd build && cmake .. && make && cd .. && make plot INPUT_FILE=../Examples/capacitor.circuit
run:
	cd build && ./main $(INPUT_FILE)

plot: run
	cd build && octave --no-gui plotData.m

perf: run
	cd build && gprof main gmon.out | gprof2dot -o output.dot && dot -Tpng output.dot -o output.png

release:
	cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . --target run
