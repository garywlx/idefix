#include "WebContext.h"

namespace idefix {

	typedef std::shared_ptr<WsServer::Connection> WsConnPtr;

	WebContext::WebContext(const unsigned short port) {
		kWsServer.config.port = port;
		kWsServer.config.reuse_address = true;

		std::cout << "Start WebSocket Server at port " << port << std::endl;
	}

	// start and initialize the websocket server
	void WebContext::startWsServer() {
		auto& endpoint = kWsServer.endpoint["^/api[/]?$"];

		endpoint.on_message = [](WsConnPtr connection, shared_ptr<WsServer::InMessage> in_message) {
			auto out_message = in_message->string();

			std::cout << "WsServer message received: " << out_message << " from " << connection.get() << std::endl;
			std::cout << "WsServer sending message:  " << out_message << " to " << connection.get() << std::endl;

			// connection->send is an asynchronous function
			connection->send(out_message, [](const SimpleWeb::error_code &ec){
				if ( ec ) {
					std::cout << "WsServer error: " << ec << " msg: " << ec.message() << std::endl;
				}
			});

			// Alternatively use streams:
		    // auto out_message = make_shared<WsServer::OutMessage>();
		    // *out_message << in_message->string();
		    // connection->send(out_message);
		};

		// on open
		endpoint.on_open = [](WsConnPtr connection) {
			std::cout << "WsServer: opened connection " << connection.get() << std::endl;
		};

		// See RFC 6455 7.4.1. for status codes
		endpoint.on_close = [](WsConnPtr connection, int status, const string& reason) {
			std::cout << "WsServer: closed connection " << connection.get() << " with status code " << status << std::endl;
		};

		// See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
		endpoint.on_error = [](WsConnPtr connection, const SimpleWeb::error_code &ec) {
			std::cout << "WsServer: error in connection " << connection.get() << ". Error: " << ec << ", msg: " << ec.message() << std::endl;
		};
	}

	// Start the server
	void WebContext::start() {
		try {
			startWsServer();

			kWsServer.start();

		} catch( std::runtime_error& e ) {
			std::cout << "WebContext Error: " << e.what() << std::endl;
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
			std::cout << "WebContext Error: " << e.what() << std::endl;
		}
	}

	WebContext::~WebContext() {
		stop();
	}
};