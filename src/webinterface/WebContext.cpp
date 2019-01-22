#include "WebContext.h"
#include "../core/logger.h"

namespace idefix {

	typedef std::shared_ptr<WsServer::Connection> WsConnPtr;

	WebContext::WebContext(const unsigned short port) {
		kWsServer.config.port = port;
		kWsServer.config.reuse_address = true;

		SPDLOG_INFO("Start WebSocket Server at port {0:d}", port);
	}

	// start and initialize the websocket server
	void WebContext::startWsServer() {
		auto& endpoint = kWsServer.endpoint["^/api[/]?$"];

		endpoint.on_message = [](WsConnPtr connection, shared_ptr<WsServer::InMessage> in_message) {
			auto out_message = in_message->string();

			SPDLOG_INFO("SocketServer message received: {}", out_message.c_str() );
			SPDLOG_INFO("SocketServer sending message: {}", out_message.c_str() );

			// connection->send is an asynchronous function
			connection->send(out_message, [](const SimpleWeb::error_code &ec){
				if ( ec ) {
					SPDLOG_ERROR("SocketServer error ({0:d}): {}", ec.value(), ec.message() );
				}
			});

			// Alternatively use streams:
		    // auto out_message = make_shared<WsServer::OutMessage>();
		    // *out_message << in_message->string();
		    // connection->send(out_message);
		};

		// on open
		endpoint.on_open = [](WsConnPtr connection) {
			SPDLOG_INFO("SocketServer: opened connection");
		};

		// See RFC 6455 7.4.1. for status codes
		endpoint.on_close = [](WsConnPtr connection, int status, const string& reason) {
			SPDLOG_INFO("SocketServer: closed connection with status code {:d}", status);
		};

		// See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
		endpoint.on_error = [](WsConnPtr connection, const SimpleWeb::error_code &ec) {
			SPDLOG_ERROR("SocketServer: error in connection. Error ({0:d}): {}", ec.value(), ec.message() );
		};
	}

	// Start the server
	void WebContext::start() {
		try {
			startWsServer();

			kWsServer.start();

		} catch( std::runtime_error& e ) {
			SPDLOG_ERROR("SocketServer: Error. {}", e.what() );
		}
	}

	void WebContext::stop() {
		// kHttpServer.stop();
		kWsServer.stop();
	}

	// send a message
	void WebContext::send(const std::string message) {
		try {
			for( auto& a_connection : kWsServer.get_connections() ) {
				a_connection->send( message );
			}

		} catch( std::runtime_error& e ) {
			SPDLOG_ERROR("SocketServer: Error. {}", e.what() );
		}
	}

	WebContext::~WebContext() {
		stop();
	}
};