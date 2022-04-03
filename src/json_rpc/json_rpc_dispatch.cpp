// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc/json_rpc_dispatch.h"
#include "daw/daw_storage_ref.h"
#include <daw/daw_construct_at.h>

#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>

namespace daw::json_rpc {
	inline namespace {
		struct impl_t {
			std::unordered_map<std::string, json_rpc::callback_type> handlers{ };
		};

		inline constexpr auto get_ref =
		  daw::storage_ref<impl_t, json_rpc_dispatch::storage_t>{ };
	} // namespace

	void
	json_rpc_dispatch::operator( )( details::json_rpc_server_request const &req,
	                                std::string &buff ) const {

		if( auto pos = get_ref( m_storage ).handlers.find( req.method );
		    pos != get_ref( m_storage ).handlers.end( ) ) {
			return pos->second( req, buff );
		}
		auto it = std::back_inserter( buff );
		daw::json::to_json(
		  json_rpc_response_error( Error( -32601, "Method not found" ), req.id ),
		  it );
	}

	void json_rpc_dispatch::add_method( std::string name,
	                                    json_rpc::callback_type &&callback ) {
		get_ref( m_storage )
		  .handlers.insert_or_assign( DAW_MOVE( name ), DAW_MOVE( callback ) );
	}

	json_rpc_dispatch::json_rpc_dispatch( ) {
		static_assert( sizeof( impl_t ) <= sizeof( storage_t ) );

		daw::construct_at<impl_t>( &m_storage );
	}

	json_rpc_dispatch::json_rpc_dispatch( json_rpc_dispatch &&other ) noexcept(
	  false ) {
		daw::construct_at<impl_t>( &m_storage,
		                           get_ref( DAW_MOVE( other.m_storage ) ) );
	}

	json_rpc_dispatch::json_rpc_dispatch(
	  json_rpc_dispatch const &other ) noexcept( false ) {
		daw::construct_at<impl_t>( &m_storage, get_ref( other.m_storage ) );
	}

	json_rpc_dispatch &
	json_rpc_dispatch::operator=( json_rpc_dispatch &&other ) noexcept( false ) {
		get_ref( m_storage ) = get_ref( DAW_MOVE( other.m_storage ) );
		return *this;
	}

	json_rpc_dispatch &json_rpc_dispatch::operator=(
	  json_rpc_dispatch const &other ) noexcept( false ) {
		get_ref( m_storage ) = get_ref( other.m_storage );
		return *this;
	}

	json_rpc_dispatch::~json_rpc_dispatch( ) {
		std::destroy_at( &get_ref( m_storage ) );
	}
} // namespace daw::json_rpc
