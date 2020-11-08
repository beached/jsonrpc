// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc_server/json_rpc_server_session_detect.h"
#include "daw/json_rpc_server/json_rpc_server_session_plain.h"
#include "daw/json_rpc_server/json_rpc_server_session_ssl.h"

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <chrono>
#include <memory>

namespace daw::json_rpc_server {
	json_rpc_server_session_detect::json_rpc_server_session_detect(
	  boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx,
	  const std::shared_ptr<const std::string> &doc_root )
	  : m_stream( std::move( socket ) )
	  , m_ssl_ctx( &ctx )
	  , m_doc_root( doc_root ) {}

	void json_rpc_server_session_detect::run( ) {
		// Set the timeout.
		boost::beast::get_lowest_layer( m_stream )
		  .expires_after( std::chrono::seconds( 30 ) );

		// Detect a TLS handshake
		async_detect_ssl(
		  m_stream, m_buffer,
		  boost::beast::bind_front_handler(
		    &json_rpc_server_session_detect::on_detect, shared_from_this( ) ) );
	}

	void json_rpc_server_session_detect::on_detect( boost::beast::error_code ec,
	                                                bool result ) {
		if( ec ) {
			return fail( ec, "detect" );
		}

		if( result ) {
			// Launch SSL session
			std::make_shared<json_rpc_server_session_ssl>(
			  m_stream.release_socket( ), *m_ssl_ctx, std::move( m_buffer ),
			  m_doc_root )
			  ->run( );
			return;
		}

		// Launch plain session
		std::make_shared<json_rpc_server_session_plain>(
		  m_stream.release_socket( ), std::move( m_buffer ), m_doc_root )
		  ->run( );
	}
} // namespace daw::json_rpc_server