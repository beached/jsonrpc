// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc/json_rpc_error_json.h"

namespace daw::json_rpc {
	struct json_rpc_dispatch::impl_t {
		std::unordered_map<std::string, json_rpc::callback_type> handlers{ };
	};

	void json_rpc_dispatch::operator( )( details::json_rpc_request req,
	                                     std::string &buff ) const {

		if( auto pos = m_impl->handlers.find( req.method );
		    pos != m_impl->handlers.end( ) ) {
			return pos->second( req, buff );
		}
		auto it = std::back_inserter( buff );
		daw::json::to_json(
		  details::json_rpc_error<void>( -32601, "Method not found", req.id ), it );
	}
	void json_rpc_dispatch::add_method( std::string name,
	                                    json_rpc::callback_type &&callback ) {
		m_impl->handlers.insert_or_assign( DAW_MOVE( name ), DAW_MOVE( callback ) );
	}

	json_rpc_dispatch::json_rpc_dispatch( )
	  : m_impl( new impl_t{ } ) {}

	json_rpc_dispatch::~json_rpc_dispatch( ) = default;
} // namespace daw::json_rpc
