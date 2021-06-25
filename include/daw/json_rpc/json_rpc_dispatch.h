// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "daw/daw_function_traits.h"
#include "daw/json_rpc/json_rpc_request_handler.h"

#include <daw/daw_move.h>
#include <daw/json/daw_json_link.h>

#include <string>
#include <string_view>
#include <utility>

namespace daw::json_rpc {
	namespace impl {
		template<typename, typename...>
		struct make_handler;

		template<typename Result, typename... Args>
		struct make_handler<Result, daw::fwd_pack<Args...>> {
			using handler = json_rpc::request_handler<Result( Args... )>;
		};
	} // namespace impl
	class json_rpc_dispatch {
		std::unordered_map<std::string, json_rpc::callback_type> m_handlers{ };

		template<std::size_t Base, typename... Args>
		static inline std::pair<std::string, json_rpc::callback_type>
		to_node( std::tuple<Args...> &&tp_args ) {
			return { std::string( std::get<Base + 0>( DAW_MOVE( tp_args ) ) ),
			         std::get<( Base + 1 )>( DAW_MOVE( tp_args ) ).callback( ) };
		}

		template<typename... Args, std::size_t... Is>
		inline explicit json_rpc_dispatch( std::tuple<Args...> &&tp_args,
		                                   std::index_sequence<Is...> )
		  : m_handlers{ { to_node<Is * 2>( DAW_MOVE( tp_args ) )... } } {
			static_assert( sizeof...( Args ) % 2 == 0,
			               "Must supply name/handler pairs" );
			static_assert(
			  ( std::is_constructible_v<std::string, decltype( std::get<( Is * 2 )>(
			                                           DAW_MOVE( tp_args ) ) )> and
			    ... ),
			  "Even arguments must be able to used to construct a string" );
			static_assert(
			  ( std::is_constructible_v<json_rpc::callback_type,
			                            decltype( std::get<( Is * 2 ) + 1>(
			                                        DAW_MOVE( tp_args ) )
			                                        .callback( ) )> and
			    ... ),
			  "Odd arguments must be able to used to construct a callback_type" );
		}

	public:
		explicit json_rpc_dispatch( ) = default;

		template<typename... Args>
		inline explicit json_rpc_dispatch( Args &&...args )
		  : json_rpc_dispatch(
		      std::tuple<Args...>( DAW_FWD( args )... ),
		      std::make_index_sequence<( sizeof...( Args ) / 2 )>{ } ) {}

		void operator( )( std::string const &name, std::string_view json_arguments,
		                   std::optional<std::string_view> id,
		                   std::string &buff ) const;

		struct deduced_signature {};

		template<typename Sig = deduced_signature, typename Handler>
		inline void add_method( std::string name, Handler &&handler ) {
			using handler_t = daw::remove_cvref_t<Handler>;
			if constexpr( is_request_handler_v<handler_t> ) {
				m_handlers.insert_or_assign( DAW_MOVE( name ),
				                             DAW_FWD( handler ).callback( ) );
			} else if constexpr( std::is_same_v<Sig, deduced_signature> ) {
				using func_t = daw::func::function_traits<handler_t>;
				using make_handler_t = impl::make_handler<typename func_t::result_t,
				                                          typename func_t::params_t>;
				using req_handler_t = typename make_handler_t::handler;
				m_handlers.template insert_or_assign(
				  DAW_MOVE( name ), req_handler_t( DAW_FWD( handler ) ).callback( ) );
			} else {
				m_handlers.insert_or_assign(
				  DAW_MOVE( name ),
				  request_handler<Sig>( DAW_FWD( handler ) ).callback( ) );
			}
		}
	};
} // namespace daw::json_rpc