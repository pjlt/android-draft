cmake_minimum_required(VERSION 3.22.1)

project(lanthing)

set(CMAKE_CXX_STANDARD 17)
set(LT_ANDROID ON)
add_compile_definitions(LT_ANDROID=1)

set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

set(protobuf_MODULE_COMPATIBLE ON)
find_package(absl REQUIRED PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/prebuilt/protobuf/lib/cmake/absl)
find_package(utf8_range REQUIRED PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/prebuilt/protobuf/lib/cmake/utf8_range)
find_package(Protobuf REQUIRED PATHS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/prebuilt/protobuf/lib/cmake/protobuf)

add_subdirectory(ltproto)
add_subdirectory(ltlib)
add_subdirectory(transport)

add_library(${PROJECT_NAME} SHARED
        ${CMAKE_CURRENT_SOURCE_DIR}/capi/lanthing.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/client/native_client.h
        ${CMAKE_CURRENT_SOURCE_DIR}/client/native_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/client/jvm_client_proxy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/client/jvm_client_proxy.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/decoder/video_decoder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/decoder/video_decoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/decoder/ndk_video_decoder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/decoder/ndk_video_decoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/video_renderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/video_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/android_gl_pipeline.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/android_gl_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/android_dummy_renderer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/renderer/android_dummy_renderer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/ct_smoother.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/ct_smoother.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/video_decode_render_pipeline.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/video_decode_render_pipeline.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/video_statistics.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/drpipeline/video_statistics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/widgets/widgets_manager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/graphics/widgets/widgets_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/audio/player/audio_player.h
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/player/audio_player.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/player/sl_audio_player.h
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/player/sl_audio_player.cpp
)

target_link_libraries(${PROJECT_NAME}
        android
        log
        OpenSLES
        mediandk
        GLESv2
        EGL
        c++_shared
        ltlib
        transport
        ltproto
        protobuf::libprotobuf-lite
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)
