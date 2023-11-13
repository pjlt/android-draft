cmake_minimum_required(VERSION 3.21)

project(ltlib)

add_library(${PROJECT_NAME} STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/logging.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ltlib/times.h

        ${CMAKE_CURRENT_SOURCE_DIR}/src/times.cpp
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