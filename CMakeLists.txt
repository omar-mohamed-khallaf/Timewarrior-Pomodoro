cmake_minimum_required(VERSION 3.22)
project(pomodoro VERSION "1.0.0")

configure_file(include/config.h.in config.h)
set(CMAKE_CXX_STANDARD 20)

aux_source_directory(src/ SRC)
add_executable(pomodoro ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE include/ ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME} -lncurses)

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        CONFIGURATIONS Release)