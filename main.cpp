#include <iostream>

#include "daw/json_rpc/json_rpc_dispatch.h"

#include <string_view>

int main( ) {
	auto srv = daw::json_rpc::json_rpc_dispatch(
	  "mul", daw::json_rpc::request_handler<int( int, int )>(
	           []( int l, int r ) { return l * r; } ) );
	constexpr std::string_view json_req =
	  R"({ "jsonrpc": "2.0", "method": "mul", "params": [45,22] })";
	std::cout << "req: " << json_req << '\n';
	auto resp = std::string( 1024, '\0' );

	srv( "mul", json_req, { }, resp.data( ) );
	std::cout << "resp: " << resp << '\n';

	return 0;
}
