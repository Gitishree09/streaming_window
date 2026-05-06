# cmake_minimum_required(VERSION 3.16)

# project(QtRTSPApp LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_AUTOMOC ON)

# find_package(Qt6 REQUIRED COMPONENTS Widgets)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
# pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)

# add_executable(appQtRTSP
#     main.cpp
# )

# target_include_directories(appQtRTSP PRIVATE
#     ${GSTREAMER_INCLUDE_DIRS}
#     ${GST_VIDEO_INCLUDE_DIRS}
# )

# target_link_libraries(appQtRTSP PRIVATE
#     Qt6::Widgets
#     ${GSTREAMER_LIBRARIES}
#     ${GST_VIDEO_LIBRARIES}
# )


cmake_minimum_required(VERSION 3.16)

project(QtRTSPApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(PkgConfig REQUIRED)

# GStreamer core
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)

# Video support
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)

# NEW: appsink support (IMPORTANT)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)

add_executable(appQtRTSP
    main.cpp
)

target_include_directories(appQtRTSP PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GST_VIDEO_INCLUDE_DIRS}
    ${GST_APP_INCLUDE_DIRS}   # NEW
)

target_link_libraries(appQtRTSP PRIVATE
    Qt6::Widgets
    ${GSTREAMER_LIBRARIES}
    ${GST_VIDEO_LIBRARIES}
    ${GST_APP_LIBRARIES}      # NEW
)

# Optional but recommended (fixes warnings sometimes)
target_compile_options(appQtRTSP PRIVATE
    ${GSTREAMER_CFLAGS_OTHER}
    ${GST_VIDEO_CFLAGS_OTHER}
    ${GST_APP_CFLAGS_OTHER}
)

