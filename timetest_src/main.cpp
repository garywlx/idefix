#include <iostream>
#include <quickfix/Field.h>
#include <thread>
#include <chrono>

using namespace std;
using namespace FIX;

/*!
 * Print UtcTimeStamp
 * @param FIX::UtcTimeStamp& t UtcTimeStamp to print.
 */
void print_time(UtcTimeStamp &t) {
	int Y,m,d,H,i,s,ms;

	t.getYMD(Y, m, d);
	t.getHMS(H, i, s, ms);
	
	printf("%d-%02d-%02d %d:%02d:%02d.%d\n", Y, m, d, H, i, s, ms);
}

/*!
 * Check if the timestamp hits the period
 * 
 * @param const std::string&  timestamp Timestamp in format yyyymmdd-H:i:s.u
 * @param const int           period    Period in seconds, e.g. 60 = 1 Minute
 * @return bool
 */
bool is_period_hit(const std::string& timestamp, const int period) {
	// convert string to UtcTimeStamp
	UtcTimeStamp nowTS = UtcTimeStampConvertor::convert( timestamp );

	// seconds 0 - 59
	if ( period < 60 && ( nowTS.getSecond() % period == 0 ) ) {
		return true;
	}
	// minutes 0 - 59
	else if ( ( period > 60 && period < 3540 ) && nowTS.getSecond() == 0 && ( nowTS.getMinute() % ( period / 60 ) == 0 ) ) {
		return true;
	} 
	// hours 0 - 23
	else if ( ( period > 3540 && period < 82800 ) && nowTS.getMinute() == 0 && nowTS.getSecond() == 0 && ( nowTS.getHour() % ( period / 3600 ) == 0 ) ) {
		return true;
	}

	return false;
}

// Entry
int main(int argc, char** argv) {

	// Start
	string now = "20180728-12:00:00.555";
	UtcTimeStamp nowTS = UtcTimeStampConvertor::convert( now );

	int period = 30; // seconds
	int step   = 1;  // seconds
	cout << "Period " << period << endl;
	cout << "Step   " << step << endl;

	while( true ) {

		// print ts
		print_time( nowTS );
		
		std::string nowTSstr = UtcTimeStampConvertor::convert( nowTS, 3 );
		if ( is_period_hit( nowTSstr, period ) ) {
			cout << "hit" << endl;
		}		

		// plus 1 second
		nowTS += step;

		// sleep for 1 second
		this_thread::sleep_for( chrono::seconds(1) );
	}
}