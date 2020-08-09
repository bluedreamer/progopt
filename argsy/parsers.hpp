// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "argsy/config.hpp"
#include "argsy/detail/cmdline.hpp"
#include "argsy/option.hpp"

#include <boost/function/function1.hpp>

#include <iosfwd>
#include <utility>
#include <vector>

namespace argsy
{
class options_description;
class positional_options_description;

/** Results of parsing an input source.
    The primary use of this class is passing information from parsers
    component to value storage component. This class does not makes
    much sense itself.
*/
template<class charT>
class basic_parsed_options
{
public:
   explicit basic_parsed_options(const options_description *xdescription, int options_prefix = 0)
      : description(xdescription)
      , m_options_prefix(options_prefix)
   {
   }
   /** Options found in the source. */
   std::vector<basic_option<charT>> options;
   /** Options description that was used for parsing.
       Parsers should return pointer to the instance of
       option_description passed to them, and issues of lifetime are
       up to the caller. Can be NULL.
    */
   const options_description *description;

   /** Mainly used for the diagnostic messages in exceptions.
    *  The canonical option prefix  for the parser which generated these results,
    *  depending on the settings for basic_command_line_parser::style() or
    *  cmdline::style(). In order of precedence of command_line_style enums:
    *      allow_long
    *      allow_long_disguise
    *      allow_dash_for_short
    *      allow_slash_for_short
    */
   int m_options_prefix;
};

/** Specialization of basic_parsed_options which:
    - provides convenient conversion from basic_parsed_options<char>
    - stores the passed char-based options for later use.
*/
template<>
class basic_parsed_options<wchar_t>
{
public:
   /** Constructs wrapped options from options in UTF8 encoding. */
   explicit basic_parsed_options(const basic_parsed_options<char> &po);

   std::vector<basic_option<wchar_t>> options;
   const options_description *        description;

   /** Stores UTF8 encoded options that were passed to constructor,
       to avoid reverse conversion in some cases. */
   basic_parsed_options<char> utf8_encoded_options;

   /** Mainly used for the diagnostic messages in exceptions.
    *  The canonical option prefix  for the parser which generated these results,
    *  depending on the settings for basic_command_line_parser::style() or
    *  cmdline::style(). In order of precedence of command_line_style enums:
    *      allow_long
    *      allow_long_disguise
    *      allow_dash_for_short
    *      allow_slash_for_short
    */
   int m_options_prefix;
};

using parsed_options  = basic_parsed_options<char>;
using wparsed_options = basic_parsed_options<wchar_t>;

/** Augments basic_parsed_options<wchar_t> with conversion from
    'parsed_options' */

typedef boost::function1<std::pair<std::string, std::string>, const std::string &> ext_parser;

/** Command line parser.

    The class allows one to specify all the information needed for parsing
    and to parse the command line. It is primarily needed to
    emulate named function parameters \-- a regular function with 5
    parameters will be hard to use and creating overloads with a smaller
    number of parameters will be confusing.

    For the most common case, the function parse_command_line is a better
    alternative.

    There are two typedefs \-- command_line_parser and wcommand_line_parser,
    for charT == char and charT == wchar_t cases.
*/
template<class charT>
class basic_command_line_parser : private detail::cmdline
{
public:
   /** Creates a command line parser for the specified arguments
       list. The 'args' parameter should not include program name.
   */
   basic_command_line_parser(const std::vector<std::basic_string<charT>> &args);
   /** Creates a command line parser for the specified arguments
       list. The parameters should be the same as passed to 'main'.
   */
   basic_command_line_parser(int argc, const charT *const argv[]);

   /** Sets options descriptions to use. */
   auto options(const options_description &desc) -> basic_command_line_parser &;
   /** Sets positional options description to use. */
   auto positional(const positional_options_description &desc) -> basic_command_line_parser &;

   /** Sets the command line style. */
   auto style(int) -> basic_command_line_parser &;
   /** Sets the extra parsers. */
   auto extra_parser(ext_parser) -> basic_command_line_parser &;

   /** Parses the options and returns the result of parsing.
       Throws on error.
   */
   auto run() -> basic_parsed_options<charT>;

   /** Specifies that unregistered options are allowed and should
       be passed though. For each command like token that looks
       like an option but does not contain a recognized name, an
       instance of basic_option<charT> will be added to result,
       with 'unrecognized' field set to 'true'. It's possible to
       collect all unrecognized options with the 'collect_unrecognized'
       funciton.
   */
   auto allow_unregistered() -> basic_command_line_parser &;

   using detail::cmdline::style_parser;

   auto extra_style_parser(style_parser s) -> basic_command_line_parser &;

private:
   const options_description *m_desc;
};

using command_line_parser  = basic_command_line_parser<char>;
using wcommand_line_parser = basic_command_line_parser<wchar_t>;

/** Creates instance of 'command_line_parser', passes parameters to it,
    and returns the result of calling the 'run' method.
 */
template<class charT>
auto parse_command_line(int argc, const charT *const argv[], const options_description & /*desc*/, int style = 0,
                        boost::function1<std::pair<std::string, std::string>, const std::string &> ext = ext_parser())
   -> basic_parsed_options<charT>;

/** Parse a config file.

    Read from given stream.
*/
template<class charT>
   auto
   parse_config_file(std::basic_istream<charT> &, const options_description &, bool allow_unregistered = false)
      -> basic_parsed_options<charT>;

/** Parse a config file.

    Read from file with the given name. The character type is
    passed to the file stream.
*/
template<class charT = char>
   auto
   parse_config_file(const char *filename, const options_description &, bool allow_unregistered = false) -> basic_parsed_options<charT>;

/** Controls if the 'collect_unregistered' function should
    include positional options, or not. */
enum collect_unrecognized_mode
{
   include_positional,
   exclude_positional
};

/** Collects the original tokens for all named options with
    'unregistered' flag set. If 'mode' is 'include_positional'
    also collects all positional options.
    Returns the vector of origianl tokens for all collected
    options.
*/
template<class charT>
auto collect_unrecognized(const std::vector<basic_option<charT>> &options, enum collect_unrecognized_mode mode)
   -> std::vector<std::basic_string<charT>>;

/** Parse environment.

    For each environment variable, the 'name_mapper' function is called to
    obtain the option name. If it returns empty string, the variable is
    ignored.

    This is done since naming of environment variables is typically
    different from the naming of command line options.
*/
auto parse_environment(const options_description &,
                                                  const boost::function1<std::string, std::string> &name_mapper) -> parsed_options;

/** Parse environment.

    Takes all environment variables which start with 'prefix'. The option
    name is obtained from variable name by removing the prefix and
    converting the remaining string into lower case.
*/
auto parse_environment(const options_description &, const std::string &prefix) -> parsed_options;

/** @overload
    This function exists to resolve ambiguity between the two above
    functions when second argument is of 'char*' type. There's implicit
    conversion to both function1 and string.
*/
auto parse_environment(const options_description &, const char *prefix) -> parsed_options;

/** Splits a given string to a collection of single strings which
    can be passed to command_line_parser. The second parameter is
    used to specify a collection of possible seperator chars used
    for splitting. The seperator is defaulted to space " ".
    Splitting is done in a unix style way, with respect to quotes '"'
    and escape characters '\'
*/
auto split_unix(const std::string &cmdline, const std::string &seperator = " \t",
                                           const std::string &quote = "'\"", const std::string &escape = "\\") -> std::vector<std::string>;

#ifndef BOOST_NO_STD_WSTRING
/** @overload */
auto split_unix(const std::wstring &cmdline, const std::wstring &seperator = L" \t",
                                           const std::wstring &quote = L"'\"", const std::wstring &escape = L"\\")
   -> std::vector<std::wstring>;
#endif

#ifdef _WIN32
/** Parses the char* string which is passed to WinMain function on
    windows. This function is provided for convenience, and because it's
    not clear how to portably access split command line string from
    runtime library and if it always exists.
    This function is available only on Windows.
*/
std::vector<std::string> split_winmain(const std::string &cmdline);

   #ifndef BOOST_NO_STD_WSTRING
/** @overload */
std::vector<std::wstring> split_winmain(const std::wstring &cmdline);
   #endif
#endif

} // namespace argsy

#undef DECL
#include "argsy/detail/parsers.hpp"
