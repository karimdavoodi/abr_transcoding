#include <iostream>
#include <catch.hpp>
#include "../src/amqp_client.h"

using namespace std;

TEST_CASE("Test of AmqpClient "){
    AmqpClient client;
    client.set_host("127.0.0.1", "5672", "guest", "guest");

    REQUIRE( client.connect() == true );
    REQUIRE(client.send_queue("test_queue", "Hello", 5));
}
