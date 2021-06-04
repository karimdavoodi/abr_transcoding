add_executable (Test_rabbitmq  test_main.cpp 
    test_rabbitmq.cpp     ../src/amqp_client.cpp )
target_link_libraries (Test_rabbitmq  ${Boost_LIBRARIES} ${LIBRABBITMQ_LIBRARIES}  pthread)

add_executable (Test_av_input  test_main.cpp 
    test_av_input.cpp     ../src/av_input.cpp )
target_link_libraries (Test_av_input  ${Boost_LIBRARIES} ${LIBAV_LIBRARIES}  pthread)
