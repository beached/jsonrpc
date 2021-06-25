// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include "daw/json_rpc_server.h"

#include <cstdint>
#include <ctre.hpp>
#include <string_view>
#include <tuple>

struct User {
	std::string id;
	std::string email;
	std::string name;
	std::string password;
};

static constexpr std::string_view email_regex =
  R"regex(^[^\s]+[^\s]+\.[^\s]+$)regex";

static constexpr auto email_matcher = ctre::match<email_regex>;

constexpr bool validate_email( std::string_view addr ) {
	return email_matcher.match( addr );
}

std::string_view validate( User const &u ) {
	if( not validate_email( u.email ) ) {
		return "invalid email";
	}

	if( u.name.size( ) < 4 ) {
		return "Name is too short";
	}

	if( u.password.size( ) < 4 ) {
		return "Password is too short";
	}
	return { };
}

struct Response {
	std::string message;
	std::int32_t code;
	User user;
};

namespace daw::json {
	template<>
	struct json_data_contract<User> {
		static constexpr char const id[] = "id";
		static constexpr char const email[] = "email";
		static constexpr char const name[] = "name";
		static constexpr char const password[] = "password";

		using type = json_member_list<
		  json_link<id, std::string>, json_link<email, std::string>,
		  json_link<name, std::string>, json_link<password, std::string>>;

		static inline auto to_json_data( User const &u ) {
			return std::forward_as_tuple( u.id, u.email, u.name, u.password );
		}
	};

	template<>
	struct json_data_contract<Response> {
		static constexpr char const message[] = "message";
		static constexpr char const code[] = "code";
		static constexpr char const user[] = "user";

		using type =
		  json_member_list<json_link<message, std::string>,
		                   json_link<code, std::int32_t>, json_link<user, User>>;

		static inline auto to_json_data( Response const &r ) {
			return std::forward_as_tuple( r.message, r.code, r.user );
		}
	};
} // namespace daw::json

int main( ) {
	auto server = daw::json_rpc::json_rpc_server( );
	auto dispatcher = daw::json_rpc::json_rpc_dispatch( );
	dispatcher.add_method( "CreateUser", [&]( User u ) {
		if( auto m = validate( u ); not m.empty( ) ) {
			throw std::runtime_error( static_cast<std::string>( m ) );
		}
		u.id = "1000000";
		return DAW_MOVE( u );
	} );

	server.add_dispatcher( "/", dispatcher );
	server.listen( 1234 );
}
