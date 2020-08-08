// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <locale>
#include <string>
#include <vector>
// for mbstate_t
#include <cwchar>
#include <stdexcept>

/** Converts from local 8 bit encoding into wchar_t string using
    the specified locale facet. */
auto from_8_bit(const std::string &s, const std::codecvt<wchar_t, char, std::mbstate_t> &cvt) -> std::wstring;

/** Converts from wchar_t string into local 8 bit encoding into using
    the specified locale facet. */
auto to_8_bit(const std::wstring &s, const std::codecvt<wchar_t, char, std::mbstate_t> &cvt) -> std::string;

/** Converts 's', which is assumed to be in UTF8 encoding, into wide
    string. */
auto from_utf8(const std::string &s) -> std::wstring;

/** Converts wide string 's' into string in UTF8 encoding. */
auto to_utf8(const std::wstring &s) -> std::string;

/** Converts wide string 's' into local 8 bit encoding determined by
    the current locale. */
auto to_local_8_bit(const std::wstring &s) -> std::string;

/** Converts 's', which is assumed to be in local 8 bit encoding, into wide
    string. */
auto from_local_8_bit(const std::string &s) -> std::wstring;

namespace program_options
{
/** Convert the input string into internal encoding used by
    program_options. Presence of this function allows to avoid
    specializing all methods which access input on wchar_t.
*/
auto to_internal(const std::string &) -> std::string;
/** @overload */
auto to_internal(const std::wstring &) -> std::string;

template<class T>
auto to_internal(const std::vector<T> &s) -> std::vector<std::string>
{
   std::vector<std::string> result;
   for(unsigned i = 0; i < s.size(); ++i) {
      result.push_back(to_internal(s[i]));
}
   return result;
}

} // namespace program_options

#include <string>
#include <vector>
auto to_internal(const std::string &) -> std::string;

template<class T>
auto to_internal(const std::vector<T> &s) -> std::vector<std::string>
{
   std::vector<std::string> result;
   for(unsigned i = 0; i < s.size(); ++i) {
      result.push_back(to_internal(s[i]));
}
   return result;
}
