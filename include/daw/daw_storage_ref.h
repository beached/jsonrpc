// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include <utility>

namespace daw {
	template<typename T, typename storage_t>
	struct storage_ref {
		T &operator( )( storage_t &storage ) const {
			return *std::launder( reinterpret_cast<T *>( &storage ) );
		}

		T &&operator( )( storage_t &&storage ) const {
			return std::move( *std::launder( reinterpret_cast<T *>( &storage ) ) );
		}

		T const &operator( )( storage_t const &storage ) const {
			return *std::launder( reinterpret_cast<T const *>( &storage ) );
		}

		T const &&operator( )( storage_t const &&storage ) const {
			return std::move(
			  *std::launder( reinterpret_cast<T const *>( &storage ) ) );
		}
	};
} // namespace daw
