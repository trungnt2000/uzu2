add_library(check_main STATIC check_main.c)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  if(EXISTS ${check_DIR})
    set(CHECK_INCLUDE_DIR ${check_DIR}/include)
    list(APPEND CHECK_LIBRARIES ${check_DIR}/lib/check.lib ${check_DIR}/lib/compat.lib)
    set(CHECK_FOUND 1)
  else()
    message(FATAL_ERROR "check_DIR doesn't exists. Tests requires check test framework.'")
  endif()
else()
  list(APPEND CHECK_LIBRARIES check m rt pthread subunit)
endif()

target_link_libraries(
  check_main
  PUBLIC
  ${CHECK_LIBRARIES}
  project_warnings)

target_include_directories(
  check_main 
  PUBLIC 
  ${CHECK_INCLUDE_DIR})

add_subdirectory(example_test)