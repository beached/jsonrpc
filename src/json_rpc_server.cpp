// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//
#define CROW_MAIN

#include "daw/json_rpc_server.h"
#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc/json_rpc_error.h"

#include <daw/json/daw_json_link.h>

#include <crow/crow_all.h>
#include <optional>
#include <string_view>
#include <variant>

namespace daw::json_rpc {
	struct json_rpc_server::impl_t {
		crow::SimpleApp server{ };
	};

	inline namespace {
		struct req_args {
			std::string method;
			std::optional<std::variant<double, std::string_view>> id;
		};
	} // namespace
} // namespace daw::json_rpc

namespace daw::json {
	template<>
	struct json_data_contract<daw::json_rpc::req_args> {
		static constexpr char const mem_method[] = "method";
		static constexpr char const mem_id[] = "id";
		using type = json_member_list<
		  json_string<mem_method>,
		  json_variant_null<mem_id,
		                    std::optional<std::variant<double, std::string_view>>>>;
	};
} // namespace daw::json

namespace daw::json_rpc {
	json_rpc_server::json_rpc_server( )
	  : m_impl( new impl_t{ } ) {

		m_impl->server.loglevel( crow::LogLevel::Warning );
	}

	json_rpc_server::~json_rpc_server( ) = default;

	void json_rpc_server::listen( std::uint16_t port ) {
		m_impl->server.port( port ).multithreaded( ).run( );
	}

	void json_rpc_server::listen( std::string_view host, std::uint16_t port ) {
		m_impl->server.bindaddr( static_cast<std::string>( host ) )
		  .port( port )
		  .multithreaded( )
		  .run( );
	}

	inline namespace {
		static constexpr char const mem_method[] = "method";
		static constexpr char const mem_id[] = "id";
	} // namespace
	void json_rpc_server::add_dispatcher( std::string_view path,
	                                      json_rpc_dispatch &disp ) {

		static thread_local auto result = std::string{ };
		m_impl->server.route_dynamic( static_cast<std::string>( path ) )
		  .methods( "POST"_method )(
		    [&]( crow::request const &req, crow::response &res ) {
			    using namespace daw::json;
			    req_args args{ };
			    try {
				    try {
					    args = from_json<req_args>( req.body );
				    } catch( daw::json::json_exception const & ) {
					    auto it = std::back_inserter( result );
					    (void)to_json( details::json_rpc_error<void>(
					                     -32700, "Error handling request", { } ),
					                   it );
					    res.add_header( "Content-Type", "application/json" );
					    res.code = 400;
					    res.write( result );
				    }
				    disp( DAW_MOVE( args.method ), req.body, { }, result );
				    res.add_header( "Content-Type", "application/json" );
				    res.write( result );
			    } catch( ... ) {
				    auto it = std::back_inserter( result );
				    (void)to_json( details::json_rpc_error<void>(
				                     -32603, "Error handling request", { } ),
				                   it );
				    res.add_header( "Content-Type", "application/json" );
				    res.code = 500;
				    res.write( result );
			    }
			    res.end( );
		    } );
	}
} // namespace daw::json_rpc
