// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include "crow/http_request.h"
#include "crow/http_response.h"

#include "json_rpc/json_rpc_dispatch.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>

namespace daw::json_rpc {
	struct json_rpc_server {
		using storage_t = std::aligned_storage_t<1024, 64>;

	private:
		storage_t m_storage{ };

	public:
		json_rpc_server( );
		~json_rpc_server( );
		json_rpc_server( json_rpc_server &&other ) = delete;
		json_rpc_server &operator=( json_rpc_server &&rhs ) = delete;
		json_rpc_server( json_rpc_server const & ) = delete;
		json_rpc_server &operator=( json_rpc_server const & ) = delete;

		json_rpc_server &listen( std::uint16_t port ) &;
		json_rpc_server &listen( std::string_view host, std::uint16_t port ) &;
		json_rpc_server &stop( ) &;

		json_rpc_server &route_path_to(
		  std::string_view req_path, std::string_view method,
		  std::function<void( crow::request const &, crow::response & )> handler )
		  &;

		template<typename Result, typename Class>
		json_rpc_server &route_path_to( std::string_view req_path,
		                                std::string_view method, Result Class::*pm,
		                                Class &obj ) & {

			return route_path_to(
			  req_path, method,
			  [pm, obj]( crow::request const &req, crow::response &res ) mutable {
				  (void)( obj.*pm )( req, res );
			  } );
		}

		json_rpc_server &route_path_to( std::string_view req_path,
		                                json_rpc_dispatch &dispatcher ) &;

		json_rpc_server &
		route_path_to( std::string_view req_path, std::string const &fs_path,
		               std::optional<std::string_view> default_resource ) &;
	};
} // namespace daw::json_rpc