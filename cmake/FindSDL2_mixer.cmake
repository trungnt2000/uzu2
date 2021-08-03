# - Locate SDL2_mixer library
# This module defines:
#  SDL2_MIXER_LIBRARIES, the name of the library to link against
#  SDL2_MIXER_INCLUDE_DIRS, where to find the headers
#  SDL2_MIXER_FOUND, if false, do not try to link against
#  SDL2_MIXER_VERSION_STRING - human-readable string containing the version of SDL2_mixer
#
# For backward compatiblity the following variables are also set:
#  SDL2_MIXER_LIBRARY (same value as SDL2_MIXER_LIBRARIES)
#  SDL2_MIXER_INCLUDE_DIR (same value as SDL2_MIXER_INCLUDE_DIRS)
#  SDL2_MIXER_FOUND (same value as SDL2_MIXER_FOUND)
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

if(NOT SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_INCLUDE_DIR)
  set(SDL2_MIXER_INCLUDE_DIR ${SDL2_MIXER_INCLUDE_DIR} CACHE PATH "directory cache
entry initialized from old variable name")
endif()

find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
  HINTS
    ENV SDL2_MIXER_DIR
    ENV SDL2MIXERDIR
    ENV SDLDIR
  PATH_SUFFIXES include/SDL2 include/SDL2.0 include
)

if(NOT SDL2_MIXER_LIBRARY AND SDL2_MIXER_LIBRARY)
  set(SDL2_MIXER_LIBRARY ${SDL2_MIXER_LIBRARY} CACHE FILEPATH "file cache entry
initialized from old variable name")
endif()

find_library(SDL2_MIXER_LIBRARY
  NAMES SDL2_mixer
  HINTS
    ENV SDL2_MIXER_DIR
    ENV SDL2MIXERDIR
    ENV SDLDIR
  PATH_SUFFIXES lib
)

if(SDL2_MIXER_INCLUDE_DIR AND EXISTS "${SDL2_MIXER_INCLUDE_DIR}/SDL2_mixer.h")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL2_mixer.h" SDL2_MIXER_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL2_MIXER_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL2_mixer.h" SDL2_MIXER_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL2_MIXER_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_MIXER_INCLUDE_DIR}/SDL2_mixer.h" SDL2_MIXER_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL2_MIXER_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL2_MIXER_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MAJOR "${SDL2_MIXER_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL2_MIXER_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_MINOR "${SDL2_MIXER_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL2_MIXER_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_MIXER_VERSION_PATCH "${SDL2_MIXER_VERSION_PATCH_LINE}")
  set(SDL2_MIXER_VERSION_STRING ${SDL2_MIXER_VERSION_MAJOR}.${SDL2_MIXER_VERSION_MINOR}.${SDL2_MIXER_VERSION_PATCH})
  unset(SDL2_MIXER_VERSION_MAJOR_LINE)
  unset(SDL2_MIXER_VERSION_MINOR_LINE)
  unset(SDL2_MIXER_VERSION_PATCH_LINE)
  unset(SDL2_MIXER_VERSION_MAJOR)
  unset(SDL2_MIXER_VERSION_MINOR)
  unset(SDL2_MIXER_VERSION_PATCH)
endif()

set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2_mixer
                                  REQUIRED_VARS SDL2_MIXER_LIBRARIES SDL2_MIXER_INCLUDE_DIRS
                                  VERSION_VAR SDL2_MIXER_VERSION_STRING)

# for backward compatiblity
set(SDL2_MIXER_LIBRARY ${SDL2_MIXER_LIBRARIES})
set(SDL2_MIXER_INCLUDE_DIR ${SDL2_MIXER_INCLUDE_DIRS})
set(SDL2_MIXER_FOUND ${SDL2_MIXER_FOUND})

if(SDL2_MIXER_FOUND AND NOT TARGET SDL2::mixer)
  add_library(SDL2::mixer SHARED IMPORTED)
  get_filename_component(SDL2_MIXER_LIBRARY_DIR ${SDL2_MIXER_LIBRARY} DIRECTORY)
  
  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(SDL2_MIXER_DLLS SDL2_mixer libFLAC-8 libmodplug-1 libmpg123-0 libogg-0 libopus-0 libopusfile-0 libvorbis-0 libvorbisfile-3)
    foreach( _file ${SDL2_MIXER_DLLS} )
      list( APPEND SDL2_MIXER_DLL_LIBRARIES  "${SDL2_MIXER_LIBRARY_DIR}/${_file}.dll" )
    endforeach( _file ${SDL2_MIXER_DLLS})
  endif()

  set_target_properties(SDL2::mixer PROPERTIES
    IMPORTED_IMPLIB "${SDL2_MIXER_LIBRARY}"
    IMPORTED_LOCATION "${SDL2_MIXER_DLL_LIBRARIES}"
    INTERFACE_COMPILE_OPTIONS "${PC_SDL2_MIXER_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR)
