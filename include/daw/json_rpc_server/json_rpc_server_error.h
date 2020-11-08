//
// Created by Darrell Wright on 2020-10-04.
//

// Copyright (c) Darrell Wright
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#pragma once

#include <boost/asio/error.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <iostream>

namespace daw::json_rpc_server {
	// Report a failure
	inline constexpr void fail( boost::beast::error_code ec, char const *what ) {
		// ssl::error::stream_truncated, also known as an SSL "short read",
		// indicates the peer closed the connection without performing the
		// required closing handshake (for example, Google does this to
		// improve performance). Generally this can be a security issue,
		// but if your communication protocol is self-terminated (as
		// it is with both HTTP and WebSocket) then you may simply
		// ignore the lack of close_notify.
		//
		// https://github.com/boostorg/beast/issues/38
		//
		// https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
		//
		// When a short read would cut off the end of an HTTP message,
		// Beast returns the error beast::http::error::partial_message.
		// Therefore, if we see a short read here, it has occurred
		// after the message has been completed, so it is safe to ignore it.

		if( ec == boost::asio::ssl::error::stream_truncated ) {
			return;
		}

		std::cerr << what << ": " << ec.message( ) << '\n';
	}
} // namespace daw::json_rpc_server