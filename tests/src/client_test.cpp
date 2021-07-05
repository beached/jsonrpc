// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include <daw/json_rpc_client.h>

#include <iostream>

int main( ) {
	auto r = daw::json_rpc::json_rpc_client<int>( "http://127.0.0.1:1234/", "add",
	                                              1, 2 );
	if( r.has_error( ) ) {
		throw r.error( );
	}
	std::cout << r.result( ) << '\n';
}
