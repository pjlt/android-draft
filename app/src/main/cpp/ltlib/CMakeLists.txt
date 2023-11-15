cmake_minimum_required(VERSION 3.21)

project(ltlib)

add_library(${PROJECT_NAME} STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/logging.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/ltlib.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/pragma_warning.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/threads.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/times.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/time_sync.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/logging.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/threads.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/times.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/time_sync.cpp
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
            android
            log
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
)