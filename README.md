# Bspice
My attempt at making a [SPICE](https://en.wikipedia.org/wiki/SPICE)-ish piece of software.
## Build
This is just a CMAKE project, so if you know how to do that then go ahead. 
If not, then make a directory called build.
Then go to the directory and run:
```console
$ cmake ..
```
This will produce the required build files. If your on Linux, or some other Unix operating system, it will generate make files, so just run:
```console
$ make
```
If you're on windows it should generate a visual studio solution file that can be opened then compiled. 
This will produce an executable called `main`.
## Running
To run the project you must pass in the path to the `.circuit` file that you want to run.
For example:
```console
./main ../Examples/capacitor.circuit
```
## Dependencies
Compiling the project requires no dependencies, except for libc obviously. However the program outputs a `.m` file that can be used to plot the data using GNU octave or Mathworks MATLAB.
## Testing
This project has been tested on Linux and Windows 11, both cases worked.


