// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/json_rpc/json_rpc_error_json.h"

namespace daw::json_rpc {
	void json_rpc_dispatch::operator( )( details::json_rpc_request req,
	                                     std::string &buff ) const {

		if( auto pos = m_handlers.find( req.method ); pos != m_handlers.end( ) ) {
			return pos->second( req, buff );
		}
		auto it = std::back_inserter( buff );
		daw::json::to_json(
		  details::json_rpc_error<void>( -32601, "Method not found", req.id ), it );
	}
} // namespace daw::json_rpc
