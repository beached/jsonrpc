// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_error.h"
#include "json_rpc_params.h"
#include "json_rpc_response.h"

#include <daw/json/daw_json_link.h>
#include <daw/json/daw_json_value_state.h>

#include <functional>
#include <optional>
#include <string_view>

namespace daw::json_rpc {
	using callback_type = std::function<char *(
	  daw::json::json_value_state, std::optional<std::string_view>, char * )>;

	template<typename Result, typename... Parameters, typename Callback>
	callback_type make_callback( Callback &&c ) {
		callback_type result =
		  [c = std::forward<Callback>( c )]( daw::json::json_value_state jv,
		                                     std::optional<std::string_view> id,
		                                     char *out_it ) -> char * {
			auto param_idx = jv.index_of( "params" );
			if constexpr( sizeof...( Parameters ) > 0 ) {
				if( param_idx >= jv.size( ) ) {
					return daw::json::to_json(
					  details::json_rpc_error<void>( -32602, "Invalid params", id ),
					  out_it );
				}
			}
			try {
				return daw::json::to_json(
				  details::json_rpc_response<Result>(
				    std::apply(
				      c, daw::json::from_json<details::json_rpc_params<Parameters...>>(
				           jv[param_idx] )
				           .params ),
				    id ),
				  out_it );
			} catch( daw::json::json_exception const &je ) {
				if( je.reason_type( ) == daw::json::ErrorReason::MissingMemberName ) {
					return daw::json::to_json(
					  details::json_rpc_error<void>( -32602, "Invalid params", id ),
					  out_it );
				}
				return daw::json::to_json(
				  details::json_rpc_error<void>( -32700, "Parse Error", id ), out_it );
			}
		};
		return result;
	}

	template<typename Result, typename... Args>
	class request_handler;

	template<typename Result, typename... Args>
	class request_handler<Result( Args... )> {
		callback_type m_callback;

	public:
		template<typename Callback>
		request_handler( Callback &&cb )
		  : m_callback(
		      make_callback<Result, Args...>( std::forward<Callback>( cb ) ) ) {}

		request_handler( Result ( *cb )( Args... ) )
		  : m_callback( make_callback<Result, Args...>( cb ) ) {}

		inline char *operator( )( std::string_view json_arguments,
		                          std::optional<std::string_view> id,
		                          char *out_it ) const {
			return m_callback( json_arguments, id, out_it );
		}

		callback_type &callback( ) & {
			return m_callback;
		}

		callback_type const &callback( ) const & {
			return m_callback;
		}

		callback_type callback( ) && {
			return m_callback;
		}
	};

	template<typename Result, typename... Args>
	request_handler( Result ( * )( Args... ) )
	  -> request_handler<Result( Args... )>;
} // namespace daw::json_rpc
