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
	template<typename Data>
	struct Error {
		int code;
		std::optional<std::string> message;
		std::optional<Data> data{ };

		explicit Error( int Code )
		  : code( Code ) {}

		explicit Error( int Code, std::string Message )
		  : code( Code )
		  , message( DAW_MOVE( Message ) ) {}

		explicit Error( int Code, std::string Message, Data const &d )
		  : code( Code )
		  , message( DAW_MOVE( Message ) )
		  , data( d ) {}

		explicit Error( int Code, std::string Message, Data &&d )
		  : code( Code )
		  , message( DAW_MOVE( Message ) )
		  , data( DAW_MOVE( d ) ) {}

		explicit Error( int &&Code, std::optional<std::string> &&Message,
		                std::optional<Data> &&d )
		  : code( Code )
		  , message( DAW_MOVE( Message ) )
		  , data( DAW_MOVE( d ) ) {}
	};

	Error( int ) -> Error<daw::json::json_value>;
	Error( int, std::string ) -> Error<daw::json::json_value>;

	template<typename Data>
	Error( int, std::string, Data ) -> Error<Data>;

	template<typename Data>
	struct json_rpc_response_error {
		std::string jsonrpc;
		Error<Data> error;
		details::id_type id;

		using i_am_a_json_rpc_response_error = void;

		explicit json_rpc_response_error( Error<Data> err,
		                                  details::id_type Id = { } )
		  : error( DAW_MOVE( err ) )
		  , id( DAW_MOVE( Id ) ) {}

		explicit json_rpc_response_error( std::string &&JsonRPC, Error<Data> &&err,
		                                  details::id_type &&Id )
		  : jsonrpc( DAW_MOVE( JsonRPC ) )
		  , error( DAW_MOVE( err ) )
		  , id( DAW_MOVE( Id ) ) {}
	};

	template<typename Data>
	json_rpc_response_error( Error<Data> ) -> json_rpc_response_error<Data>;

	template<typename Data>
	json_rpc_response_error( Error<Data>, details::id_type )
	  -> json_rpc_response_error<Data>;
} // namespace daw::json_rpc

namespace daw::json {
	template<typename Data>
	struct json_data_contract<daw::json_rpc::Error<Data>> {
		static constexpr char const mem_code[] = "code";
		static constexpr char const mem_message[] = "message";
		static constexpr char const mem_data[] = "data";
		using type =
		  json_member_list<json_link<mem_code, int>,
		                   json_link<mem_message, std::optional<std::string>>,
		                   json_link<mem_data, std::optional<Data>>>;

		static inline constexpr auto
		to_json_data( daw::json_rpc::Error<Data> const &v ) {
			return std::forward_as_tuple( v.code, v.message, v.data );
		}
	};

	template<typename Data>
	struct json_data_contract<daw::json_rpc::json_rpc_response_error<Data>> {
		static constexpr char const mem_jsonrpc[] = "jsonrpc";
		static constexpr char const mem_error[] = "error";
		using type =
		  json_member_list<json_link<mem_jsonrpc, std::string>,
		                   json_link<mem_error, daw::json_rpc::Error<Data>>,
		                   daw::json_rpc::details::id_json_map_type>;

		static inline auto
		to_json_data( daw::json_rpc::json_rpc_response_error<Data> const &v ) {
			static constexpr daw::string_view jsonrpc = "2.0";
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

		inline json_rpc_response_result( daw::string_view /*rpc_ver*/, Result &&r,
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

		using type = json_member_list<json_link<mem_jsonrpc, daw::string_view>,
		                              json_link<mem_result, Result>,
		                              daw::json_rpc::details::id_json_map_type>;

		static inline auto to_json_data(
		  daw::json_rpc::json_rpc_response_result<Result> const &value ) {
			static constexpr daw::string_view jsonrpc = "2.0";
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

	template<typename T>
	using i_am_a_json_rpc_response_error_test =
	  typename T::i_am_a_json_rpc_response_error;

	template<typename T>
	inline constexpr bool i_am_a_json_rpc_response_error_v =
	  daw::is_detected_v<i_am_a_json_rpc_response_error_test,
	                     daw::remove_cvref_t<T>>;
} // namespace daw::json_rpc::json_rpc_response_details

namespace daw::json_rpc {
	template<typename T, typename ErrorData = daw::json::json_value>
	class json_rpc_response {
		std::variant<json_rpc_response_result<T>,
		             json_rpc_response_error<ErrorData>>
		  m_response;

	public:
		explicit inline json_rpc_response( json_rpc_response_error<ErrorData> err )
		  : m_response( DAW_MOVE( err ) ) {}

		explicit inline json_rpc_response( json_rpc_response_result<T> result )
		  : m_response( DAW_MOVE( result ) ) {}

		[[nodiscard]] inline bool has_error( ) const {
			return m_response.index( ) == 1;
		}

		[[nodiscard]] inline bool has_response( ) const {
			return m_response.index( ) == 0;
		}

		explicit inline operator bool( ) const {
			return has_response( );
		}

		[[nodiscard]] inline json_rpc_response_result<T> const &
		response( ) const & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_result<T> const & {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_result<T> &response( ) & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_result<T> & {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_result<T> &&response( ) && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_result<T> && {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_result<T> const &&
		response( ) const && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_result<T> const && {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  throw v;
				  } else {
					  return v;
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_error<ErrorData> const &
		error( ) const & {
			return daw::visit_nt(
			  m_response,
			  []( auto &&v ) -> json_rpc_response_error<ErrorData> const & {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_error<ErrorData> &error( ) & {
			return daw::visit_nt(
			  m_response, []( auto &&v ) -> json_rpc_response_error<ErrorData> & {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_error<ErrorData> &&error( ) && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_error<ErrorData> && {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		[[nodiscard]] inline json_rpc_response_error<ErrorData> const &&
		error( ) const && {
			return daw::visit_nt(
			  DAW_MOVE( m_response ),
			  []( auto &&v ) -> json_rpc_response_error<ErrorData> const && {
				  if constexpr( daw::json_rpc::json_rpc_response_details::
				                  i_am_a_json_rpc_response_error_v<decltype( v )> ) {
					  return v;
				  } else {
					  throw std::bad_variant_access{ };
				  }
			  } );
		}

		[[nodiscard]] inline T &result( ) & {
			return response( ).result;
		}

		[[nodiscard]] inline T const &result( ) const & {
			return response( ).result;
		}

		[[nodiscard]] inline T &&result( ) && {
			return response( ).result;
		}

		[[nodiscard]] inline T const &&result( ) const && {
			return response( ).result;
		}
	};
} // namespace daw::json_rpc

namespace daw::json {
	template<typename Result, typename ErrorData>
	struct json_data_contract<
	  daw::json_rpc::json_rpc_response<Result, ErrorData>> {
		static constexpr char const mem_error[] = "error";
		using type = json_submember_tagged_variant<
		  json_raw_null<mem_error>,
		  daw::json_rpc::json_rpc_response_details::response_switcher,
		  daw::json_rpc::json_rpc_response_result<Result>,
		  daw::json_rpc::json_rpc_response_error<ErrorData>>;
	};
} // namespace daw::json
