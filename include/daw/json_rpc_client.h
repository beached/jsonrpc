// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include <daw/curl_wrapper.h>
#include <daw/daw_fwd_pack_apply.h>
#include <daw/json/daw_json_link.h>

#include <string>
#include <string_view>

namespace daw::json_rpc::details {
	template<typename... Ts>
	struct json_tuple_class : daw::fwd_pack<Ts...> {};
} // namespace daw::json_rpc::details

namespace daw::json {
	template<typename... Ts>
	struct json_data_contract<daw::json_rpc::details::json_tuple_class<Ts...>> {
		using type = json_ordered_member_list<json_link_no_name<Ts...>>;
	};
} // namespace daw::json

namespace daw::json_rpc {
	template<typename Result, typename... Args>
	Result json_rpc_client( std::string const &uri, std::string_view method_name,
	                        std::tuple<Args...> args ) {

		//		auto client = daw::curl_wrapper
		return { };
	}
} // namespace daw::json_rpc
