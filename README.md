# Bspice
My attempt at making a [SPICE](https://en.wikipedia.org/wiki/SPICE)-ish piece of software.
## Build
This is just a CMAKE project, so if you know how to do that then go ahead. 
If not, then make a directory called build.
Then go to the directory and run:
```console
$ cmake ..
```
This will produce the required build files.
Then run 
```console
$ make
```
Which will compile the project and make the executable called `main`.
## Running
To run the project you must pass in the path to the `.circuit` file that you want to run.
For example:
```console
./main ../Examples/capacitor.circuit
```


