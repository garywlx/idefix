#include <simple-websocket-server/server_ws.hpp>

#include <algorithm>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

using namespace std;
using WsServer   = SimpleWeb::SocketServer<SimpleWeb::WS>;

namespace idefix {

	static WsServer kWsServer;

class WebContext {
public:

	WebContext(const unsigned short port);
	~WebContext();

	void start();
	void stop();

	void send(const std::string message);

private:
	void startWsServer();
};

};