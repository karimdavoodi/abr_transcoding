cmake_minimum_required (VERSION 3.15)
project (abr_transcode)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions( 
   # -fdiagnostics-color=always
   -fdiagnostics-show-location=every-line
   -DBOOST_LOG_DYN_LINK=1
   -Wall
   -g
   )

find_package(Boost COMPONENTS system log log_setup REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGST REQUIRED 
    gstreamer-1.0           # for all 
    glib-2.0                # for all
    )

aux_source_directory(src SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} ) 
target_include_directories(${PROJECT_NAME} PUBLIC 
        ${Boost_INCLUDE_DIRS}
        ${LIBGST_INCLUDE_DIRS}
    )
target_link_libraries (${PROJECT_NAME}  
        ${Boost_LIBRARIES} 
        ${LIBGST_LIBRARIES}
    )

    #add_subdirectory(test)
    #enable_testing ()
    #add_test (NAME MyTest COMMAND Test_av_input)
