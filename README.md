# IDEFIX
Integrated Development Environment for Financial Information Exchange

## Style Guide
Follow this cpp style guide [http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

## Dependencies

- C++11 Compiler
- CMake 3.7+
- quickfix https://github.com/karopawil/quickfix
- spdlog https://github.com/gabime/spdlog
- anyStock https://www.anychart.com/
- nod https://github.com/fr00b0/nod


## anyChart/anyStock
The chart data will be written to public_html/ folder. The filenames are like:

EURUSD_bars.csv

When idefix is startet these files will be removed to prevent issues in drawing gaps.

## Build
### Debug
To build a debug binary, you can use the `scripts/debug.sh` script or run:

```bash
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
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

