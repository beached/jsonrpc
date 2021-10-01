// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include <daw/json/daw_json_link.h>

#include <optional>
#include <string>
#include <variant>

namespace daw::json_rpc::details {
	using req_id_type = std::variant<double, std::string>;
	using id_type = std::optional<req_id_type>;
	static constexpr inline char const id_json_mem_name[] = "id";

	using id_json_map_type = daw::json::json_variant<
	  id_json_mem_name, id_type,
	  daw::json::json_variant_type_list<double, std::string>,
	  daw::json::nullable_constructor<id_type>,
	  daw::json::JsonNullable::NullVisible>;
} // namespace daw::json_rpc::details
