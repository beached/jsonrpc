// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_common.h"
#include "json_rpc_server_request.h"

#include <daw/json/daw_json_link_types.h>

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json {
	inline static constexpr char const mem_jsonrpc[] = "jsonrpc";
	inline static constexpr char const mem_method[] = "method";
	inline static constexpr char const mem_params[] = "params";

	template<>
	struct json_data_contract<daw::json_rpc::details::json_rpc_server_request> {
		using type = json_member_list<json_link<mem_jsonrpc, daw::string_view>,
		                              json_link<mem_method, std::string>,
		                              json_raw_null<mem_params>,
		                              daw::json_rpc::details::id_json_map_type>;

		static inline auto to_json_data(
		  daw::json_rpc::details::json_rpc_server_request const &value ) {
			return std::forward_as_tuple( value.jsonrpc, value.method, value.params,
			                              value.id );
		}
	};

	template<typename... Ts>
	struct json_data_contract<
	  daw::json_rpc::details::json_rpc_client_request<Ts...>> {
		using type = json_member_list<json_link<mem_jsonrpc, daw::string_view>,
		                              json_link<mem_method, std::string>,
		                              json_link<mem_params, std::tuple<Ts...>>,
		                              daw::json_rpc::details::id_json_map_type>;

		static inline constexpr auto to_json_data(
		  daw::json_rpc::details::json_rpc_client_request<Ts...> const &v ) {
			return std::forward_as_tuple( v.jsonrpc, v.method, v.params, v.id );
		}
	};
} // namespace daw::json
