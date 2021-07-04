// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_params.h"
#include "json_rpc_response.h"
#include "json_rpc_server_request.h"

#include <daw/json/daw_json_link.h>
#include <daw/json/daw_json_value_state.h>

#include <functional>
#include <optional>
#include <string_view>

namespace daw::json_rpc {
	using callback_type =
	  std::function<void( details::json_rpc_server_request, std::string & )>;

	template<typename Result, typename... Parameters, typename Callback>
	callback_type make_callback( Callback &&c ) {
		return [c = DAW_FWD( c )]( details::json_rpc_server_request req,
		                           std::string &buff ) -> void {
			auto it = std::back_inserter( buff );
			if( not req.params and sizeof...( Parameters ) != 0 ) {
				daw::json::to_json(
				  json_rpc_response_error( { -32602, "Invalid params" }, req.id ), it );
				return;
			}
			try {
				if constexpr( sizeof...( Parameters ) == 0 ) {
					if( not req.params ) {
						daw::json::to_json(
						  json_rpc_response_result<Result>( c( ), req.id ), it );
						return;
					}
				} else {
					if( not req.params ) {
						daw::json::to_json(
						  json_rpc_response_error{ { -32602, "Invalid params" }, req.id },
						  it );
						return;
					}
				}
				auto p = daw::json::from_json<
				  daw::json::json_tuple_no_name<std::tuple<Parameters...>>>(
				  *req.params );
				auto resp =
				  json_rpc_response_result<Result>( std::apply( c, p ), req.id );
				daw::json::to_json( resp, it );
			} catch( daw::json::json_exception const &je ) {
				if( je.reason_type( ) == daw::json::ErrorReason::MissingMemberName ) {
					daw::json::to_json(
					  json_rpc_response_error{ { -32602, "Invalid params" }, req.id },
					  it );
				}
				daw::json::to_json(
				  json_rpc_response_error{ { -32700, "Parse Error" }, req.id }, it );
			}
		};
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

		inline void operator( )( details::json_rpc_server_request req,
		                         std::string &buff ) const {
			m_callback( DAW_MOVE( req ), buff );
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

	template<typename...>
	inline constexpr bool is_request_handler_v = false;

	template<typename Result, typename... Args>
	inline constexpr bool is_request_handler_v<request_handler<Result, Args...>> =
	  true;
} // namespace daw::json_rpc
