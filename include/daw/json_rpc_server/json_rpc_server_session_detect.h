// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <memory>

namespace daw::json_rpc_server {
	class json_rpc_server_session_detect
	  : public std::enable_shared_from_this<json_rpc_server_session_detect> {
		// Detects SSL handshakes
		boost::beast::tcp_stream m_stream;
		boost::asio::ssl::context *m_ssl_ctx;
		std::shared_ptr<std::string const> m_doc_root;
		boost::beast::flat_buffer m_buffer;

	public:
		json_rpc_server_session_detect(
		  boost::asio::ip::tcp::socket &&socket, boost::asio::ssl::context &ctx,
		  std::shared_ptr<std::string const> const &doc_root );

		// Launch the detector
		void run( );

		void on_detect( boost::beast::error_code ec, bool result );
	};
} // namespace daw::json_rpc_server