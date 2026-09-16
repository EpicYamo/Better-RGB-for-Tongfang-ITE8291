cmake_minimum_required(VERSION 3.15)
project(better-rgb C RC)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "Better RGB only builds for Windows (Win32 API + HID).")
endif()

add_executable(better-rgb WIN32
    src/main.c
    resources/resource.rc
	src/keymap.c
)

target_include_directories(better-rgb PRIVATE resources)

target_compile_definitions(better-rgb PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)