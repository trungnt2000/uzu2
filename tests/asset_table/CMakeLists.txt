
cmake_minimum_required( VERSION 3.10 )

set(TARGET_NAME asset_table)

add_executable( ${TARGET_NAME} )

find_package(SDL2 REQUIRED)
target_link_libraries(${TARGET_NAME}
  check_main
  ${SDL2_LIBRARIES}
)


target_sources( ${TARGET_NAME} PRIVATE
  ../../src/graphics/asset_table.c
 asset_table_test.c)

target_include_directories(${TARGET_NAME} PRIVATE
  ../../include
  ${SDL2_INCLUDE_DIRS})

add_test(${TARGET_NAME} ${TARGET_NAME})
