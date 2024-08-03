#[=======================================================================[.rst:
FindPortAudio
-------

Finds the PortAudio library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PortAudio::PortAudio``
  The PortAudio library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PortAudio_FOUND``
  True if the system has the PortAudio library.
``PortAudio_VERSION``
  The version of the PortAudio library which was found.

#]=======================================================================]

# Use pkg-config if available
find_package(PkgConfig QUIET)
pkg_check_modules(PC_PORTAUDIO QUIET portaudio-2.0)

# Find the headers and library
find_path(
  PortAudio_INCLUDE_DIR
  NAMES "portaudio.h"
  HINTS "${PC_PORTAUDIO_INCLUDEDIR}")

find_library(
  PortAudio_LIBRARY
  NAMES "portaudio"
  HINTS "${PC_PORTAUDIO_LIBDIR}")

function ( get_target_properties_from_pkg_config _library _prefix _out_prefix )
  if ( "${_library}" MATCHES "${CMAKE_STATIC_LIBRARY_SUFFIX}$" )
    set ( _cflags ${_prefix}_STATIC_CFLAGS_OTHER )
    set ( _link_libraries ${_prefix}_STATIC_LIBRARIES )
    set ( _library_dirs ${_prefix}_STATIC_LIBRARY_DIRS )
  else ()
    set ( _cflags ${_prefix}_CFLAGS_OTHER )
    set ( _link_libraries ${_prefix}_LIBRARIES )
    set ( _library_dirs ${_prefix}_LIBRARY_DIRS )
  endif ()

  # The link_libraries list always starts with the library itself, and POP_FRONT is >=3.15
  list(REMOVE_AT "${_link_libraries}" 0)

  set ( ${_out_prefix}_compile_options "${${_cflags}}" PARENT_SCOPE )
  set ( ${_out_prefix}_link_libraries "${${_link_libraries}}" PARENT_SCOPE )
  set ( ${_out_prefix}_link_directories "${${_library_dirs}}" PARENT_SCOPE )
endfunction ( get_target_properties_from_pkg_config )

# Handle transitive dependencies
if(PC_PORTAUDIO_FOUND)
  get_target_properties_from_pkg_config("${PortAudio_LIBRARY}" "PC_PORTAUDIO"
                                        "_portaudio")
else()
  set(_portaudio_link_libraries "ALSA::ALSA" ${MATH_LIBRARY} "Threads::Threads")
endif()

# Forward the result to CMake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PortAudio REQUIRED_VARS "PortAudio_LIBRARY" "PortAudio_INCLUDE_DIR")

if(PortAudio_FOUND AND NOT TARGET PortAudio::PortAudio)
  add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
  set_target_properties(
    PortAudio::PortAudio
    PROPERTIES IMPORTED_LOCATION "${PortAudio_LIBRARY}"
               INTERFACE_COMPILE_OPTIONS "${_portaudio_compile_options}"
               INTERFACE_INCLUDE_DIRECTORIES "${PortAudio_INCLUDE_DIR}"
               INTERFACE_LINK_LIBRARIES "${_portaudio_link_libraries}"
               INTERFACE_LINK_DIRECTORIES "${_portaudio_link_directories}")
endif()

mark_as_advanced(PortAudio_INCLUDE_DIR PortAudio_LIBRARY)
