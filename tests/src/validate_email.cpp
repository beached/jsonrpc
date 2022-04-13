// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#include "validate_email.h"

#include <daw/daw_string_view.h>
#include <daw/daw_utility.h>

#include <boost/regex.hpp>
#include <string>

inline namespace {
	static std::string const email_regex = R"regex(^[^\s]+[^\s]+\.[^\s]+$)regex";
	static auto const email_matcher = boost::regex( email_regex );
} // namespace

bool is_valid_email( daw::string_view addr ) {
	return boost::regex_match( std::data( addr ), daw::data_end( addr ),
	                           email_matcher );
}
