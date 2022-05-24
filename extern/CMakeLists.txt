# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/beached/jsonrpc
#

option(DAW_ENABLE_TESTING "Build unit tests" OFF)
if (DAW_ENABLE_TESTING)
  set(DAW_ENABLE_TESTING OFF)
endif ()

include(FetchContent)

FetchContent_Declare(
    daw_header_libraries
    GIT_REPOSITORY https://github.com/beached/header_libraries.git
    GIT_TAG master 
)

FetchContent_Declare(
    daw_json_link
    GIT_REPOSITORY https://github.com/beached/daw_json_link.git
    GIT_TAG v3
)

FetchContent_Declare(
    daw_curl_wrapper
    GIT_REPOSITORY https://github.com/beached/curl_wrapper.git
    GIT_TAG master
)

FetchContent_MakeAvailable(daw_json_link daw_curl_wrapper daw_header_libraries)


