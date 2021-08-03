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

find_package(SDL REQUIRED)
find_package(SDL2_image 2.0 REQUIRED)
find_package(SDL2_mixer 2.0 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Lua 5.2 REQUIRED)
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_compile_options(${APP_NAME} PRIVATE "-fno-strict-aliasing")
endif()

target_link_libraries(${APP_NAME} PRIVATE 
	project_warnings
	${SDL_LIBRARIES}
	${SDL_MAIN_LIBRARY}
	${SDL2_IMAGE_LIBRARIES} 
	${SDL2_MIXER_LIBRARIES}
	${OPENGL_LIBRARIES}
	${JSON_C_LIBRARIES}
	$<TARGET_NAME_IF_EXISTS:json-c::json-c>
	${LUA_LIBRARIES}
	cglm_headers
)

target_include_directories(${APP_NAME} PRIVATE 
		${SDL_INCLUDE_DIRS}
		${SDL2_IMAGE_INCLUDE_DIRS} 
		${SDL2_MIXER_INCLUDE_DIRS} 
		${OPENGL_INCLUDE_DIR}
		${Glad_INCLUDE_DIR}
		${JSON_C_INCLUDE_DIRS}
		${LUA_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}/include
)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
  target_precompile_headers(
    ${APP_NAME}
    PRIVATE
		"src/precompiled.h"
)
endif()

macro(post_build_copy APP FILE_LIST)
	foreach(_file ${FILE_LIST})
		add_custom_command(TARGET ${APP} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_file}" $<TARGET_FILE_DIR:${APP}>)
	endforeach()
endmacro()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	#set_target_properties(${APP_NAME} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
	target_compile_definitions(${APP_NAME} PUBLIC NOMINMAX PRIVATE HAVE_M_PI)

	get_filename_component(SDL2_LIBRARY_DIR ${SDL_LIBRARIES} DIRECTORY)
	get_filename_component(SDL2_DIR ${SDL2_LIBRARY_DIR} DIRECTORY)
	set(SDL_DLL "${SDL2_DIR}/../SDL2.dll")
	post_build_copy(${APP_NAME} "${SDL_DLL}")

	list(GET LUA_LIBRARIES 0 __LUA_FIRST_LIB)
	get_filename_component(__LUA_LIBRARY_FILE_NAME ${__LUA_FIRST_LIB} NAME_WE)
  get_filename_component(__LUA_LIBRARY_DIR ${__LUA_FIRST_LIB} DIRECTORY)
	set(LUA_DLL "${__LUA_LIBRARY_DIR}/${__LUA_LIBRARY_FILE_NAME}.dll")
	post_build_copy(${APP_NAME} "${LUA_DLL}")

	if(SDL2_IMAGE_FOUND)
		get_filename_component(SDL2_IMAGE_LIBRARY_DIR ${SDL2_IMAGE_LIBRARY} DIRECTORY)
		set(_SDL2_IMAGE_DLLS SDL2_image libjpeg-9 libpng16-16 libtiff-5 libwebp-7 zlib1)
		foreach( _file ${_SDL2_IMAGE_DLLS} )
			list( APPEND SDL2_IMAGE_DLLS  "${SDL2_IMAGE_LIBRARY_DIR}/${_file}.dll" )
		endforeach()
	endif()
	post_build_copy(${APP_NAME} "${SDL2_IMAGE_DLLS}")

	if(SDL2_MIXER_FOUND)
		get_filename_component(SDL2_MIXER_LIBRARY_DIR ${SDL2_MIXER_LIBRARY} DIRECTORY)
    set(_SDL2_MIXER_DLLS SDL2_mixer libFLAC-8 libmodplug-1 libmpg123-0 libogg-0 libopus-0 libopusfile-0 libvorbis-0 libvorbisfile-3)
    foreach( _file ${_SDL2_MIXER_DLLS} )
      list( APPEND SDL2_MIXER_DLLS  "${SDL2_MIXER_LIBRARY_DIR}/${_file}.dll" )
    endforeach()
	endif()
	post_build_copy(${APP_NAME} "${SDL2_MIXER_DLLS}")

endif()

add_custom_command(TARGET ${APP_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${APP_NAME}>/res)

set(DIST_DIR ${CMAKE_SOURCE_DIR}/dist)

#install(TARGETS ${APP_NAME} 
#		DESTINATION ${DIST_DIR})
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/res 
#		DESTINATION ${DIST_DIR})
