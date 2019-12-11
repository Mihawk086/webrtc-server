
add_executable(client client.cpp)

message(${LINK_LIB_LIST})

target_link_libraries(client ${LINK_LIB_LIST})
