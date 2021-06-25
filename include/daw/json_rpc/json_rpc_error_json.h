// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_error.h"

#include <daw/json/daw_json_link_types.h>

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json {
	template<typename Error>
	struct json_data_contract<daw::json_rpc::details::json_rpc_error<Error>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_code[] = "code";
		static constexpr char const mem_message[] = "message";
		static constexpr char const mem_data[] = "data";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_link<mem_jsonrpc, std::string_view>, json_link<mem_code, int>,
		  json_link<mem_message, std::string>, json_link<mem_data, Error>,
		  json_variant_null<mem_id,
		                    std::optional<std::variant<double, std::string_view>>>>;

		static inline auto
		to_json_data( daw::json_rpc::details::json_rpc_error<Error> const &value ) {
			static constexpr std::string_view jsonrpc_ver = "2.0";
			return std::forward_as_tuple( jsonrpc_ver, value.code, value.message,
			                              value.data, value.id );
		}
	};

	template<>
	struct json_data_contract<daw::json_rpc::details::json_rpc_error<void>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_code[] = "code";
		static constexpr char const mem_message[] = "message";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_link<mem_jsonrpc, std::string_view>, json_link<mem_code, int>,
		  json_link<mem_message, std::string>,
		  json_variant_null<mem_id,
		                    std::optional<std::variant<double, std::string_view>>>>;

		static inline auto
		to_json_data( daw::json_rpc::details::json_rpc_error<void> const &value ) {
			static constexpr std::string_view jsonrpc_ver = "2.0";
			return std::forward_as_tuple( jsonrpc_ver, value.code, value.message,
			                              value.id );
		}
	};
} // namespace daw::json
