cmake_minimum_required(VERSION 3.13)
project(json_rpc
        VERSION "0.1.0"
        DESCRIPTION "Generic JSON RPC Server"
        HOMEPAGE_URL "https://github.com/beached/jsonrpc"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost COMPONENTS system regex filesystem REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_path(CROW_INCLUDE_DIRS "crow.h")

add_subdirectory(extern/)

add_library(${PROJECT_NAME}
        src/json_rpc/json_rpc_dispatch.cpp
        src/json_rpc_server.cpp
        )

add_compile_options(-Wall -Wextra -pedantic -Werror)

add_custom_target(full)

target_link_libraries(${PROJECT_NAME} PRIVATE daw::daw-json-link Boost::headers Boost::system Boost::regex)
target_include_directories(${PROJECT_NAME} PUBLIC include/ ${CROW_INCLUDE_DIRS})
add_library(daw::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_dependencies(full ${PROJECT_NAME})

add_executable(json_rpc_server_test tests/src/server_test.cpp tests/src/validate_email.cpp)
target_link_libraries(json_rpc_server_test PRIVATE ${PROJECT_NAME} daw::daw-json-link Boost::regex)
add_dependencies(full json_rpc_server_test)

add_executable(json_rpc_client_test tests/src/client_test.cpp)
target_link_libraries(json_rpc_client_test PRIVATE ${PROJECT_NAME} daw::daw-json-link daw::curl_wrapper)
add_dependencies(full json_rpc_client_test)
