add_library(demoinfogo-proto OBJECT
    "${CMAKE_CURRENT_LIST_DIR}/cstrike15_usermessages_public.proto"
    "${CMAKE_CURRENT_LIST_DIR}/netmessages_public.proto"
)

target_link_libraries(demoinfogo-proto PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_include_directories(demoinfogo-proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
    TARGET demoinfogo-proto
    IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
)
