//
// Created by Darrell Wright on 2020-09-28.
//

// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <daw/json/daw_json_link.h>
#include <tuple>

namespace daw::json_rpc::details {
	template<typename... Params>
	struct json_rpc_params {
		std::tuple<Params...> params;

		template<typename... Ps>
		constexpr json_rpc_params( Ps &&...ps )
		  : params{ DAW_FWD( ps )... } {}
	};
	template<typename... Ps>
	json_rpc_params( Ps &&... ) -> json_rpc_params<Ps...>;
} // namespace daw::json_rpc::details

namespace daw::json {
	template<typename... Params>
	struct json_data_contract<
	  daw::json_rpc::details::json_rpc_params<Params...>> {
		using type = json_ordered_member_list<Params...>;

		[[nodiscard, maybe_unused]] static inline auto const &to_json_data(
		  daw::json_rpc::details::json_rpc_params<Params...> const &value ) {
			return value.params;
		}
	};
} // namespace daw::json
