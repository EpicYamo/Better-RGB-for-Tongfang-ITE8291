cmake_minimum_required(VERSION 3.15)
project(better-rgb C RC)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT WIN32)
	message(FATAL_ERROR "Better RGB only builds for Windows (Win32 API + HID).")
endif()

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(third_party/hidapi)

add_executable(better-rgb WIN32
	src/main.c
	src/keymap.c
	src/log.c
	src/device.c
	resources/resource.rc
	src/config.c
	src/effects/effect_common.c
	src/effects/effect_breathing.c
	src/effects/effect_wave.c
	src/effects/effect_wheel.c
	src/effects/effect_lightning.c
	src/effects/effect_flame.c
)

target_include_directories(better-rgb PRIVATE resources src)

target_compile_definitions(better-rgb PRIVATE UNICODE _UNICODE _WIN32_WINNT=0x0A00)

target_link_libraries(better-rgb PRIVATE
	hidapi::hidapi
	setupapi
	hid
)
