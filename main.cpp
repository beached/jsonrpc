#include <iostream>

#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc_server/json_rpc_server_listener.h"

#include <boost/asio.hpp>
#include <string_view>

int add( int a, int b ) {
	return a + b;
}

int main( ) {
	auto srv = daw::json_rpc::json_rpc_dispatch( );
	srv.add_method( "mul", []( int l, int r ) { return l * r; } );
	srv.add_method( "add", add );

	/*
	constexpr std::string_view json_req =
	  R"({ "jsonrpc": "2.0", "method": "mul", "params": [45,22] })";
	std::cout << "req: " << json_req << '\n';
	auto resp = std::string( 1024, '\0' );

	srv( "mul", json_req, { }, resp.data( ) );
	std::cout << "resp: " << resp.c_str( ) << '\n';
*/

	auto ioctx = boost::asio::io_context( 1 );
	auto const address = boost::asio::ip::make_address( "0.0.0.0" );
	std::uint16_t port = 12345U;
	auto endpoint = boost::asio::ip::tcp::endpoint( address, port );
	auto ctx = boost::asio::ssl::context{ boost::asio::ssl::context::tlsv12 };
	auto doc_root = std::make_shared<std::string>( "/" );
	auto listener =
	  std::make_shared<daw::json_rpc_server::json_rpc_server_listener>(
	    ioctx, ctx, endpoint, doc_root );

	listener->run( );
	return 0;
}
