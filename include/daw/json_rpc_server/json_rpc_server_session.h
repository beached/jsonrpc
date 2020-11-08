// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_server_error.h"

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <memory>

/*
 * namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
 */

namespace daw::json_rpc_server {
	// Handles an HTTP server connection.
	// This uses the Curiously Recurring Template Pattern so that
	// the same code works with both SSL streams and regular sockets.
	template<class Derived>
	class json_rpc_server_session {
		// Access the derived class, this is part of
		// the Curiously Recurring Template Pattern idiom.
		Derived &derived( ) {
			return static_cast<Derived &>( *this );
		}

		// This is the C++11 equivalent of a generic lambda.
		// The function object is used to send an HTTP message.
		struct send_lambda {
			json_rpc_server_session &m_self;

			explicit send_lambda( json_rpc_server_session &self )
			  : m_self( self ) {}

			template<bool isRequest, class Body, class Fields>
			void operator( )(
			  boost::beast::http::message<isRequest, Body, Fields> &&msg ) const {
				// The lifetime of the message has to extend
				// for the duration of the async operation so
				// we use a shared_ptr to manage it.
				auto sp = std::make_shared<
				  boost::beast::http::message<isRequest, Body, Fields>>(
				  std::move( msg ) );

				// Store a type-erased version of the shared
				// pointer in the class to keep it alive.
				m_self.m_res = sp;

				// Write the response
				boost::beast::http::async_write(
				  m_self.derived( ).stream( ), *sp,
				  boost::beast::bind_front_handler(
				    &json_rpc_server_session::on_write,
				    m_self.derived( ).shared_from_this( ), sp->need_eof( ) ) );
			}
		};

		std::shared_ptr<std::string const> m_doc_root;
		boost::beast::http::request<boost::beast::http::string_body> m_req;
		std::shared_ptr<void> m_res;
		send_lambda m_lambda;

	protected:
		boost::beast::flat_buffer m_buffer;

	public:
		// Take ownership of the buffer
		json_rpc_server_session(
		  boost::beast::flat_buffer buffer,
		  std::shared_ptr<std::string const> const &doc_root )
		  : m_doc_root( doc_root )
		  , m_lambda( *this )
		  , m_buffer( std::move( buffer ) ) {}

		void do_read( ) {
			// Set the timeout.
			boost::beast::get_lowest_layer( derived( ).stream( ) )
			  .expires_after( std::chrono::seconds( 30 ) );

			// Read a request
			boost::beast::http::async_read(
			  derived( ).stream( ), m_buffer, m_req,
			  boost::beast::bind_front_handler( &json_rpc_server_session::on_read,
			                                    derived( ).shared_from_this( ) ) );
		}

		void on_read( boost::beast::error_code ec, std::size_t bytes_transferred ) {
			boost::ignore_unused( bytes_transferred );

			// This means they closed the connection
			if( ec == boost::beast::http::error::end_of_stream )
				return derived( ).do_eof( );

			if( ec )
				return fail( ec, "read" );

			// Send the response
			handle_request( *m_doc_root, std::move( m_req ), m_lambda );
		}

		void on_write( bool close, boost::beast::error_code ec,
		               std::size_t bytes_transferred ) {
			boost::ignore_unused( bytes_transferred );

			if( ec ) {
				return fail( ec, "write" );
			}

			if( close ) {
				// This means we should close the connection, usually because
				// the response indicated the "Connection: close" semantic.
				return derived( ).do_eof( );
			}

			// We're done with the response so delete it
			m_res = nullptr;

			// Read another request
			do_read( );
		}
	};
} // namespace daw::json_rpc_server