cmake_minimum_required(VERSION 3.1)
#add find sdl2 and sdl2 image modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fno-strict-aliasing")
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(uzu LANGUAGES C)
add_subdirectory(external/cglm EXCLUDE_FROM_ALL)

include(src/CMakeLists.txt)


if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(PkgConfig REQUIRED)
	find_package(SDL2 REQUIRED)
	find_package(SDL2_image REQUIRED)
	find_package(SDL2_mixer REQUIRED)
	find_package(SDL2_ttf REQUIRED)
	find_package(SDL2_gfx REQUIRED)
    find_package(EXPAT 2.2.5 MODULE REQUIRED)
	find_package(Lua REQUIRED)
	find_package(OpenGL REQUIRED)
	pkg_check_modules(JSON_C REQUIRED json-c)
	set(JSON_C_LIBRARIES ${JSON_C_STATIC_LDFLAGS})
	set(JSON_C_INCLUDE_DIRS ${JSON_C_INCLUDE_DIRS})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(SDL2_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/)
		set(SDL2_LIBRARIES ${CMAKE_SOURCE_DIR}/libraries/)

		set(SDL2_IMAGE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/)
		set(SDL2_IMAGE_LIBRARIES ${CMAKE_SOURCE_DIR}/libraries/)

		set(SDL2_MIXER_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/)
		set(SDL2_MIXER_LIBRARIES ${CMAKE_SOURCE_DIR}/libraries/)

		set(SDL2_TTF_LIBRARIES ${CMAKE_SOURCE_DIR}/libraries/)
		set(SDL2_TTF_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/)

		set(JSON_C_LIBRARIES ${CMAKE_SOURCE_DIR}/libraries/)
		set(JSON_C_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE 
		${SDL2_INCLUDE_DIRS} 
		${SDL2_IMAGE_INCLUDE_DIRS} 
		${SDL2_MIXER_INCLUDE_DIRS} 
		${SDL2_TTF_INCLUDE_DIRS}
		${JSON_C_INCLUDE_DIRS}
		${SDL2_GFX_INCLUDE_DIRS}
		${EXPAT_INCLUDE_DIRS}
		${LUA_INCLUDE_DIR}
		${OPENGL_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
		${SDL2_LIBRARIES} 
		${SDL2_IMAGE_LIBRARIES} 
		${SDL2_MIXER_LIBRARIES} 
		${SDL2_TTF_LIBRARIES}
		${SDL2_GFX_LIBRARIES}
		${EXPAT_LIBRARIES}
		${JSON_C_LIBRARIES}
		${LUA_LIBRARIES}
		${OPENGL_LIBRARIES}
		cglm_headers
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

install(TARGETS ${PROJECT_NAME} 
		DESTINATION ${DIST_DIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/res 
		DESTINATION ${DIST_DIR})
