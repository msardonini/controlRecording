/**
 * @file TestNetworking.hpp
 * @brief Tests the networking library.
 *
 * @author Igor Janjic
 * @date 02/19/2018
 */

#ifndef TEST_NETWORKING_HPP
#define TEST_NETWORKING_HPP

#include <pthread.h>
#include <unistd.h>

// GTest
#include <gtest/gtest.h>

// Ours
#include "Networking.h"


/** Fixture for global tests */
class TestNetworking : public ::testing::Test
{
protected:

    /** Default constructor.
     */
    TestNetworking();
  

    /** Default destructor.
     */
    virtual ~TestNetworking();


    /** Code here will be called immediately after the constructor (right
     *  before each test).
     */
    virtual void SetUp();


    /** Code here will be called immediately after each test (right
     *  before the destructor).
     */
    virtual void TearDown();

};  // TEST_NETWORKING


#endif  // TEST_NETWORKING_HPP

