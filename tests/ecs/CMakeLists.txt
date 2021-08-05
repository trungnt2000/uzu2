
cmake_minimum_required( VERSION 3.10 )

set(TARGET_NAME ecs_test)

add_executable( ${TARGET_NAME} )

target_link_libraries( 
  ${TARGET_NAME}
  check_main
)

target_sources(
  ${TARGET_NAME}
  PRIVATE
  "example.c"
)

add_test(${TARGET_NAME} ${TARGET_NAME})
