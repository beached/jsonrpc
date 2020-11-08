// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_params.h"

#include <daw/json/daw_json_link.h>

#include <optional>
#include <string_view>
#include <tuple>

namespace daw::json_rpc::details {
	template<typename... Params>
	struct json_rpc_request {
		std::string method;
		std::optional<json_rpc_params<Params...>> params;
		std::optional<std::string_view> id;

		inline json_rpc_request( std::string_view /*rpc_ver*/, std::string &&m,
		                         std::optional<json_rpc_params<Params...>> &&p,
		                         std::optional<std::string_view> i )
		  : method( std::move( m ) )
		  , params( std::move( p ) )
		  , id( i ) {}
	};
} // namespace daw::json_rpc::details

namespace daw::json {
	template<typename... Params>
	struct json_data_contract<
	  daw::json_rpc::details::json_rpc_request<Params...>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_method[] = "method";
		static constexpr char const mem_params[] = "params";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_string_raw<mem_jsonrpc>, json_string<mem_method>,
		  json_class_null<
		    mem_params,
		    std::optional<daw::json_rpc::details::json_rpc_params<Params...>>>,
		  json_string_raw_null<mem_id, std::optional<std::string_view>>>;

		static inline auto to_json_data(
		  daw::json_rpc::details::json_rpc_request<Params...> const &value ) {
			static constexpr std::string_view jsonrpc_ver = "2.0";
			return std::forward_as_tuple( jsonrpc_ver, value.method, value.params,
			                              value.id );
		}
	};
} // namespace daw::json
