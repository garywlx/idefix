#include <Simple-Web-Server/server_http.hpp>

#define BOOST_SPIRIT_THREADSAFE
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace boost::property_tree;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

namespace idefix {

	static HttpServer kHttpServer;

class WebContext {
public:

	WebContext(const unsigned short port);
	~WebContext();

	void start();
	void stop();
};

};