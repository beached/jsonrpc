// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include "daw/json_rpc/validate_email.h"

#include <ctre.hpp>

#include <string_view>

inline namespace {
	static inline constexpr std::string_view email_regex =
	  R"regex(^[^\s]+[^\s]+\.[^\s]+$)regex";

	static inline constexpr auto email_matcher = ctre::match<email_regex>;
} // namespace

bool is_valid_email( std::string_view addr ) {
	return email_matcher.match( addr );
}
