# IDEFIX

IDEFIX is the abreviation for "Integrated Development Environment for Financial Information eXchange". It is the framework to build fully automated trading applications and supports the industry standard FIX protocol. The framework gives all necessary tools, functions and classes to build a running trading application from scratch.

## Folder Structure

```
|- build/ -> contains the binary files
|- docs/ -> the documentation files
|- src/ -> the source files
|- src/adapter/ -> the price data adapter
|- src/assets/ -> the asset definitions
|- tools/ -> helper scripts
```

## main.cpp example

```c++
#include <iostream>
#include "idefix.h"
#include "adapter/fileadapter.h"
#include "assets/eurusd.h"

int main(int argc, char** argv){
	
	idefix::assets::eurusd symbol;
	idefix::FileAdapter fa("../test/eurusd.csv", &symbol);

	return 0;
}
```

## build requirements

- cmake 3.11.0 

```bash
$ cmake --version
cmake version 3.11.0
```

- c++11 compiler

```bash
$ gcc --version
Configured with: --prefix=/Applications/Xcode.app/Contents/Developer/usr --with-gxx-include-dir=/usr/include/c++/4.2.1
Apple LLVM version 9.1.0 (clang-902.0.39.1)
Target: x86_64-apple-darwin17.5.0
Thread model: posix
```

## dependencies

- [QuickFIX C++ Fix Engine Library](https://github.com/quickfix/quickfix)

- [FXCM Tick Data](https://github.com/fxcm/FXCMTickData)

- [FXCM FIX Api](https://github.com/fxcm/FIXAPI)

  

## c++ style

Follow this cpp style guide [http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

