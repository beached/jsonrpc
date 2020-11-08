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
	class json_rpc_server_listener
	  : std::enable_shared_from_this<json_rpc_server_listener> {

		boost::asio::io_context *m_io_ctx;
		boost::asio::ssl::context *m_ssl_ctx;
		boost::asio::ip::tcp::acceptor m_acceptor;
		std::shared_ptr<std::string> m_doc_root;

	public:
		json_rpc_server_listener( boost::asio::io_context &io_ctx,
		                          boost::asio::ssl::context &ssl_ctx,
		                          boost::asio::ip::tcp::endpoint endpoint,
		                          std::shared_ptr<std::string> const &doc_root );

		void run( ) {
			do_accept( );
		}

	private:
		void do_accept( );

		void on_accept( boost::beast::error_code ec,
		                boost::asio::ip::tcp::socket socket );
	};
} // namespace daw::json_rpc_server
