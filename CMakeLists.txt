cmake_minimum_required(VERSION 3.10)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(uzu LANGUAGES C)

include(src/CMakeLists.txt)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(JSON_C REQUIRED json-c)
  set(JSON_C_LIBRARIES ${JSON_C_STATIC_LDFLAGS})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	if(Glad_DIR)
		set(Glad_INCLUDE_DIR ${Glad_DIR}/include)
	  set(Glad_SOURCE ${Glad_DIR}/src/gl.c)
		if(NOT EXISTS "${Glad_INCLUDE_DIR}")
			message(FATAL_ERROR "Unable to find Glad2's include directory")
		endif()
		if(NOT EXISTS "${Glad_SOURCE}")
			message(FATAL_ERROR "Unable to find Glad2's source file")
		endif()
	else()
	  message(FATAL_ERROR "Glad2 is required")
	endif()
	set(SOURCE_FILES ${SOURCE_FILES} ${Glad_SOURCE})
	find_package(json_c REQUIRED)
endif()

find_package(SDL2 2.0 REQUIRED)
find_package(SDL2_image 2.0.5 REQUIRED)
find_package(SDL2_mixer 2.0.4 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua 5.1 REQUIRED)
add_subdirectory(external/cglm EXCLUDE_FROM_ALL)

set(APP_NAME "uzu_app")

add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

add_executable(${APP_NAME} ${SOURCE_FILES})

if(MSVC OR CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR NOT CMAKE_C_COMPILER_ID)
  string(REGEX REPLACE "/W[3|4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

target_compile_features(${APP_NAME} INTERFACE c_std_11)

target_link_libraries(${APP_NAME} PRIVATE 
	project_warnings
	SDL2::SDL2
	SDL2::SDL2main
	$<TARGET_NAME_IF_EXISTS:PkgConfig::JSON_C>
	$<TARGET_NAME_IF_EXISTS:json-c::json-c>
	OpenGL::GL
	${SDL2_IMAGE_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${LUA_LIBRARIES}
	cglm_headers
)

target_include_directories(${APP_NAME} PRIVATE 
		${SDL2_IMAGE_INCLUDE_DIRS} 
	  ${SDL2_MIXER_INCLUDE_DIRS} 
		${Glad_INCLUDE_DIR}
		${LUA_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}/include
)

# Very basic PCH example
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
  target_precompile_headers(
    ${APP_NAME}
    PRIVATE
		"src/precompiled.h"
)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	#set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
	target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX PRIVATE HAVE_M_PI)
endif()

add_custom_command(TARGET ${APP_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${APP_NAME}>/res)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

install(TARGETS ${APP_NAME} 
		DESTINATION ${DIST_DIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/res 
		DESTINATION ${DIST_DIR})
