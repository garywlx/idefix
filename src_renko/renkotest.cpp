#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../src/indicator/RenkoBrick.h"
#include "../src/Math.h"
#include "../src/Operators.h"
#include "../src/Console.h"
#include <quickfix/FieldConvertors.h>

#include <QApplication>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCandlestickSet>
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCore/QDateTime>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QString>

QT_CHARTS_USE_NAMESPACE

using namespace std;
using namespace IDEFIX;
using namespace FIX;

struct Position {
	enum STATUS {
		OPEN, CLOSED
	};
	double open_price;
	double close_price;
	double profit;
	double qty;
	std::string open_time;
	std::string close_time;
	RenkoBrick::STATUS direction;
	STATUS status;
};

inline std::ostream& operator<<(std::ostream& out, const Position& pos) {
	out << "\033[" << ( pos.direction == RenkoBrick::STATUS::SHORT ? "31" : "32" ) << "m";
	out << "POS " << pos.open_time << "@" << setprecision(2) << fixed << pos.open_price;
	if ( pos.status == Position::STATUS::CLOSED ) {
		out << " " << pos.close_time << "@" << setprecision(2) << fixed << pos.close_price;	
	}
	out << " Qty " << setprecision(0) << pos.qty << " Profit " << setprecision(2) << fixed << pos.profit << " USD"; 
	out << "\033[0m";
	return out;
}

class Strategy {
private:
	std::vector<RenkoBrick> m_bricks;
	std::vector<Position> m_positions;
	int m_open_long;
	int m_open_short;

	int m_total_long_positions;
	int m_total_short_positions;

	int m_max_pyramid;
	bool m_verbose;

public:
	Strategy(const int pyramid, const bool verbose = false): 
	  m_open_short(0), m_open_long(0), m_total_short_positions(0), 
	  m_total_long_positions(0), m_max_pyramid(pyramid), m_verbose(verbose) {
		registerConsole();
	}

	inline int total_long_positions() const {
		return m_total_long_positions;
	}

	inline int total_short_positions() const {
		return m_total_short_positions;
	}

	inline double total_profit() const {
		double profit = 0;

		for ( auto& pos : m_positions ) {
			if ( pos.status == Position::STATUS::CLOSED ) {
				profit += pos.profit;
			}
		}

		return profit;
	}

	inline void show_trade_list() {
		for ( auto& pos : m_positions ) {
			cout << pos << endl;
		}
	}

	inline double profit(const Position& pos, const RenkoBrick& brick) {
		double pip_diff_decimal = 0;
		double pnl = 0;
		double pip_value = 10;

		if ( pos.direction == RenkoBrick::STATUS::LONG ) {
			pip_diff_decimal = brick.close_price - pos.open_price;
		} else {
			pip_diff_decimal = pos.open_price - brick.close_price;
		}

		double pips = pip_diff_decimal / brick.point_size;

		pnl = ( ( pips * ( pos.qty / 100000 ) ) * pip_value );

		return pnl;
	}

	inline void onBrick(const RenkoBrick& brick) {
		// add brick to brick stack
		m_bricks.push_back( brick );

		// get previous brick
		if ( m_bricks.size() < 2 ) {
			if ( m_verbose ) {
				cout << "Not enough bricks." << endl;	
			}
			return;
		}

		auto brick_it = m_bricks.rbegin();
		brick_it += 1;
		RenkoBrick brick_1 = *brick_it;

		// output brick
		if ( m_verbose ) {
			cout << "BRICK " << brick << endl;	
		}		
		
		std::string open_msg;
		// STRATEGY LOGIC
		if ( ! above( m_open_long, m_max_pyramid ) ) {
			// we have no open long positions
			// entry signal long
			// brick_1 == SHORT
			// brick   == LONG					
			bool open_buy = equals( brick_1.status, RenkoBrick::STATUS::SHORT ) && equals( brick.status, RenkoBrick::STATUS::LONG );
			open_msg = "b1 == SHORT && b0 == LONG";
			// OR
			// brick_1 == LONG
			// brick   == LONG
			if ( ! open_buy ) {
				open_buy = equals( brick_1.status, RenkoBrick::STATUS::LONG ) && equals( brick.status, RenkoBrick::STATUS::LONG );
				open_msg = "b1 == LONG  && b0 == LONG";
			} 
			// brick > last_ma
			if ( open_buy ) {
				Position pos;
				pos.open_time  = brick.close_time;
				pos.open_price = brick.close_price;
				pos.profit     = 0;
				pos.direction  = RenkoBrick::STATUS::LONG;
				pos.status     = Position::STATUS::OPEN;
				pos.qty        = 100000; // 1 lot

				m_positions.push_back( pos );
				
				if ( m_verbose ) {
					console()->warn( "BUY  {}", open_msg );	
				}
				
				m_open_long += 1;
				m_total_long_positions += 1;
			}
		} else {
			// we have open long positions, check for closing
			// exit signal long
			// brick_1 == LONG
			// brick   == SHORT
			bool exit_buy = equals( brick_1.status, RenkoBrick::STATUS::LONG ) && equals( brick.status, RenkoBrick::STATUS::SHORT );
			// brick.close_price < last_ma
			if ( exit_buy ) {
				for ( auto& pos : m_positions ) {
					if ( pos.direction == RenkoBrick::STATUS::LONG && pos.status == Position::STATUS::OPEN ) {
						// set values
						pos.close_price = brick.close_price;
						pos.close_time  = brick.close_time;

						// calculate profit						
						pos.profit = profit(pos, brick);

						// set status
						pos.status = Position::STATUS::CLOSED;

						if ( m_verbose ) {
							console()->warn("EXIT BUY POS b1 == LONG && b0 == SHORT PnL: {:.2f}", pos.profit );	
						}
					}
				}

				m_open_long = 0;
			}
		}

		if ( ! above( m_open_short, m_max_pyramid ) ) {
			// we have no short positions	
			std::string open_msg;
			// entry signal short
			// brick_1 == LONG
			// brick   == SHORT
			bool open_sell = equals( brick_1.status, RenkoBrick::STATUS::LONG ) && equals( brick.status, RenkoBrick::STATUS::SHORT );
			open_msg = "b1 == LONG && b0 == SHORT";
			// OR
			// brick_1 == SHORT
			// brick   == SHORT
			if ( ! open_sell ) {
				open_sell = equals( brick_1.status, RenkoBrick::STATUS::SHORT ) && equals( brick.status, RenkoBrick::STATUS::SHORT );
				open_msg = "b1 == SHORT && b0 == SHORT";
			} 
			// 
			// brick <= last_ma
			if ( open_sell ) {

				Position pos;
				pos.open_time  = brick.close_time;
				pos.open_price = brick.close_price;
				pos.profit     = 0;
				pos.direction  = RenkoBrick::STATUS::SHORT;
				pos.status     = Position::STATUS::OPEN;
				pos.qty        = 100000;

				m_positions.push_back( pos );

				if ( m_verbose ) {
					console()->warn( "SELL {}", open_msg );	
				}
				
				m_open_short += 1;
				m_total_short_positions += 1;
			}
		} else {
			// we have short positions, check for closing
			// exit singal short
			// brick_1 == SHORT
			// brick   == LONG
			bool exit_sell = equals( brick_1.status, RenkoBrick::STATUS::SHORT ) && equals( brick.status, RenkoBrick::STATUS::LONG );
			// brick.close_price > last_ma
			if ( exit_sell ) {

				for ( auto& pos : m_positions ) {
					if ( pos.direction == RenkoBrick::STATUS::SHORT && pos.status == Position::STATUS::OPEN ) {
						// set values
						pos.close_price = brick.close_price;
						pos.close_time  = brick.close_time;

						// calculate profit						
						pos.profit = profit(pos, brick);

						// set status
						pos.status = Position::STATUS::CLOSED;

						if ( m_verbose ) {
							console()->warn( "EXIT SHORT POS b1 == SHORT && b0 == LONG PnL: {:.2f}", pos.profit );	
						}
					}
				}

				m_open_short = 0;
			}
		}
	}
};

// ---------------------------------------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	if ( argc < 2 ) {
		cerr << "Renko Strategy Test." << endl;
		cerr << "Usage: " << argv[0] << " <option(s)> renkobricks.csv" << endl;
		cerr << "Options:" << endl;
		cerr << "\t-p,--pyramid int    \t How many trades in one direction? Default is 0. That means only 1 Position at a time." << endl;
		cerr << "\t-v,--verbose        \t Show entries and exits when they happen." << endl;
		cerr << "\t-t,--trades         \t Show trade list" << endl;
		cerr << "\t-a,--account double \t Account start value. Default is 10.000 USD." << endl;
		cerr << "\t-r,--risk double    \t Risk per trade in percent of free margin. Default is 1%." << endl;
		cerr << "\t-q,--qty double     \t Quantity for new positions. Default is 100000." << endl;
		cerr << "\t-c,--chart          \t Show chart, desktop only." << endl;
		return EXIT_FAILURE;
	}

	bool show_verbose    = false;
	bool show_trade_list = false;
	bool show_chart      = false;
	int pyramid          = 0;
	double account       = 10000;
	double risk          = 1;
	double qty           = 100000;
	std::string input_file;

	for ( int i = 1; i < argc; i++ ) {
		std::string arg = argv[i];
		if ( ( arg == "-p" ) || ( arg == "--pyramid" ) ) {
			if ( i + 1 < argc ) {
				pyramid = IntConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-p,--pyramid option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-a" ) || ( arg == "--account" ) ) {
			if ( i + 1 < argc ) {
				account = DoubleConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-a,--account option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-r" ) || ( arg == "--risk" ) ) {
			if ( i + 1 < argc ) {
				risk = DoubleConvertor::convert( argv[ i + 1] );
			} else {
				cerr << "-r,--risk option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-q" ) || ( arg == "--qty" ) ) {
			if ( i + 1 < argc ) {
				qty = DoubleConvertor::convert( argv[ i + 1 ] );
			} else {
				cerr << "-q,--qty option needs one argument." << endl;
				return EXIT_FAILURE;
			}
		} else if ( ( arg == "-v" ) || ( arg == "--verbose" ) ) {
			show_verbose = true;
		} else if ( ( arg == "-t" ) || ( arg == "--trades" ) ) {
			show_trade_list = true;
		} else if ( ( arg == "-c" ) || ( arg == "--chart" ) ) { 
			show_chart = true;
		} else {
			input_file = argv[i];
		}
	}

	ifstream ifile( input_file );
	if ( ! ifile.good() ) {
		cerr << "File could not be opened." << endl;
		ifile.close();
		return EXIT_FAILURE;
	}

	std::vector<RenkoBrick> bricks;

	int line_i = 0;
	std::string line;
	while ( std::getline( ifile, line ) ) {
		std::stringstream line_stream( line );
		std::string cell;

		int cell_i = 0;

		// skip headlines
		if ( line_i == 0 ) {
			line_i++;
			continue;
		}

		RenkoBrick brick;

		// proceed in fields
		while ( std::getline( line_stream, cell, ',' ) ) {
			switch( cell_i ) {
				case 0: // status
					if ( cell == "-1" ) {
						brick.status = RenkoBrick::STATUS::NOSTATUS;
					} else if ( cell == "0" ) {
						brick.status = RenkoBrick::STATUS::LONG;
					} else if ( cell == "1" ) {
						brick.status = RenkoBrick::STATUS::SHORT;
					}
					break;
				case 1: // open time
					brick.open_time = cell;
					break;
				case 2: // open
					brick.open_price = DoubleConvertor::convert( cell );
					break;
				case 3: // high
					brick.high_price = DoubleConvertor::convert( cell );
					break;
				case 4: // low
					brick.low_price = DoubleConvertor::convert( cell );
					break;
				case 5: // close
					brick.close_price = DoubleConvertor::convert( cell );
					break;
				case 6: // close time
					brick.close_time = cell;
					break;
				case 7: // period
					brick.period = DoubleConvertor::convert( cell );
					break;
				case 8: // volume
					brick.volume = IntConvertor::convert( cell );
					break;
				case 9: // pointsize
					brick.point_size = DoubleConvertor::convert( cell );
					break;
			}

			cell_i++;
		}

		brick.diff = Math::get_spread( brick.open_price, brick.close_price, brick.point_size );

		bricks.push_back( brick );

		line_i++;
	}

	Strategy strategy( pyramid, show_verbose );

	int ticks = 0;
	for ( auto& b : bricks ) {
		strategy.onBrick( b );
		ticks += b.volume;
	}

	if ( show_trade_list ) {
		cout << "Trades:" << endl;

		strategy.show_trade_list();	
		cout << "==============================================" << endl;
	}
	cout << "File:                  " << input_file << endl;
	cout << "Start Time:            " << bricks[0].open_time << endl;
	cout << "Stop Time:             " << bricks.back().close_time << endl;
	cout << "Period:                " << setprecision(0) << bricks[0].period << endl;
	//cout << "Point Size:            " << setprecision(4) << fixed << bricks[0].point_size << endl;
	cout << "Bricks:                " << setprecision(0) << bricks.size() << endl;
	cout << "Ticks:                 " << setprecision(0) << ticks << endl;
	cout << "Pyramid Trades:        " << setprecision(0) << pyramid << endl;
	cout << "==============================================" << endl;
	cout << "Total Long Positions:  " << setprecision(0) << strategy.total_long_positions() << endl;
	cout << "Total Short Positions: " << setprecision(0) << strategy.total_short_positions() << endl;
	cout << "Account begin:         " << setprecision(2) << fixed << account << " USD" << endl;
	cout << "Account end:           " << setprecision(2) << fixed << ( account + strategy.total_profit() ) << " USD" << endl;
	cout << "Total Profit:          " << setprecision(2) << fixed << strategy.total_profit() << " USD" << endl;
	cout << "----------------------------------------------" << endl;

	// --------------------------------------------------------------------------------------------------------------
	// Draw chart with qt
	// --------------------------------------------------------------------------------------------------------------
	if ( ! show_chart ) {
		return EXIT_SUCCESS;
	}

	QApplication app(argc, argv);

	QCandlestickSeries *series = new QCandlestickSeries();
	series->setName( "Candles" );
	series->setIncreasingColor( QColor( Qt::green ) );
	series->setDecreasingColor( QColor( Qt::red ) );

	// QLineSeries *lineseries = new QLineSeries();
	// lineseries->setName("MovingAverage");

	QStringList categories;

	int i = 0;
	std::vector<double> brickSteps;

	for ( auto& b : bricks ) {

		brickSteps.push_back( b.close_price );

		UtcTimeStamp ts = UtcTimeStampConvertor::convert( b.close_time );

		QCandlestickSet *set = new QCandlestickSet( ts.getTimeT() );
		set->setOpen( b.open_price );
		set->setClose( b.close_price );
		
		if ( b.status == RenkoBrick::STATUS::LONG ) {
			set->setHigh( b.close_price );
			set->setLow( b.open_price );
		} else if ( b.status == RenkoBrick::STATUS::SHORT ) {
			set->setHigh( b.open_price );
			set->setLow( b.close_price );
		}

		if ( set ) {
			// Candle
			series->append( set );	
			//categories << QDateTime::fromMSecsSinceEpoch( set->timestamp() ).toString("ss");
			categories << QString(i);

			// MovingAverage
			double ma = Math::get_moving_average( brickSteps, 3 );
			// lineseries->append( QPoint(i, ma) );

			i++;
		}
	}

	QChart *chart = new QChart();
	chart->addSeries( series );
	// chart->addSeries( lineseries );
	chart->setTitle( QString( input_file.c_str() ) );
	chart->setAnimationOptions( QChart::SeriesAnimations );
	chart->createDefaultAxes();

	QBarCategoryAxis *axisX = qobject_cast<QBarCategoryAxis *>( chart->axes( Qt::Horizontal ).at( 0 ) );
	axisX->setCategories( categories );


	QValueAxis *axisY = qobject_cast<QValueAxis *>( chart->axes( Qt::Vertical ).at( 0 ) );
	axisY->setMax( axisY->max() * 1.01 );
	axisY->setMin( axisY->min() * 0.99 );

	chart->legend()->setVisible( false );
	// chart->legend()->setAlignment( Qt::AlignBottom );

	QChartView *chartView = new QChartView( chart );
	chartView->setRenderHint( QPainter::Antialiasing );

	QMainWindow window;
	window.setCentralWidget( chartView );
	window.resize( 800, 600 );
	window.setWindowTitle( QApplication::translate( "toplevel", "Strategy Test" ) );
	window.show();

	return app.exec();
}