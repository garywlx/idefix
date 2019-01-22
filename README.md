# IDEFIX
Integrated Development Environment for Financial Information Exchange

## Style Guide
Follow this cpp style guide [http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

## Dependencies

- C++11 Compiler
- CMake 3.10+
- quickfix https://github.com/karopawil/quickfix
- Simple-WebSocket-Server https://gitlab.com/eidheim/Simple-WebSocket-Server
- spdlog https://github.com/gabime/spdlog
- nod https://github.com/fr00b0/nod

## Build
### Custom Compiler Definitions

- CMAKE_PROJECT_VERSION # sets project version. this is used in code for version printing.


### Debug
To build a debug binary, you can use the `scripts/debug.sh` script or run:

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make
```

In Debug mode the console output goes to stdout.

### Release
To build a release binary, you can use the `scripts/release.sh` script or run:

```bash
$ mkdir build
$ cd build 
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
$ sudo make install
```

In Release mode the console output for debugging goes to a file `release.log` in the folder where you have executed idefix.

## Run

```bash
$ idefix 
```
