// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include "json_rpc/json_rpc_dispatch.h"

#include <crow.h>
#include <crow/middlewares/cookie_parser.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>

namespace daw::json_rpc {
	struct json_rpc_server {
		using storage_t = std::aligned_storage_t<3376, 64>;
		using cookie_t = typename crow::CookieParser::context;

	private:
		crow::App<crow::CookieParser> server{ };

	public:
		json_rpc_server( );

		~json_rpc_server( ) = default;
		json_rpc_server( json_rpc_server &&other ) = delete;
		json_rpc_server &operator=( json_rpc_server &&rhs ) = delete;
		json_rpc_server( json_rpc_server const & ) = delete;
		json_rpc_server &operator=( json_rpc_server const & ) = delete;

		/// @brief Start listening for network connections on specified port
		/// @param port Network port to listen for connections on
		json_rpc_server &listen( std::uint16_t port ) &;

		/// @brief Start listening for network connections on specified port/host
		/// @param host Bind to the specified host
		/// @param port Network port to listen for connections on
		json_rpc_server &listen( std::string_view host, std::uint16_t port ) &;

		/// @brief Stop listening for connections and terminate existing connections
		json_rpc_server &stop( ) &;

		json_rpc_server &route_path_to(
		  std::string_view req_path, std::string const &method,
		  std::function<void( crow::request const &, crow::response & )> handler )
		  &;

		json_rpc_server &
		route_path_to( std::string_view req_path_prefix,
		               std::filesystem::path fs_base,
		               std::optional<std::string> default_file = { } ) &;

		template<typename Result, typename Class>
		json_rpc_server &route_path_to( std::string_view req_path,
		                                std::string const &method,
		                                Result Class::*pm, Class &obj ) & {

			return route_path_to(
			  req_path, method,
			  [pm, obj]( crow::request const &req, crow::response &res ) mutable {
				  (void)( obj.*pm )( req, res );
			  } );
		}

		json_rpc_server &route_path_to( std::string_view req_path,
		                                json_rpc_dispatch &dispatcher ) &;

		cookie_t &get_cookie_context( crow::request const &req );
	};
} // namespace daw::json_rpc