#include "WebContext.h"

namespace idefix {
	WebContext::WebContext(const unsigned short port) {
		kHttpServer.config.port = port;
	}

	// Start the server
	void WebContext::start() {
		// default GET. If no other matches, this anonymous function will be called.
		// Will respond with content in the web/-directory, and its subdirectories.
		// Default file: index.html
		kHttpServer.default_resource["GET"] = []( shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
			try {
				auto web_root_path = boost::filesystem::canonical( "web" );
				auto path          = boost::filesystem::canonical( web_root_path / request->path );

				// check if path is withing web_root_path
				if ( distance( web_root_path.begin(), web_root_path.end() ) > distance( path.begin(), path.end() ) || 
					! equal( web_root_path.begin(), web_root_path.end(), path.begin() ) ) {
					throw invalid_argument("path must be within root path");
				}

				if ( boost::filesystem::is_directory( path ) ) {
					path /= "index.html";
				}

				SimpleWeb::CaseInsensitiveMultimap header;

				// Uncomment the following line to enable cache-control
				// header.emplace("Cache-Control", "max-age=86400");

				auto ifs = make_shared<ifstream>();
				ifs->open( path.string(), ifstream::in | ios::binary | ios::ate );

				if ( *ifs ) {
					auto length = ifs->tellg();
					ifs->seekg(0, ios::beg);

					header.emplace( "Content-Length", to_string( length ) );
					response->write( header );

					// Trick to define a recursive function within this scope (for example purposes)
					class FileServer {
						public:
							static void read_and_send(const shared_ptr<HttpServer::Response> &response, const shared_ptr<ifstream> &ifs) {
				            // Read and send 128 KB at a time
				            static vector<char> buffer(131072); // Safe when server is running on one thread
				            streamsize read_length;
				            if((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0) {
				            	response->write(&buffer[0], read_length);
				            	if(read_length == static_cast<streamsize>(buffer.size())) {
				            		response->send([response, ifs](const SimpleWeb::error_code &ec) {
				            			if(!ec)
				            				read_and_send(response, ifs);
				            			else
				            				cerr << "Connection interrupted" << endl;
				            		});
				            	}
				            }
				        }
				    };
				    FileServer::read_and_send(response, ifs);
				} else {
					throw invalid_argument("could not read file");
				}
				
			} catch( const exception &e ) {
				response->write( SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what() );
			}
		};

		// on error
		kHttpServer.on_error = []( shared_ptr<HttpServer::Request> request, const SimpleWeb::error_code& error_code ) {
			// Handle errors here
			std::cout << "WebContext Error: " << error_code.message() << std::endl;
		};

		try {
			kHttpServer.start();
			
			
		} catch( std::runtime_error& e ) {
			std::cout << "WebContext Error: " << e.what() << std::endl;
		}
		// thread server_thread([&]() {
		// 	// start server
		// 	kHttpServer.start();
		// });

		// // wait for server to start
		// this_thread::sleep_for( chrono::seconds( 1 ) );
	}

	void WebContext::stop() {
		kHttpServer.stop();
	}

	WebContext::~WebContext() {
		stop();
	}
};