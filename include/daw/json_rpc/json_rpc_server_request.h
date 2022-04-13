// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_common.h"
#include "json_rpc_params.h"

#include <daw/daw_move.h>
#include <daw/json/daw_json_link_types.h>

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json_rpc::details {
	// This is used by the server to two step the parsing process.
	// First it parses enough to determine the method requested and then passes
	// the params to the handler to parse
	struct json_rpc_server_request {
		daw::string_view jsonrpc = "2.0";
		std::string method{ };
		std::optional<daw::json::json_value<>> params{ };
		details::id_type id{ };
	};

	// This is the client request sent to the server to process
	template<typename... Ts>
	struct json_rpc_client_request {
		daw::string_view jsonrpc = "2.0";
		std::string method{ };
		std::tuple<Ts...> params;
		details::id_type id{ };

		json_rpc_client_request(
		  daw::string_view Method, std::tuple<Ts...> args,
		  std::optional<std::variant<double, std::string>> Id = { } )
		  : method( static_cast<std::string>( Method ) )
		  , params{ DAW_MOVE( args ) }
		  , id( DAW_MOVE( Id ) ) {}

		/// Constructor used by serialization library
		json_rpc_client_request(
		  daw::string_view jsonRpc, daw::string_view Method,
		  std::tuple<Ts...> Params,
		  std::optional<std::variant<double, std::string>> Id )
		  : jsonrpc( jsonRpc )
		  , method( static_cast<std::string>( Method ) )
		  , params( DAW_MOVE( Params ) )
		  , id( DAW_MOVE( Id ) ) {}
	};
	template<typename... Ts>
	json_rpc_client_request( daw::string_view, std::tuple<Ts...>,
	                         details::id_type ) -> json_rpc_client_request<Ts...>;
} // namespace daw::json_rpc::details
