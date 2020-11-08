// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include "json_rpc_server_session.h"

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
	class json_rpc_server_session_plain
	  : public json_rpc_server_session<json_rpc_server_session_plain>,
	    public std::enable_shared_from_this<json_rpc_server_session_plain> {
		boost::beast::tcp_stream m_stream;

	public:
		// Create the session
		json_rpc_server_session_plain(
		  boost::asio::ip::tcp::socket &&socket, boost::beast::flat_buffer buffer,
		  std::shared_ptr<std::string const> const &doc_root );
		// Called by the base class
		boost::beast::tcp_stream &stream( );

		// Start the asynchronous operation
		void run( );
		void do_eof( );
	};
} // namespace daw::json_rpc_server