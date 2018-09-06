# IDEFIX
Integrated Development Environment for Financial Information Exchange

## Style Guide
Follow this cpp style guide [http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)

## Dependencies

- C++11 Compiler
- CMake 3.9+
- quickfix https://github.com/karopawil/quickfix
- spdlog https://github.com/gabime/spdlog
- anyStock https://www.anychart.com/
- nod https://github.com/fr00b0/nod

## anyChart/anyStock
The chart data will be written to public_html/ folder. The filenames are like:

EURUSD_bars.csv

When idefix is startet these files will be removed to prevent issues in drawing gaps.