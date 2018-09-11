# Compile examples
if(GLOBAL_WITH_EXAMPLES)
    add_executable(ExampleUdpImageServer
        ${RTKLIB_ROOT}/example/ExampleUdpImageServer.cpp
    )
    set_target_properties(ExampleUdpImageServer PROPERTIES OUTPUT_NAME "ExampleUdpImageServer")
    target_link_libraries(ExampleUdpImageServer NetLib)
    install(TARGETS ExampleUdpImageServer DESTINATION ${RTKLIB_INSTALL_DIR}/bin)

    add_executable(ExampleUdpImageClient
        ${RTKLIB_ROOT}/example/ExampleUdpImageClient.cpp
    )
    set_target_properties(ExampleUdpImageClient PROPERTIES OUTPUT_NAME "ExampleUdpImageClient")
    target_link_libraries(ExampleUdpImageClient NetLib)
    install(TARGETS ExampleUdpImageClient DESTINATION ${RTKLIB_INSTALL_DIR}/bin)


endif()
