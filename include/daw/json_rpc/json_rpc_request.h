// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_params.h"

#include <daw/daw_move.h>
#include <daw/json/daw_json_link_types.h>

#include <optional>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json_rpc::details {
	struct json_rpc_request {
		std::string_view jsonrpc = "2.0";
		std::string method{ };
		std::optional<daw::json::json_value> params{ };
		std::optional<std::variant<double, std::string_view>> id{ };
	};
} // namespace daw::json_rpc::details

namespace daw::json {
	template<>
	struct json_data_contract<daw::json_rpc::details::json_rpc_request> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_method[] = "method";
		static constexpr char const mem_params[] = "params";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_link<mem_jsonrpc, std::string_view>,
		  json_link<mem_method, std::string>, json_delayed_null<mem_params>,
		  json_variant_null<mem_id,
		                    std::optional<std::variant<double, std::string_view>>>>;

		static inline auto
		to_json_data( daw::json_rpc::details::json_rpc_request const &value ) {
			return std::forward_as_tuple( value.jsonrpc, value.method, value.params,
			                              value.id );
		}
	};
} // namespace daw::json
