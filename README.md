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
### Custom Compiler Definitions

- CMAKE_SHOW_DEBUG_OUTPUT=1 # enables renkobrick brick output
- CMAKE_USE_HTML_CHARTS=1 # writes csv files to public_html and creates chart.html
- CMAKE_PROJECT_VERSION # sets project version. this is used in code for version printing.


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

## Run

You have to set a strategy configuration file. Without the configuration, nothing happens.

```bash
$ screen
$ idefix -s awesome.cfg fxcm.cfg
```

Leave screen session with STRG+A+d. In release mode all debug output goes to `release.log`, which can be watched by:

```bash
$ tail -f release.log
```
