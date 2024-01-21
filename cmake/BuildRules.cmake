message(STATUS "Setting buildrules")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")

# Below line is for using C++20 standard
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${${PROJECTNAME}_SOURCE_DIR}/bin)
