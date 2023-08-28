// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "daw/daw_function_traits.h"
#include "daw/json_rpc/json_rpc_request_handler.h"
#include "daw/json_rpc/json_rpc_server_request.h"

#include <daw/json/daw_json_link.h>
#include <daw/daw_move.h>

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace daw::json_rpc {
	namespace impl {
		template<typename Result, typename... Args>
		struct make_handler;

		template<typename Result, typename... Args>
		struct make_handler<Result, daw::fwd_pack<Args...>> {
			using handler = json_rpc::request_handler<Result( Args... )>;
		};
	} // namespace impl

	struct json_rpc_dispatch {
		using storage_t = std::aligned_storage_t<64, 64>;

	private:
		storage_t m_storage{ };

		void add_method( std::string, json_rpc::callback_type && );

		struct deduce_signature;

	public:
		explicit json_rpc_dispatch( );
		json_rpc_dispatch( json_rpc_dispatch &&other ) noexcept( false );
		json_rpc_dispatch( json_rpc_dispatch const &other ) noexcept( false );
		json_rpc_dispatch &operator=( json_rpc_dispatch &&rhs ) noexcept( false );
		json_rpc_dispatch &
		operator=( json_rpc_dispatch const &rhs ) noexcept( false );
		~json_rpc_dispatch( );

		void operator( )( details::json_rpc_server_request const &req,
		                  std::string &buff ) const;

		template<typename Sig = deduce_signature, typename Handler,
		         typename std::enable_if_t<
		           not is_request_handler_v<daw::remove_cvref_t<Handler>>,
		           std::nullptr_t> = nullptr>
		inline json_rpc_dispatch &add_method( std::string name,
		                                      Handler &&handler ) & {

			using handler_t = daw::remove_cvref_t<Handler>;
			if constexpr( is_request_handler_v<handler_t> ) {
				add_method( std::move( name ), DAW_FWD( handler ).callback( ) );
			} else if constexpr( std::is_same_v<Sig, deduce_signature> ) {
				using func_t = daw::func::function_traits<handler_t>;
				using make_handler_t = impl::make_handler<typename func_t::result_t,
				                                          typename func_t::params_t>;
				using req_handler_t = typename make_handler_t::handler;
				add_method( std::move( name ),
				            req_handler_t( DAW_FWD( handler ) ).callback( ) );
			} else /* explicitly specified signature */ {
				add_method( std::move( name ),
				            request_handler<Sig>( DAW_FWD( handler ) ).callback( ) );
			}
			return *this;
		}
	};
} // namespace daw::json_rpc