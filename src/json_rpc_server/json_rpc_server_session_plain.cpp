// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc_server/json_rpc_server_session_plain.h"

#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <chrono>
#include <memory>

namespace daw::json_rpc_server {
	// Handles a plain HTTP connection
	json_rpc_server_session_plain::json_rpc_server_session_plain(
	  boost::asio::ip::tcp::socket &&socket, boost::beast::flat_buffer buffer,
	  std::shared_ptr<std::string const> const &doc_root )
	  : json_rpc_server_session<json_rpc_server_session_plain>(
	      std::move( buffer ), doc_root )
	  , m_stream( std::move( socket ) ) {}

	// Called by the base class
	boost::beast::tcp_stream &json_rpc_server_session_plain::stream( ) {
		return m_stream;
	}

	// Start the asynchronous operation
	void json_rpc_server_session_plain::run( ) {
		// We need to be executing within a strand to perform async operations
		// on the I/O objects in this session. Although not strictly necessary
		// for single-threaded contexts, this example code is written to be
		// thread-safe by default.
		boost::asio::dispatch(
		  m_stream.get_executor( ),
		  boost::beast::bind_front_handler( &json_rpc_server_session::do_read,
		                                    shared_from_this( ) ) );
	}

	void json_rpc_server_session_plain::do_eof( ) {
		// Send a TCP shutdown
		boost::beast::error_code ec;
		m_stream.socket( ).shutdown( boost::asio::ip::tcp::socket::shutdown_send,
		                             ec );

		// At this point the connection is closed gracefully
	}
} // namespace daw::json_rpc_server