// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc/json_rpc_dispatch.h"

namespace daw::json_rpc {
	char *json_rpc_dispatch::operator( )( std::string const &name,
	                                      std::string_view json_arguments,
	                                      std::optional<std::string_view> id,
	                                      char *out_it ) const {

		if( auto pos = m_handlers.find( name ); pos != m_handlers.end( ) ) {
			return pos->second( json_arguments, id, out_it );
		}
		return daw::json::to_json(
		  details::json_rpc_error<void>( -32601, "Method not found", id ), out_it );
	}
} // namespace daw::json_rpc
