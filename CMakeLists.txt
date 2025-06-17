cmake_minimum_required(VERSION 3.10)
project(ChimeraSpoofer C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files for the main spoofer executable
add_executable(spoofer src/spoofer.cpp)

# Add source files for the LD_PRELOAD interceptor
add_library(interceptor SHARED src/interceptor.cpp)
set_target_properties(interceptor PROPERTIES PREFIX "") # Removes the 'lib' prefix

# Install rules
install(TARGETS spoofer interceptor
        RUNTIME DESTINATION bin)
install(FILES chimera.sh
        DESTINATION bin
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ) 