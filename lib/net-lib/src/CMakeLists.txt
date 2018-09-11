file(GLOB NETLIB_SRC
    ${NETLIB_ROOT}/src/*.cpp
)

add_library(NetLib STATIC
    ${NETLIB_SRC}
)

target_link_libraries(NetLib
    pthread
)

target_include_directories(NetLib PUBLIC ${NETLIB_INCLUDE_DIR})

install(TARGETS NetLib DESTINATION ${NETLIB_INSTALL_DIR}/lib)

# Install headers 
install(DIRECTORY ${NETLIB_ROOT}/include DESTINATION ${NETLIB_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
