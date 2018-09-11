/**
 * @file TestNetworking.cpp
 * @brief Definition file.
 *
 * @author Igor Janjic
 * @date 02/19/2018
 */

#include "TestNetworking.h"


TestNetworking::TestNetworking() {}

TestNetworking::~TestNetworking() {}

void TestNetworking::SetUp() {}

void TestNetworking::TearDown() {}


/** Application main entry point.
 */
int main(int argc, char* argv[])
{
    // Initiate testing
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

