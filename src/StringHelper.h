#ifndef IDEFIX_STRING_EXTENSIONS_H
#define IDEFIX_STRING_EXTENSIONS_H

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

namespace IDEFIX {
  namespace str {
    // trim from start (in place)
    //  inline void ltrim(std::string &s) {
    //   s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
    //     return !std::isspace(ch);
    //   }));
    // }

    // // trim from end (in place)
    //  inline void rtrim(std::string &s) {
    //   s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
    //     return !std::isspace(ch);
    //   }).base(), s.end());
    // }

    // // trim from both ends (in place)
    //  inline void trim(std::string &s) {
    //   ltrim(s);
    //   rtrim(s);
    // }

    // // trim from start (copying)
    //  inline std::string ltrim_copy(std::string s) {
    //   ltrim(s);
    //   return s;
    // }

    // // trim from end (copying)
    //  inline std::string rtrim_copy(std::string s) {
    //   rtrim(s);
    //   return s;
    // }

    // // trim from end (copying)
    //  inline std::string trim_copy(std::string s) {
    //   rtrim(s);
    //   return s;
    // }

    /*!
     * trim from left
     *
     * @param char* s
     * @return char*
     */
    // inline char *ltrim(char *s) {
    //   while(isspace(*s)) s++;
    //   return s;
    // }

    // /*!
    //  * trim from right
    //  *
    //  * @param char* s
    //  * @return char*
    //  */
    // inline char *rtrim(char *s) {
    //   char* back = s + strlen(s);
    //   while(isspace(*--back));
    //   *(back+1) = '\0';
    //   return s;
    // }

    // /*!
    //  * trim left and right
    //  *
    //  * @param char* s
    //  * @return char*
    //  */
    // inline char *trim(char *s) {
    //   return rtrim(ltrim(s));
    // }

    /*!
     * Implementation of phps explode(string, delimiter)
     *
     * @param std::string const& s
     * @param char dlim
     * @return std::vector<std::string>
     */
    inline std::vector<std::string> explode(std::string const &s, char dlim) {
      std::vector<std::string> result;
      std::istringstream iss(s);

      for(std::string token; std::getline(iss, token, dlim); ){
        result.push_back(std::move(token));
      }

      return result;
    }
  }; // - ns str
}; // - ns idefix

#endif