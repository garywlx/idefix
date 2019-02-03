#include "quickfix/FileStore.h"
#include "quickfix/FileLog.h"
#include "quickfix/SocketAcceptor.h"
#include "quickfix/Log.h"
#include "quickfix/SessionSettings.h"

#include "sim.h"

int main(int argc, char const *argv[])
{
	
	if ( argc < 2 )
	{
		std::cout << "usage: " << argv[ 0 ]
		<< " FILE." << std::endl;
		return 0;
	}
	std::string file = argv[ 1 ];

	FIX::Acceptor * acceptor = 0;
	try
	{
		FIX::SessionSettings settings( file );

		idefix::Sim sim;
		FIX::FileStoreFactory storeFactory( settings );
		//FIX::ScreenLogFactory logFactory( settings );
		FIX::FileLogFactory logFactory( settings );

		acceptor = new FIX::SocketAcceptor ( sim, storeFactory, settings, logFactory );

		acceptor->start();

		while(true) {
			std::cout << "Press q to quit." << std::endl;
			std::string cmd;
			std::cin >> cmd;

			if ( cmd == "q" ) break;
		}

		acceptor->stop();
		delete acceptor;
		return 0;
	}
	catch ( std::exception & e )
	{
		std::cout << e.what() << std::endl;
		delete acceptor;
		return 1;
	}

	return 0;
}