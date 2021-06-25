// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include "daw/json_rpc_server.h"

#include <iostream>
#include <string_view>

int add( int a, int b ) {
	return a + b;
}

struct Foo {
	std::string bar;
	double d;
};

namespace daw::json {
	template<>
	struct json_data_contract<Foo> {
		static constexpr char const bar[] = "bar";
		static constexpr char const d[] = "d";
		using type =
		  json_member_list<json_link<bar, std::string>, json_link<d, double>>;
	};
} // namespace daw::json

// { "jsonrpc": "2.0", "method": "mul", "params": [45,22] }

int main( ) {
	auto server = daw::json_rpc::json_rpc_server( );
	auto dispatcher = daw::json_rpc::json_rpc_dispatch( );
	dispatcher.add_method( "mul", []( int l, int r ) { return l * r; } );
	dispatcher.add_method( "add", add );
	dispatcher.add_method( "status", []( Foo f ) {
		std::cout << "Hello" << f.bar << '\n';
		return f.d;
	} );

	server.add_dispatcher( "/rpc", dispatcher );
	server.listen( 1234 );
}
