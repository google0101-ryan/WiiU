project(WIIU)

set(CMAKE_MINIMUM_REQUIRED_VERSION 3.16.3)

set(CXX_STANDARD c++20)
set(CMAKE_CXX_STANDARD 20)

set(SOURCES src/main.cpp
            src/system/WiiU.cpp
            src/bus/Bus.cpp
            src/cpu/ARM.cpp
            src/cpu/ARMInstructions.cpp
            src/cpu/Thumb.cpp
            src/cpu/ThumbAlu.cpp
            src/cpu/ThumbControlFlow.cpp
            src/cpu/DataProcessing.cpp
            src/cpu/LSU.cpp
            src/cpu/ControlFlow.cpp
            src/cpu/Multiply.cpp
            src/cpu/CP15.cpp
            src/hw/usb/OHCI.cpp
            src/hw/usb/EHCI.cpp
            src/hw/Latte.cpp)

set(CMAKE_BUILD_TYPE Debug)

add_executable(wiiu ${SOURCES})
set(TARGET_NAME wiiu)

include_directories(wiiu ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)

find_package(SDL2 REQUIRED)
include_directories(wiiu ${SDL2_INCLUDE_DIRS})

target_link_libraries(wiiu ${SDL2_LIBRARIES})