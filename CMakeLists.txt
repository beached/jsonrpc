cmake_minimum_required(VERSION 3.13)
project(json_rpc
        VERSION "0.1.0"
        DESCRIPTION "Generic JSON RPC Server"
        HOMEPAGE_URL "https://github.com/beached/daw_json_rpc"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(extern/)

#add_library(${PROJECT_NAME} include/daw/json_rpc/json_rpc_dispatch.h include/daw/json_rpc/json_rpc_params.h include/daw/json_rpc/json_rpc_request.h include/daw/json_rpc/json_rpc_response.h include/daw/json_rpc/json_rpc_error.h include/daw/json_rpc/json_rpc_request_handler.h src/json_rpc/json_rpc_dispatch.cpp src/json_rpc_server/json_rpc_server_listener.cpp include/daw/json_rpc_server/json_rpc_server_listener.h src/json_rpc_server/json_rpc_server_session_detect.cpp include/daw/json_rpc_server/json_rpc_server_session_detect.h src/json_rpc_server/json_rpc_server_session_ssl.cpp include/daw/json_rpc_server/json_rpc_server_session_ssl.h src/json_rpc_server/json_rpc_server_session.cpp include/daw/json_rpc_server/json_rpc_server_session.h include/daw/json_rpc_server/json_rpc_server_error.h include/daw/json_rpc_server/json_rpc_server_session_plain.h src/json_rpc_server/json_rpc_server_session_plain.cpp)
add_library(${PROJECT_NAME}
        src/json_rpc/json_rpc_dispatch.cpp
        src/json_rpc_server.cpp
)

target_link_libraries(${PROJECT_NAME} httplib::httplib daw::json_link Threads::Threads)
target_include_directories(${PROJECT_NAME} PUBLIC include/)
#add_library(daw::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_executable(json_rpc_test main.cpp)
target_link_libraries(json_rpc_test ${PROJECT_NAME})
