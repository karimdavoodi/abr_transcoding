cmake_minimum_required(VERSION 3.10)
project(gst_fmp4_mq_sink)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions( 
    -fdiagnostics-show-location=every-line
    -fdiagnostics-generate-patch 
    -DBOOST_LOG_DYN_LINK=1
    -DGST_USE_UNSTABLE_API=1
    )

#GSTREAMER
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED 
    gstreamer-1.0
    glib-2.0    
    gstreamer-base-1.0
    )

set(PLUGIN gstfmp4mqsink )
add_library(${PLUGIN} SHARED src/${PLUGIN}.cpp )
target_link_libraries(${PLUGIN} ${GST_LIBRARIES} 
    -ldvbcsa                  
    -lcrypto
    )
target_include_directories(${PLUGIN} PUBLIC ${GST_INCLUDE_DIRS})

