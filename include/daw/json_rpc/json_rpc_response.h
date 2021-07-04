// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_common.h"

#include <daw/json/daw_json_link.h>

#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <variant>

namespace daw::json_rpc {
	struct json_rpc_response_error {
		struct Error {
			int code;
			std::optional<std::string> message;
			std::optional<daw::json::json_value> data;
		};

		std::string jsonrpc;
		Error error;
		details::id_type id;

		json_rpc_response_error( Error err, details::id_type Id = { } )
		  : error( DAW_MOVE( err ) )
		  , id( DAW_MOVE( id ) ) {}

		json_rpc_response_error( std::string &&JsonRPC, Error &&err,
		                         details::id_type &&Id )
		  : jsonrpc( DAW_MOVE( JsonRPC ) )
		  , error( DAW_MOVE( err ) )
		  , id( DAW_MOVE( id ) ) {}
	};
} // namespace daw::json_rpc

namespace daw::json {
	template<>
	struct json_data_contract<daw::json_rpc::json_rpc_response_error::Error> {
		static constexpr char const mem_code[] = "code";
		static constexpr char const mem_message[] = "message";
		static constexpr char const mem_data[] = "data";
		using type =
		  json_member_list<json_link<mem_code, int>,
		                   json_link<mem_message, std::optional<std::string>>,
		                   json_delayed_null<mem_data>>;

		static inline constexpr auto
		to_json_data( daw::json_rpc::json_rpc_response_error::Error const &v ) {
			return std::forward_as_tuple( v.code, v.message, v.data );
		}
	};

	template<>
	struct json_data_contract<daw::json_rpc::json_rpc_response_error> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_error[] = "error";
		using type = json_member_list<
		  json_link<mem_jsonrpc, std::string>,
		  json_link<mem_error, daw::json_rpc::json_rpc_response_error::Error>,
		  daw::json_rpc::details::id_json_map_type>;

		static inline auto
		to_json_data( daw::json_rpc::json_rpc_response_error const &v ) {
			static constexpr std::string_view jsonrpc = "2.0";
			return std::forward_as_tuple( jsonrpc, v.error, v.id );
		}
	};
} // namespace daw::json

namespace daw::json_rpc {
	template<typename Result>
	struct json_rpc_response_result {
		Result result{ };
		details::id_type id;

		template<typename R, std::enable_if_t<std::is_convertible_v<R, Result>,
		                                      std::nullptr_t> = nullptr>
		inline json_rpc_response_result( R &&result, details::id_type Id )
		  : result( DAW_FWD( result ) )
		  , id( DAW_MOVE( Id ) ) {}

		inline json_rpc_response_result( std::string_view /*rpc_ver*/, Result &&r,
		                                 details::id_type &&Id )
		  : result( DAW_MOVE( r ) )
		  , id( DAW_MOVE( Id ) ) {}
	};
	template<typename Result>
	json_rpc_response_result( Result,
	                          std::optional<std::variant<double, std::string>> )
	  -> json_rpc_response_result<Result>;

} // namespace daw::json_rpc
namespace daw::json {
	template<typename Result>
	struct json_data_contract<daw::json_rpc::json_rpc_response_result<Result>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_result[] = "result";

		using type = json_member_list<json_link<mem_jsonrpc, std::string_view>,
		                              json_link<mem_result, Result>,
		                              daw::json_rpc::details::id_json_map_type>;

		static inline auto to_json_data(
		  daw::json_rpc::json_rpc_response_result<Result> const &value ) {
			static constexpr std::string_view jsonrpc = "2.0";
			return std::forward_as_tuple( jsonrpc, value.result, value.id );
		}
	};
} // namespace daw::json

namespace daw::json_rpc::json_rpc_response_details {
	struct response_switcher {
		constexpr std::size_t
		operator( )( std::optional<daw::json::json_value> const &v ) const {
			if( v ) {
				// There is a error member, so we are the error alternative
				return 1;
			}
			// Choose result alternative
			return 0;
		}
	};
} // namespace daw::json_rpc::json_rpc_response_details

namespace daw::json_rpc {
	template<typename T>
	class json_rpc_response {
		std::variant<json_rpc_response_result<T>, json_rpc_response_error>
		  m_response;

	public:
		inline json_rpc_response( json_rpc_response_error err )
		  : m_response( DAW_MOVE( err ) ) {}

		inline json_rpc_response( json_rpc_response_result<T> result )
		  : m_response( DAW_MOVE( result ) ) {}

		inline bool has_error( ) const {
			return m_response.index( ) == 1;
		}

		inline bool has_response( ) const {
			return m_response.index( ) == 0;
		}

		inline operator bool( ) const {
			return has_response( );
		}

		inline json_rpc_response_result<T> const &response( ) const & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_result<T> const & {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		inline json_rpc_response_result<T> &response( ) & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_result<T> & {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		inline json_rpc_response_result<T> &&response( ) && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_result<T> && {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		inline json_rpc_response_result<T> const &&response( ) const && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_result<T> const && {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		inline json_rpc_response_error const &error( ) const & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_error const & {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		inline json_rpc_response_error &error( ) & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_error & {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		inline json_rpc_response_error &&error( ) && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ), []( auto &&v ) -> json_rpc_response_error && {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		inline json_rpc_response_error const &&error( ) const && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_error const && {
				  if constexpr( std::is_same_v<daw::remove_cvref_t<decltype( v )>,
				                               json_rpc_response_error> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		inline T &result( ) & {
			return response( ).result;
		}

		inline T const &result( ) const & {
			return response( ).result;
		}

		inline T &&result( ) && {
			return response( ).result;
		}

		inline T const &&result( ) const && {
			return response( ).result;
		}
	};
} // namespace daw::json_rpc

namespace daw::json {
	template<typename Result>
	struct json_data_contract<daw::json_rpc::json_rpc_response<Result>> {
		static constexpr char const mem_error[] = "error";
		using type = json_submember_tagged_variant<
		  json_delayed_null<mem_error>,
		  daw::json_rpc::json_rpc_response_details::response_switcher,
		  daw::json_rpc::json_rpc_response_result<Result>,
		  daw::json_rpc::json_rpc_response_error>;
	};
} // namespace daw::json
