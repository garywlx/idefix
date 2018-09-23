#include <iostream>
#include <nod/nod.hpp>
#include <functional>
#include <string>
// https://github.com/fr00b0/nod

class A {
public:
	nod::signal<void()> clicked;
	nod::signal<void(std::string&)> fuck;

	A(){

	}
	
};

class B {
public:
	B() {

	}

	void test() {
		std::cout << "B::test" << std::endl;
	}

	void fick_mit(std::string& name){
		std::cout << "fick mit " << name << std::endl;
	}
};

int main(int argc, char const *argv[])
{

	A a;
	a.clicked.connect( []() {
		std::cout << "clicked first" << std::endl;
	} );
	a.clicked.connect( []() {
		std::cout << "clicked second" << std::endl;
	});

	B b;
	a.clicked.connect( std::bind( &B::test, &b ) );

	a.fuck.connect( std::bind( &B::fick_mit, &b, std::placeholders::_1 ) );

	// Execute
	a.clicked();
	std::string maike = "Maike";
	a.fuck( maike );

	a.fuck.disconnect();

	return 0;
}