// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include "json_rpc/json_rpc_request_json.h"
#include "json_rpc/json_rpc_response.h"

#include <daw/curl_wrapper.h>
#include <daw/daw_fwd_pack_apply.h>
#include <daw/json/daw_json_link.h>

#include <string>
#include <string_view>

namespace daw::json_rpc {
	namespace details {
		template<typename T>
		struct client_type_map {
			using type = T;
		};

		template<std::size_t N>
		struct client_type_map<char const[N]> {
			using type = std::string;
		};

		template<std::size_t N>
		struct client_type_map<char[N]> {
			using type = std::string;
		};

		template<typename T>
		using client_type_map_t = typename client_type_map<T>::type;
	} // namespace details
	template<typename Result, typename... Args>
	json_rpc_response<Result> json_rpc_client_with_id(
	  std::string const &uri, std::string const &method_name,
	  std::optional<std::variant<double, std::string>> id, Args const &...args ) {
		auto req = details::json_rpc_client_request(
		  method_name, std::tuple<details::client_type_map_t<Args>...>{ args... },
		  id );
		auto req_json = daw::json::to_json( req );
		auto client = daw::curl_wrapper( );
		client.add_header( "Content-Type", "application/json" );
		client.set_body( req_json );
		auto resp_str = client.get_string( uri );
		return daw::json::from_json<json_rpc_response<Result>>( resp_str );
	}

	template<typename Result, typename... Args>
	json_rpc_response<Result> json_rpc_client( std::string const &uri,
	                                           std::string method_name,
	                                           Args const &...args ) {
		return json_rpc_client_with_id<Result>( uri, method_name, { }, args... );
	}

} // namespace daw::json_rpc
