cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CPP_FILES
    src/main.cpp
    src/BMaths/algebraicEquationSolver.cpp
    src/BMaths/differentialEquationSolver.cpp
    src/BMaths/DAESolve.cpp
    src/BMaths/function.cpp
    src/component.cpp
    src/fileParser.cpp
    src/tokenParser.cpp
)

add_executable(main ${CPP_FILES})

target_include_directories(main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BMaths
)
