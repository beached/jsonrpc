cmake_minimum_required(VERSION 3.13)
project(json_rpc
        VERSION "0.1.0"
        DESCRIPTION "Generic JSON RPC Server"
        HOMEPAGE_URL "https://github.com/beached/jsonrpc"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS system regex REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory(extern/)

add_library(${PROJECT_NAME}
        src/json_rpc/json_rpc_dispatch.cpp
        src/json_rpc_server.cpp
        )

add_compile_options( -Wall -Wextra -pedantic -Werror )

add_custom_target( full )

target_link_libraries(${PROJECT_NAME} daw::json_link daw::curl_wrapper Threads::Threads Boost::headers Boost::system Boost::regex)
target_include_directories(${PROJECT_NAME} PUBLIC include/)
add_library(daw::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_dependencies( full ${PROJECT_NAME} )

add_executable(json_rpc_server_test tests/src/server_test.cpp tests/src/validate_email.cpp)
target_link_libraries(json_rpc_server_test ${PROJECT_NAME} Boost::regex )
add_dependencies( full json_rpc_server_test )

add_executable(json_rpc_client_test tests/src/client_test.cpp)
target_link_libraries(json_rpc_client_test ${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto CURL::libcurl )
add_dependencies( full json_rpc_client_test )
