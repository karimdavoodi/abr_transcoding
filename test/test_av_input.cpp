#include <iostream>
#include <catch.hpp>
#include "../src/av_input.h"

using namespace std;

TEST_CASE("Test of AvInput "){
    AvInput input;

    input.set_input("file:///home/karim/Videos/20.mp4");
}
