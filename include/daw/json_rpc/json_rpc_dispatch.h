// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "daw/json_rpc/json_rpc_request_handler.h"

#include <daw/json/daw_json_link.h>

#include <string>
#include <string_view>
#include <utility>

namespace daw::json_rpc {
	class json_rpc_dispatch {
		std::unordered_map<std::string, json_rpc::callback_type> m_handlers{ };

		template<typename... Args, std::size_t... Is>
		inline explicit json_rpc_dispatch( std::tuple<Args...> &&tp_args,
		                                   std::index_sequence<Is...> )
		  : m_handlers{ std::pair<std::string, json_rpc::callback_type>(
		      std::string( std::get<Is * 2>( std::move( tp_args ) ) ),
		      std::get<( Is * 2 + 1 )>( std::move( tp_args ) ) )... } {
			static_assert(
			  ( std::is_constructible_v<std::string,
			                            std::get<Is * 2>( std::move( tp_args ) )> and
			    ... ),
			  "Even arguments must be able to used to construct a string" );
			static_assert(
			  ( std::is_constructible_v<std::string, std::get<Is * 2 + 1>(
			                                           std::move( tp_args ) )> and
			    ... ),
			  "Odd arguments must be able to used to construct a callback_type" );
		}

	public:
		explicit json_rpc_dispatch( ) = default;

		template<typename... Args>
		inline explicit json_rpc_dispatch( Args &&... args )
		  : json_rpc_dispatch(
		      std::tuple<Args...>( std::forward<Args>( args )... ),
		      std::make_index_sequence<( sizeof...( Args ) / 2 )>{ } ) {}

		char *operator( )( std::string const &name, std::string_view json_arguments,
		                   std::optional<std::string_view> id, char *out_it ) const;

		template<typename Result, typename... Args>
		inline void
		add_method( std::string name,
		            json_rpc::request_handler<Result, Args...> &&handler ) {
			m_handlers.insert_or_assign( std::move( name ),
			                             std::move( handler ).callback( ) );
		}

		template<typename Result, typename... Args>
		inline void
		add_method( std::string name,
		            json_rpc::request_handler<Result, Args...> const &handler ) {
			m_handlers.insert_or_assign( std::move( name ), handler.callback( ) );
		}
	};
} // namespace daw::json_rpc