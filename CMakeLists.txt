cmake_minimum_required (VERSION 3.15)
project (abr_transcode)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions( 
                 -fdiagnostics-show-location=every-line
                 -DBOOST_LOG_DYN_LINK=1
                 -Wall
                 -g
                 )

find_package(Boost COMPONENTS system log log_setup REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBRABBITMQ REQUIRED librabbitmq)
pkg_check_modules(LIBAV REQUIRED 
            libavdevice   
            libavformat  
            libavresample 
            libavfilter  
            libavcodec    
            libavutil     
            )

add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/av_input.cpp
    src/av_output.cpp
    src/amqp_client.cpp 
    src/util.cpp
    )
#target_include_directories(${PROJECT_NAME} PUBLIC third_party)
target_link_libraries (${PROJECT_NAME}  
        SimpleAmqpClient
        ${Boost_LIBRARIES} 
        ${LIBRABBITMQ_LIBRARIES} 
        ${LIBAV_LIBRARIES} )

    #add_subdirectory(test)
    #enable_testing ()
    #add_test (NAME MyTest COMMAND Test_av_input)
