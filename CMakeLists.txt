cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

set(CPP_FILES
    src/main.cpp
    src/Bmaths/Bmaths.cpp
    src/Bmaths/AlgebraicEquationSolver.cpp
    src/Bmaths/DifferentialEquationSolver.cpp
    src/Bmaths/DAESolve.cpp
    src/Circuit.cpp
    src/fileParser.cpp
)

add_executable(main ${CPP_FILES})

add_custom_target(run
    COMMAND main
    DEPENDS main
)

add_custom_target(plot
    COMMAND octave --no-gui plotData.m
    DEPENDS run
)

