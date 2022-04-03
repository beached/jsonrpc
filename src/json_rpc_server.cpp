// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//
#define CROW_MAIN

#include "daw/json_rpc_server.h"
#include "crow/app.h"
#include "crow/http_request.h"
#include "crow/http_response.h"
#include "daw/daw_storage_ref.h"
#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc/json_rpc_request_json.h"
#include "daw/json_rpc/json_rpc_server_request.h"

#include <daw/daw_construct_at.h>
#include <daw/daw_move.h>
#include <daw/json/daw_json_link.h>

#include <boost/filesystem.hpp>
#include <memory>
#include <optional>
#include <string_view>

namespace daw::json_rpc {
	inline namespace {
		struct impl_t {
			crow::SimpleApp server{ };
		};

		inline constexpr auto get_ref =
		  daw::storage_ref<impl_t, json_rpc_server::storage_t>{ };
	} // namespace

	json_rpc_server::json_rpc_server( ) {
		static_assert( sizeof( impl_t ) <= sizeof( storage_t ) );
		daw::construct_at<impl_t>( &m_storage );

#if defined( NDEBUG )
		get_ref( m_storage ).server.loglevel( crow::LogLevel::Warning );
#endif
	}

	json_rpc_server::~json_rpc_server( ) {
		std::destroy_at( &get_ref( m_storage ) );
	}

	json_rpc_server &json_rpc_server::listen( std::uint16_t port ) & {
		get_ref( m_storage ).server.port( port ).multithreaded( ).run( );
		return *this;
	}

	json_rpc_server &json_rpc_server::listen( std::string_view host,
	                                          std::uint16_t port ) & {
		get_ref( m_storage )
		  .server.bindaddr( static_cast<std::string>( host ) )
		  .port( port )
		  .multithreaded( )
		  .run( );
		return *this;
	}

	json_rpc_server &json_rpc_server::route_path_to(
	  std::string_view req_path, std::string_view method,
	  std::function<void( const crow::request &, crow::response & )> handler ) & {

		get_ref( m_storage )
		  .server.route_dynamic( static_cast<std::string>( req_path ) )
		  .methods( operator""_method( method.data( ), method.size( ) ) )(
		    DAW_MOVE( handler ) );
		return *this;
	}

	json_rpc_server &
	json_rpc_server::route_path_to( std::string_view req_path,
	                                json_rpc_dispatch &dispatcher ) & {
		get_ref( m_storage )
		  .server.route_dynamic( static_cast<std::string>( req_path ) )
		  .methods( crow::HTTPMethod::POST )(
		    [&dispatcher]( crow::request const &req, crow::response &res ) {
			    using namespace daw::json;

			    auto args = details::json_rpc_server_request{ };
			    try {
				    try {
					    args = from_json<details::json_rpc_server_request>( req.body );
				    } catch( daw::json::json_exception const & ) {
					    auto it = std::back_inserter( res.body );
					    (void)to_json( json_rpc_response_error(
					                     Error( -32700, "Error handling request" ) ),
					                   it );
					    res.code = 400;
				    }
				    dispatcher( args, res.body );
				    if( args.id ) {
					    res.add_header( "Content-Type", "application/json" );
				    } else {
					    res.body.clear( );
				    }
			    } catch( ... ) {
				    auto it = std::back_inserter( res.body );
				    (void)to_json( json_rpc_response_error(
				                     Error( -32603, "Error handling request" ) ),
				                   it );
				    res.code = 500;
			    }
			    res.end( );
		    } );
		return *this;
	}

	json_rpc_server &json_rpc_server::stop( ) & {
		get_ref( m_storage ).server.stop( );
		return *this;
	}

	json_rpc_server &json_rpc_server::route_path_to(
	  std::string_view req_path, const std::string &/*fs_path*/,
	  std::optional<std::string_view> /*default_resource*/ ) & {

		get_ref( m_storage )
		  .server.route_dynamic( static_cast<std::string>( req_path ) )
		  .methods( crow::HTTPMethod::GET )(
		    [&]( crow::request const & /*req*/, crow::response & /*res*/ ) {
			    using namespace daw::json;
		    } );
		return *this;
	}

} // namespace daw::json_rpc
