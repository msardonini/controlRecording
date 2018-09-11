if(GLOBAL_WITH_TESTING AND NOT HAS_PARENT)

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_TESTING")

    include_directories(
        ${NETLIB_ROOT}/test/include
    )

    add_executable(TestTcp
        src/TestTcp.cpp
    )

    target_link_libraries(TestTcp
        NetLib
        gtest
        gtest_main
        pthread
    )

    add_test(TestTcp TestTcp
        --gtest_color=yes)

    add_executable(TestNetworking
        src/TestNetworking.cpp
    )

    target_link_libraries(TestNetworking
        NetLib
        gtest
        gtest_main
        pthread
    )

    add_test(TestNetworking TestNetworking
        --gtest_color=yes)

endif()

