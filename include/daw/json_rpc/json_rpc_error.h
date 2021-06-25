// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <daw/daw_move.h>
#include <daw/json/daw_json_link.h>

#include <optional>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json_rpc::details {
	template<typename Error>
	struct json_rpc_error {
		int code;
		std::string message;
		Error data;
		std::optional<std::variant<double, std::string_view>> id{ };

		inline json_rpc_error(
		  int c, std::string msg, Error d,
		  std::optional<std::variant<double, std::string_view>> i )
		  : code( c )
		  , message( DAW_MOVE( msg ) )
		  , data( DAW_MOVE( d ) )
		  , id( i ) {}

		inline json_rpc_error(
		  std::string_view /*rpc_ver*/, int c, std::string msg, Error d,
		  std::optional<std::variant<double, std::string_view>> i )
		  : json_rpc_error( c, DAW_MOVE( msg ), DAW_MOVE( d ), i ) {}
	};

	template<>
	struct json_rpc_error<void> {
		int code;
		std::string message;
		std::optional<std::variant<double, std::string_view>> id;

		inline json_rpc_error(
		  int c, std::string msg,
		  std::optional<std::variant<double, std::string_view>> i )
		  : code( c )
		  , message( DAW_MOVE( msg ) )
		  , id( i ) {}

		inline json_rpc_error(
		  std::string_view /*rpc_ver*/, int c, std::string msg,
		  std::optional<std::variant<double, std::string_view>> i )
		  : json_rpc_error( c, DAW_MOVE( msg ), i ) {}
	};
} // namespace daw::json_rpc::details
