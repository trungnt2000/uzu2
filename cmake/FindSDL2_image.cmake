# - Locate SDL_image library
# This module defines:
#  SDL2_IMAGE_LIBRARIES, the name of the library to link against
#  SDL2_IMAGE_INCLUDE_DIRS, where to find the headers
#  SDL2_IMAGE_FOUND, if false, do not try to link against
#  SDL2_IMAGE_VERSION_STRING - human-readable string containing the version of SDL_image
#
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
#
# Created by Eric Wing. This was influenced by the FindSDL.cmake
# module, but with modifications to recognize OS X frameworks and
# additional Unix paths (FreeBSD, etc).

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
# Copyright 2012 Benjamin Eikel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(SDL2_IMAGE_INCLUDE_DIR SDL_image.h
  HINTS
    ENV SDL2_IMAGE_DIR
    ENV SDLIMAGEDIR
    ENV SDLDIR
  PATH_SUFFIXES include/SDL2 include/SDL2.0 include
)

find_library(SDL2_IMAGE_LIBRARY
  NAMES SDL2_image SDL2_image-2.0
  HINTS
    ENV SDL2_IMAGE_DIR
    ENV SDLIMAGEDIR
    ENV SDLDIR
  PATH_SUFFIXES lib
)

if(SDL2_IMAGE_INCLUDE_DIR AND EXISTS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL2_IMAGE_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_MAJOR "${SDL2_IMAGE_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_MINOR "${SDL2_IMAGE_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_IMAGE_VERSION_PATCH "${SDL2_IMAGE_VERSION_PATCH_LINE}")
  set(SDL2_IMAGE_VERSION_STRING ${SDL2_IMAGE_VERSION_MAJOR}.${SDL2_IMAGE_VERSION_MINOR}.${SDL2_IMAGE_VERSION_PATCH})
  unset(SDL2_IMAGE_VERSION_MAJOR_LINE)
  unset(SDL2_IMAGE_VERSION_MINOR_LINE)
  unset(SDL2_IMAGE_VERSION_PATCH_LINE)
  unset(SDL2_IMAGE_VERSION_MAJOR)
  unset(SDL2_IMAGE_VERSION_MINOR)
  unset(SDL2_IMAGE_VERSION_PATCH)
endif()

set(SDL2_IMAGE_LIBRARIES ${SDL2_IMAGE_LIBRARY})
set(SDL2_IMAGE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_image
                                  REQUIRED_VARS SDL2_IMAGE_LIBRARIES SDL2_IMAGE_INCLUDE_DIRS
                                  VERSION_VAR SDL2_IMAGE_VERSION_STRING)

if(SDL2_IMAGE_FOUND AND NOT TARGET SDL2::image)
  add_library(SDL2::image SHARED IMPORTED)
  get_filename_component(SDL2_IMAGE_LIBRARY_DIR ${SDL2_IMAGE_LIBRARY} DIRECTORY)
  
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SDL2_IMAGE_DLLS SDL2_image libjpeg-9 libpng16-16 libtiff-5 libwebp-7 zlib1)
    foreach( _file ${SDL2_IMAGE_DLLS} )
      list( APPEND SDL2_IMAGE_DLL_LIBRARIES  "${SDL2_IMAGE_LIBRARY_DIR}/${_file}.dll" )
    endforeach( _file ${SDL2_IMAGE_DLLS})
  endif()

  set_target_properties(SDL2::image PROPERTIES
    IMPORTED_IMPLIB "${SDL2_IMAGE_LIBRARY}"
    IMPORTED_LOCATION "${SDL2_IMAGE_DLL_LIBRARIES}"
    INTERFACE_COMPILE_OPTIONS "${PC_SDL2_IMAGE_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_IMAGE_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(SDL2_IMAGE_LIBRARY SDL2_IMAGE_INCLUDE_DIR)
