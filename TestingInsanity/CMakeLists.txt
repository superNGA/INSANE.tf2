cmake_minimum_required(VERSION 3.20)
project(Insane_tf2 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "INSANITY.tf2/*.cpp" "INSANITY.tf2/*.cxx")

# Create executable
add_library(Insane_tf2 SHARED ${SOURCES})


# Yes, it must be msvc, cause I am not smart enough to make it work with clang-cl or gcc. ( I tried. )
if(NOT MSVC)
  message(FATAL_ERROR "This project must be built with MSVC (cl.exe).")
endif()


#//////////////////////////////////////////////////
# Linking DirectX9 library. ( abs. adrs. u might need to fix it. )
#//////////////////////////////////////////////////
target_link_libraries(Insane_tf2 PRIVATE d3d9 d3dx9)
target_include_directories(Insane_tf2 PRIVATE
    "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
)
target_link_directories(Insane_tf2 PRIVATE
    "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x64"  # or x86
)


#//////////////////////////////////////////////////
# Linking MinHook.lib x64 
#//////////////////////////////////////////////////
target_link_libraries(Insane_tf2 PRIVATE "${CMAKE_SOURCE_DIR}/MinHook.lib")
target_link_libraries(Insane_tf2 PRIVATE "${CMAKE_SOURCE_DIR}/INSANITY.tf2/Utility/Profiler")
# NOTE : CMAKE_SOURCE_DIR -> Where CMakeLists.txt is.
# NOTE : CMAKE_BINARY_DIR -> Where build files are.


set_target_properties(Insane_tf2 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_SOURCE_DIR}/../Output"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/../Output"
)

target_compile_options(Insane_tf2 PRIVATE /SUBSYSTEM:WINDOWS )
