#include "tick70.h"
#include "core/logger.h"
#include "core/configfile.h"

#include <string>

namespace idefix {
Tick70::Tick70() {}

std::string Tick70::name() const noexcept {
	return "Tick70";	
}

void Tick70::initialize(DataContext& ctx) {
	SPDLOG_INFO("Initialize {}", name() );

	try {
		// config tree
		idefix::config::ptree_t ptree;

		// try to load config file
		if ( ! idefix::config::read_json( getConfigFile(), ptree ) ) {
			SPDLOG_ERROR("Can't read config file {}. Init failed.", getConfigFile() );
			return;
		}

		for ( auto& instrument_cfg : ptree.get_child( "instruments" ) ) {
			auto symbol = instrument_cfg.second.get<std::string>( "symbol" );
			//auto ticksize = instrument_cfg.second.get<unsigned int>( "ticksize" );

			ctx.subscribe( symbol );
		}

	} catch(std::exception& e) {
		SPDLOG_ERROR( "{}", e.what() );
	} catch( ... ) {
		SPDLOG_ERROR( "Unknown error." );
	}
};

void Tick70::onTick(DataContext& ctx, std::shared_ptr<Instrument> instrument) {
	SPDLOG_INFO("{} Bid {} Ask {} Spread {} SpreadPoints {}", 
		instrument->getSymbol(), instrument->format( instrument->getBidPrice() ), instrument->format( instrument->getAskPrice() ), 
		instrument->format( instrument->getSpread() ), instrument->format( instrument->getSpreadPoints() ) );
}

};

extern "C" {
	idefix::Algo* create() {
		return new idefix::Tick70{};
	}
}