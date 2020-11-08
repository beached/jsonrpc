#include <iostream>

#include "daw/json_rpc/json_rpc_dispatch.h"

#include <iostream>
#include <string>
#include <string_view>

int main( ) {
	auto srv = daw::json_rpc::json_rpc_dispatch(
	  "add", daw::json_rpc::request_handler<int( int, int )>(
	           []( int l, int r ) { return l + r; } ) );
	constexpr std::string_view json_req =
	  R"({ "jsonrpc": "2.0", "method": "add", "params": [1,2] })";
	std::cout << "req: " << json_req << '\n';
	auto resp = std::string( 1024, '\0' );

	srv( "add", json_req, { }, resp.data( ) );
	std::cout << "resp: " << resp << '\n';

	return 0;
}
