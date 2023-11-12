cmake_minimum_required(VERSION 3.22.1)

project(lanthing)

add_library(${PROJECT_NAME} SHARED
        capi/lanthing.cpp
        client/native_client.h
        client/native_client.cpp
        graphics/decoder/video_decoder.h
        graphics/decoder/video_decoder.cp
        graphics/renderer/video_renderer.h
        graphics/renderer/video_renderer.cpp
        graphics/drpipeline/video_decode_render_pipeline.h
        graphics/drpipeline/video_decode_render_pipeline.cpp
)

target_link_libraries(${PROJECT_NAME}
        android
        log
        c++_shared
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)
