cmake_minimum_required(VERSION 3.16)
project(Chip8Emulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/SDL3-3.4.14")

find_package(SDL3 REQUIRED)

add_executable(chip8_emu
    SourceFiles/Main.cpp
    SourceFiles/Chip8.cpp
)

target_include_directories(chip8_emu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Headers)

target_link_libraries(chip8_emu PRIVATE SDL3::SDL3)

if (MSVC)
    add_custom_command(
        TARGET chip8_emu PRE_BUILD
        COMMAND powershell -Command "Stop-Process -Name chip8_emu, cppvsdbg -Force -ErrorAction SilentlyContinue; Start-Sleep -Milliseconds 500"
        COMMENT "Clearing existing executable and debugger locks before linking..."
    )
endif()