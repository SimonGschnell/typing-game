cmake_minimum_required(VERSION 3.16)
project(TypingGame LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

# Add the path to the curlpp header files
include_directories(/usr/local/include)

# Add the path to the curlpp libraries
link_directories(/usr/local/lib)

# Find the nlohmann_json package
find_package(nlohmann_json REQUIRED)

add_executable(main src/main.cpp src/GameText.cpp src/PokeApi.cpp)
target_link_libraries(main PRIVATE sfml-graphics curlpp curl nlohmann_json::nlohmann_json)
target_compile_features(main PRIVATE cxx_std_17)
if(WIN32)
    add_custom_command(
        TARGET main
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()
