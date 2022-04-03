cmake_policy(SET CMP0065 NEW)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(CURL CONFIG REQUIRED)
find_package(Boost 1.78.0 COMPONENTS container)

add_subdirectory(extern)

add_custom_target( full )

add_executable(json_rpc_server_test src/server_test.cpp src/validate_email.cpp)
target_link_libraries(json_rpc_server_test PRIVATE ${PROJECT_NAME} daw::daw-json-link Boost::regex)
add_dependencies(full json_rpc_server_test)

add_executable(json_rpc_client_test src/client_test.cpp)
target_link_libraries(json_rpc_client_test PRIVATE ${PROJECT_NAME} daw::daw-json-link daw::daw-curl-wrapper)
add_dependencies(full json_rpc_client_test)
