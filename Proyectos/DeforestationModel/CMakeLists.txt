cmake_minimum_required(VERSION 3.14)
project(IRDA_Deforestation VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/RasterGrid.cpp
    src/SpectralIndices.cpp
    src/DeforestationModel.cpp
    src/InteractiveApp.cpp
)

add_executable(irda_model ${SOURCES})

target_include_directories(irda_model PRIVATE include)

# En MinGW (WinLibs): si hay error "bits/messages_members.h not found",
# agregar -isystem <mingw>/include/c++/<version>/x86_64-w64-mingw32
