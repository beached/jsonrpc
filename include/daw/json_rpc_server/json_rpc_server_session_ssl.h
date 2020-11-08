// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_server_session.h"

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <chrono>
#include <memory>

/*
 * namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
 */

namespace daw::json_rpc_server {
	class json_rpc_server_session_ssl
	  : public json_rpc_server_session<json_rpc_server_session_ssl>,
	    public std::enable_shared_from_this<json_rpc_server_session_ssl> {
		boost::beast::ssl_stream<boost::beast::tcp_stream> m_stream;

	public:
		// Create the session
		json_rpc_server_session_ssl(
		  boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx,
		  boost::beast::flat_buffer buffer,
		  std::shared_ptr<std::string const> const &doc_root )
		  : json_rpc_server_session<json_rpc_server_session_ssl>(
		      std::move( buffer ), doc_root )
		  , m_stream( std::move( socket ), ctx ) {}

		// Called by the base class
		boost::beast::ssl_stream<boost::beast::tcp_stream> &stream( ) {
			return m_stream;
		}

		// Start the asynchronous operation
		void run( ) {
			auto self = shared_from_this( );
			// We need to be executing within a strand to perform async operations
			// on the I/O objects in this session.
			boost::asio::dispatch( m_stream.get_executor( ), [self]( ) {
				// Set the timeout.
				boost::beast::get_lowest_layer( self->m_stream )
				  .expires_after( std::chrono::seconds( 30 ) );

				// Perform the SSL handshake
				// Note, this is the buffered version of the handshake.
				self->m_stream.async_handshake(
				  boost::asio::ssl::stream_base::server, self->m_buffer.data( ),
				  boost::beast::bind_front_handler(
				    &json_rpc_server_session_ssl::on_handshake, self ) );
			} );
		}

		void on_handshake( boost::beast::error_code ec, std::size_t bytes_used ) {
			if( ec ) {
				return fail( ec, "handshake" );
			}

			// Consume the portion of the buffer used by the handshake
			m_buffer.consume( bytes_used );

			do_read( );
		}

		void do_eof( ) {
			// Set the timeout.
			boost::beast::get_lowest_layer( m_stream )
			  .expires_after( std::chrono::seconds( 30 ) );

			// Perform the SSL shutdown
			m_stream.async_shutdown( boost::beast::bind_front_handler(
			  &json_rpc_server_session_ssl::on_shutdown, shared_from_this( ) ) );
		}

		void on_shutdown( boost::beast::error_code ec ) {
			if( ec ) {
				return fail( ec, "shutdown" );
			}

			// At this point the connection is closed gracefully
		}
	};
} // namespace daw::json_rpc_server
