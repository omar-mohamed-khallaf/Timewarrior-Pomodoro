cmake_minimum_required(VERSION 3.22)
project(tw-pomodoro VERSION "2.3.2")

configure_file(include/config.h.in config.h)
set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

aux_source_directory(src/ SRC)

if (ANDROID)
    aux_source_directory(src/sound/platform/android SRC)
else ()
    aux_source_directory(src/sound/platform/desktop SRC)
endif ()

add_executable(${PROJECT_NAME} ${SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        include/
        ${CMAKE_CURRENT_BINARY_DIR})

find_library(NCURSES ncurses REQUIRED)

if (ANDROID)
    target_link_libraries(${PROJECT_NAME} ${NCURSES} -lOpenSLES)
else ()
    find_library(OPENAL openal REQUIRED)
    find_library(VORBIS vorbis REQUIRED)
    find_library(VORBIS_FILE vorbisfile REQUIRED)
    target_link_libraries(${PROJECT_NAME} ${NCURSES} ${OPENAL} ${VORBIS} ${VORBIS_FILE})

    install(PROGRAMS extras/scripts/on-modify.99-tw-pomodoro
            DESTINATION $ENV{HOME}/.task/hooks/
            PERMISSIONS OWNER_READ OWNER_EXECUTE)
endif ()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        CONFIGURATIONS Release)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/
        DESTINATION share/${PROJECT_NAME}
        FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# make uninstall
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
        TARGET "uninstall"
        POST_BUILD
        COMMENT "Uninstall files with install_manifest.txt"
        COMMAND xargs rm -vf < install_manifest.txt && echo "${PROJECT_NAME} uninstalled"
)
