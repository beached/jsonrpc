// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include "daw/json_rpc_server.h"
#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc/json_rpc_error.h"

#include <daw/json/daw_json_link.h>

#include <httplib.h>
#include <optional>
#include <string_view>
#include <variant>

namespace daw::json_rpc {
	struct json_rpc_server::impl_t {
		httplib::Server server{ };
	};

	struct req_args {
		std::string method;
		std::optional<std::variant<double, std::string_view>> id;
	};
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
	  : m_impl( new impl_t{ } ) {}

	json_rpc_server::~json_rpc_server( ) = default;

	bool json_rpc_server::listen( char const *host, std::uint16_t port,
	                              int socket_flags ) {
		return m_impl->server.listen( host, static_cast<int>( port ),
		                              socket_flags );
	}

	static constexpr char const mem_method[] = "method";
	static constexpr char const mem_id[] = "id";
	void json_rpc_server::add_dispatcher( std::string_view path,
	                                      json_rpc_dispatch &disp ) {
		m_impl->server.Post(
		  path.data( ), path.size( ),
		  [&disp, result = std::string( 1024, '\0' )](
		    httplib::Request const &req, httplib::Response &res ) mutable {
			  using namespace daw::json;
			  req_args args{ };
			  try {
				  try {
					  args = from_json<req_args>( req.body );
				  } catch( daw::json::json_exception const & ) {
					  res.set_content( to_json( details::json_rpc_error<void>(
					                     -32700, "Error handling request", { } ) ),
					                   "application/json" );
				  }
				  disp( DAW_MOVE( args.method ), req.body, { }, result.data( ) );
				  res.set_content( result.c_str( ), "application/json" );
			  } catch( ... ) {
				  res.set_content( to_json( details::json_rpc_error<void>(
				                     -32603, "Error handling request", args.id ) ),
				                   "application/json" );
			  }
		  } );
	}

	bool json_rpc_server::is_running( ) const {
		return m_impl->server.is_running( );
	}

} // namespace daw::json_rpc
