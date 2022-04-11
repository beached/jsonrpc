// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/beached/jsonrpc
//

#pragma once

#include "json_rpc/json_rpc_dispatch.h"

#include <daw/daw_concepts.h>
#include <daw/daw_move.h>

#include <crow.h>
#include <crow/middlewares/cookie_parser.h>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>

namespace daw::json_rpc {
	namespace impl {
		template<template<typename> typename Pred, typename... Opts>
		constexpr auto *find_opt( Opts &...opts ) noexcept {
			if constexpr( sizeof...( opts ) == 0 ) {
				return static_cast<void *>( nullptr );
			} else if constexpr( Pred<daw::traits::first_type<Opts...>>::value ) {
				return &std::get<0>( std::forward_as_tuple( opts... ) );
			} else {
				constexpr auto pop_front = []( auto const &, auto &...vs ) {
					return find_opt<Pred>( vs... );
				};
				return pop_front( opts... );
			}
		}

		template<template<typename...> typename Result>
		struct named_ws_arg {
			using i_am_an_option = void;
			template<not_me<named_ws_arg> Func>
			requires( requires( Func &&f ) { Result<Func>{ DAW_FWD( f ) }; } ) //
			  constexpr auto
			  operator=( Func &&f ) const {
				return Result<Func>{ DAW_FWD( f ) };
			}
		};

		template<invocable_result<bool, crow::request const &> Func>
		struct on_ws_accept_t {
			using i_am_an_option = void;
			using accept_arg = void;
			Func f;
		};
		template<typename T>
		  struct find_accept_arg : std::bool_constant < requires {
			typename T::accept_arg;
		} > { };

		template<invocable<crow::websocket::connection &> Func>
		struct on_ws_open_t : Func {
			using i_am_an_option = void;
			using open_arg = void;
			using Func::operator( );
		};
		template<typename T>
		  struct find_open_arg : std::bool_constant < requires {
			typename T::open_arg;
		} > { };

		template<
		  invocable<crow::websocket::connection &, std::string const &, bool> Func>
		struct on_ws_message_t : Func {
			using i_am_an_option = void;
			using message_arg = void;
			using Func::operator( );
		};
		template<typename T>
		  struct find_message_arg : std::bool_constant < requires {
			typename T::message_arg;
		} > { };

		template<invocable<crow::websocket::connection &> Func>
		struct on_ws_error_t : Func {
			using i_am_an_option = void;
			using error_arg = void;
			using Func::operator( );
		};
		template<typename T>
		  struct find_error_arg : std::bool_constant < requires {
			typename T::error_arg;
		} > { };

		template<invocable<crow::websocket::connection &, std::string const &> Func>
		struct on_ws_close_t : Func {
			using i_am_an_option = void;
			using close_arg = void;
			using Func::operator( );
		};

		template<typename T>
		  struct find_close_arg : std::bool_constant < requires {
			typename T::close_arg;
		} > { };
	} // namespace impl

	template<typename T>
	concept WSOption = requires {
		typename T::i_am_an_option;
	};

	inline namespace ws_opts {
		inline constexpr auto on_ws_accept =
		  impl::named_ws_arg<impl::on_ws_accept_t>{ };
		inline constexpr auto on_ws_open =
		  impl::named_ws_arg<impl::on_ws_open_t>{ };
		inline constexpr auto on_ws_message =
		  impl::named_ws_arg<impl::on_ws_message_t>{ };
		inline constexpr auto on_ws_error =
		  impl::named_ws_arg<impl::on_ws_error_t>{ };
		inline constexpr auto on_ws_close =
		  impl::named_ws_arg<impl::on_ws_close_t>{ };
	} // namespace ws_opts

	struct json_rpc_server {
		using storage_t = std::aligned_storage_t<3376, 64>;
		using cookie_t = typename crow::CookieParser::context;

	private:
		crow::App<crow::CookieParser> server{ };

	public:
		json_rpc_server( );

		~json_rpc_server( ) = default;
		json_rpc_server( json_rpc_server &&other ) = delete;
		json_rpc_server &operator=( json_rpc_server &&rhs ) = delete;
		json_rpc_server( json_rpc_server const & ) = delete;
		json_rpc_server &operator=( json_rpc_server const & ) = delete;

		/// @brief Start listening for network connections on specified port
		/// @param port Network port to listen for connections on
		json_rpc_server &listen( std::uint16_t port ) &;

		/// @brief Start listening for network connections on specified port/host
		/// @param host Bind to the specified host
		/// @param port Network port to listen for connections on
		json_rpc_server &listen( std::string_view host, std::uint16_t port ) &;

		/// @brief Stop listening for connections and terminate existing connections
		json_rpc_server &stop( ) &;

		json_rpc_server &route_path_to(
		  std::string_view req_path, std::string const &method,
		  std::function<void( crow::request const &, crow::response & )> handler )
		  &;

		json_rpc_server &
		route_path_to( std::string_view req_path_prefix,
		               std::filesystem::path fs_base,
		               std::optional<std::string> default_file = { } ) &;

		template<typename Result, typename Class>
		json_rpc_server &route_path_to( std::string_view req_path,
		                                std::string const &method,
		                                Result Class::*pm, Class &obj ) & {

			return route_path_to(
			  req_path, method,
			  [pm, obj]( crow::request const &req, crow::response &res ) mutable {
				  (void)( obj.*pm )( req, res );
			  } );
		}

		json_rpc_server &route_path_to( std::string_view req_path,
		                                json_rpc_dispatch &dispatcher ) &;

		json_rpc_server &websocket( std::string_view req_path,
		                            WSOption auto &&...opts ) {
			auto &ws = server.route_dynamic( static_cast<std::string>( req_path ) )
			             .websocket( );
			if( auto *ptr = impl::find_opt<impl::find_accept_arg>( opts... ); ptr ) {
				if constexpr( not same_as<void *, decltype( ptr )> ) {
					ws.onaccept( *ptr );
				}
			}
			if( auto *ptr = impl::find_opt<impl::find_close_arg>( opts... ); ptr ) {
				if constexpr( not same_as<void *, decltype( ptr )> ) {
					ws.onclose( *ptr );
				}
			}
			if( auto *ptr = impl::find_opt<impl::find_error_arg>( opts... ); ptr ) {
				if constexpr( not same_as<void *, decltype( ptr )> ) {
					ws.onerror( *ptr );
				}
			}
			if( auto *ptr = impl::find_opt<impl::find_message_arg>( opts... ); ptr ) {
				if constexpr( not same_as<void *, decltype( ptr )> ) {
					ws.onmessage( *ptr );
				}
			}
			if( auto *ptr = impl::find_opt<impl::find_open_arg>( opts... ); ptr ) {
				if constexpr( not same_as<void *, decltype( ptr )> ) {
					ws.onopen( *ptr );
				}
			}
			return *this;
		}

		cookie_t &get_cookie_context( crow::request const &req );
	};
} // namespace daw::json_rpc