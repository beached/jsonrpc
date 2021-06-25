// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <daw/json/daw_json_link.h>

#include <optional>
#include <string_view>
#include <tuple>

namespace daw::json_rpc::details {
	template<typename Result>
	struct json_rpc_response {
		Result result;
		std::optional<std::string_view> id;

		inline json_rpc_response( Result const &r,
		                          std::optional<std::string_view> i )
		  : result( r )
		  , id( i ) {}

		inline json_rpc_response( Result &&r, std::optional<std::string_view> i )
		  : result( std::move( r ) )
		  , id( i ) {}

		inline json_rpc_response( std::string_view /*rpc_ver*/, Result const &r,
		                          std::optional<std::string_view> i )
		  : json_rpc_response( r, i ) {}

		inline json_rpc_response( std::string_view /*rpc_ver*/, Result &&r,
		                          std::optional<std::string_view> i )
		  : json_rpc_response( std::move( r ), i ) {}
	};
} // namespace daw::json_rpc::details

namespace daw::json {
	template<typename Result>
	struct json_data_contract<daw::json_rpc::details::json_rpc_response<Result>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_result[] = "result";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_link<mem_jsonrpc, std::string_view>, json_link<mem_result, Result>,
		  json_string_raw_null<mem_id, std::optional<std::string_view>>>;

		static inline auto to_json_data(
		  daw::json_rpc::details::json_rpc_response<Result> const &value ) {
			static constexpr std::string_view jsonrpc_ver = "2.0";
			return std::forward_as_tuple( jsonrpc_ver, value.result, value.id );
		}
	};
} // namespace daw::json
