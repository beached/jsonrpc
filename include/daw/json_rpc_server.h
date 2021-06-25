// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include "json_rpc/json_rpc_dispatch.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace daw::json_rpc {
	class json_rpc_server {
		struct impl_t;
		std::unique_ptr<impl_t> m_impl;

	public:
		json_rpc_server( );
		~json_rpc_server( );

		json_rpc_server( json_rpc_server const & ) = delete;
		json_rpc_server &operator=( json_rpc_server const & ) = delete;
		json_rpc_server( json_rpc_server && ) noexcept = default;
		json_rpc_server &operator=( json_rpc_server && ) noexcept = default;

		bool listen( char const *host, std::uint16_t port, int socket_flags = 0 );
		void stop( );
		bool is_running( ) const;

		void add_dispatcher( std::string_view path, json_rpc_dispatch &disp );
	};
} // namespace daw::json_rpc