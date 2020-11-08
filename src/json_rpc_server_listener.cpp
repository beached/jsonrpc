// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "daw/json_rpc_server/json_rpc_server_listener.h"
#include "daw/json_rpc_server/json_rpc_server_error.h"
#include "daw/json_rpc_server/json_rpc_server_session_detect.h"

#include <iostream>

namespace daw::json_rpc_server {

	json_rpc_server_listener::json_rpc_server_listener(
	  boost::asio::io_context &io_ctx, boost::asio::ssl::context &ssl_ctx,
	  boost::asio::ip::tcp::endpoint endpoint,
	  const std::shared_ptr<std::string> &doc_root )
	  : m_io_ctx( &io_ctx )
	  , m_ssl_ctx( &ssl_ctx )
	  , m_acceptor( boost::asio::make_strand( io_ctx ) )
	  , m_doc_root( doc_root ) {

		m_acceptor.open( endpoint.protocol( ) );
		m_acceptor.set_option( boost::asio::socket_base::reuse_address( true ) );
		m_acceptor.bind( endpoint );
		m_acceptor.listen( boost::asio::socket_base::max_listen_connections );
	}

	void json_rpc_server_listener::do_accept( ) {
		m_acceptor.async_accept(
		  boost::asio::make_strand( *m_io_ctx ),
		  boost::beast::bind_front_handler( &json_rpc_server_listener::on_accept,
		                                    shared_from_this( ) ) );
	}

	void
	json_rpc_server_listener::on_accept( boost::beast::error_code ec,
	                                     boost::asio::ip::tcp::socket socket ) {
		if( ec ) {
			fail( ec, "listen" );
			do_accept( );
			return;
		}
		std::make_shared<json_rpc_server_session_detect>( std::move( socket ),
		                                                  m_io_ctx, m_doc_root )
		  ->run( );
		do_accept( );
	}

} // namespace daw::json_rpc_server