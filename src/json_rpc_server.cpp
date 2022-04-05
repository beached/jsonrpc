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
#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_move.h>
#include <daw/json/daw_json_link.h>

#include <algorithm>
#include <crow/http_request.h>
#include <crow/http_response.h>
#include <crow/middlewares/cookie_parser.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

namespace daw::json_rpc {
	inline namespace {
		struct impl_t {
			crow::App<crow::CookieParser> server{ };
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
	  std::string_view req_path, std::string const &method,
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

	inline bool is_base_of( std::filesystem::path const &base,
	                        std::filesystem::path const &path ) {
		assert( is_directory( base ) );
		return std::mismatch( base.begin( ), base.end( ), path.begin( ),
		                      path.end( ) )
		         .first == base.end( );
	} // namespace

	json_rpc_server &
	json_rpc_server::route_path_to( std::string_view req_path_prefix,
	                                std::filesystem::path fs_base,
	                                std::optional<std::string> default_file ) & {
		assert( fs_base != std::filesystem::path( ) );
		assert( exists( fs_base ) and is_directory( fs_base ) );
		fs_base = canonical( fs_base );
		get_ref( m_storage )
		  .server.catchall_route( )( [=]( crow::request const &req,
		                                  crow::response &res ) -> void {
			  if( req.method != crow::HTTPMethod::GET ) {
				  res = crow::response( 404 );
				  return;
			  }
			  try {
				  auto rel_path =
				    std::string_view( req.url ).substr( req_path_prefix.size( ) );
				  if( rel_path.starts_with( '/' ) ) {
					  rel_path.remove_prefix( 1 );
				  }
				  auto fs_path = fs_base / rel_path;
				  if( not exists( fs_path ) ) {
					  res = crow::response( 404 );
					  return;
				  }
				  fs_path = canonical( fs_path );
				  if( not is_base_of( fs_base, fs_path ) ) {
					  res = crow::response( 404 );
				  } else if( is_regular_file( fs_path ) ) {
					  res.set_static_file_info( static_cast<std::string>( fs_path ) );
					  auto mmf = daw::filesystem::memory_mapped_file_t(
					    static_cast<std::string>( fs_path ) );
					  res.body = std::string( std::data( mmf ), daw::data_end( mmf ) );
				  } else if( default_file and is_directory( fs_path ) ) {
					  auto f = fs_path / ( *default_file );
					  if( exists( f ) ) {
						  res.set_static_file_info( f );
						  auto mmf = daw::filesystem::memory_mapped_file_t(
						    static_cast<std::string>( f ) );
						  res.body = std::string( std::data( mmf ), daw::data_end( mmf ) );
					  } else {
						  res = crow::response( 404 );
					  }
				  } else {
					  res = crow::response( 404 );
				  }
			  } catch( std::exception const &ex ) {
				  std::cerr << "Exception while processing file request: " << ex.what( )
				            << '\n';
				  res = crow::response( 500 );
			  }
		  } );
		return *this;
	}

	json_rpc_server &json_rpc_server::stop( ) & {
		get_ref( m_storage ).server.stop( );
		return *this;
	}

	json_rpc_server::cookie_t &
	json_rpc_server::get_cookie_context( crow::request const &req ) {
		return get_ref( m_storage ).server.get_context<crow::CookieParser>( req );
	}
} // namespace daw::json_rpc
