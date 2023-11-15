cmake_minimum_required(VERSION 3.21)
project(transport)

add_library(${PROJECT_NAME}_api INTERFACE)
target_include_directories(${PROJECT_NAME}_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/transport/transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/transport_dummy.cpp
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_NAME}_api
)

add_subdirectory(rtc/android)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        rtc
)