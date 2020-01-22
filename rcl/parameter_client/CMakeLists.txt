cmake_minimum_required(VERSION 3.5)

project(parameter_client LANGUAGES C)

find_package(ament_cmake REQUIRED)
find_package(rcl REQUIRED)

add_executable(${PROJECT_NAME} main.c)

ament_target_dependencies(${PROJECT_NAME}
  rcl
  )

install(TARGETS ${PROJECT_NAME}
  DESTINATION ${PROJECT_NAME}
  )
