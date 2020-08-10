// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
#include "argsy/config.hpp"
#include "argsy/detail/cmdline.hpp"
#include "argsy/detail/config_file.hpp"
#include "argsy/detail/convert.hpp"
#include "argsy/environment_iterator.hpp"
#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"
#include "argsy/positional_options.hpp"

#include <cctype>
#include <fstream>
#include <istream>
#include <utility>

#include <unistd.h>

// The 'environ' should be declared in some cases. E.g. Linux man page says:
// (This variable must be declared in the user program, but is declared in
// the header file unistd.h in case the header files came from libc4 or libc5,
// and in case they came from glibc and _GNU_SOURCE was defined.)
// To be safe, declare it here.

// It appears that on Mac OS X the 'environ' variable is not
// available to dynamically linked libraries.
// See: http://article.gmane.org/gmane.comp.lib.boost.devel/103843
// See: http://lists.gnu.org/archive/html/bug-guile/2004-01/msg00013.html
#if defined(__APPLE__) && defined(__DYNAMIC__)
// The proper include for this is crt_externs.h, however it's not
// available on iOS. The right replacement is not known. See
// https://svn.boost.org/trac/boost/ticket/5053
extern "C"
{
   extern char ***_NSGetEnviron(void);
}
   #define environ (*_NSGetEnviron())
#else
   #if defined(__MWERKS__)
      #include <crtl.h>
   #else
      #if !defined(_WIN32) || defined(__COMO_VERSION__)
extern char **environ;
      #endif
   #endif
#endif

namespace argsy
{
#ifndef BOOST_NO_STD_WSTRING
namespace
{
auto woption_from_option(const option &opt) -> woption
{
   woption result;
   result.string_key   = opt.string_key;
   result.position_key = opt.position_key;
   result.unregistered = opt.unregistered;

   //            std::transform(opt.value.begin(), opt.value.end(),
   //                           back_inserter(result.value),
   //                           boost::bind(from_utf8, _1));
   //
   //            std::transform(opt.original_tokens.begin(),
   //                           opt.original_tokens.end(),
   //                           back_inserter(result.original_tokens),
   //                           boost::bind(from_utf8, _1));
   return result;
}
} // namespace

basic_parsed_options<wchar_t>::basic_parsed_options(const parsed_options &po)
   : description(po.description)
   , utf8_encoded_options(po)
   , m_options_prefix(po.m_options_prefix)
{
   for(const auto &option : po.options)
   {
      options.push_back(woption_from_option(option));
   }
}
#endif

template<class charT>
auto parse_config_file(std::basic_istream<charT> &is, const options_description &desc, bool allow_unregistered)
   -> basic_parsed_options<charT>
{
   std::set<std::string> allowed_options;

   const std::vector<std::shared_ptr<option_description>> &options = desc.options();
   for(const auto &option : options)
   {
      const option_description &d = *option;

      if(d.long_name().empty())
      {
         boost::throw_exception(error("abbreviated option names are not permitted in options configuration files"));
      }

      allowed_options.insert(d.long_name());
   }

   // Parser return char strings
   parsed_options result(&desc);
   copy(detail::basic_config_file_iterator<charT>(is, allowed_options, allow_unregistered), detail::basic_config_file_iterator<charT>(),
        back_inserter(result.options));
   // Convert char strings into desired type.
   return basic_parsed_options<charT>(result);
}

template basic_parsed_options<char> parse_config_file(std::basic_istream<char> &is, const options_description &desc,
                                                      bool allow_unregistered);

#ifndef BOOST_NO_STD_WSTRING
template basic_parsed_options<wchar_t> parse_config_file(std::basic_istream<wchar_t> &is, const options_description &desc,
                                                         bool allow_unregistered);
#endif

template<class charT>
auto parse_config_file(const char *filename, const options_description &desc, bool allow_unregistered) -> basic_parsed_options<charT>
{
   // Parser return char strings
   std::basic_ifstream<charT> strm(filename);
   if(!strm)
   {
      boost::throw_exception(reading_file(filename));
   }

   basic_parsed_options<charT> result = parse_config_file(strm, desc, allow_unregistered);

   if(strm.bad())
   {
      boost::throw_exception(reading_file(filename));
   }

   return result;
}

template basic_parsed_options<char> parse_config_file(const char *filename, const options_description &desc, bool allow_unregistered);

#ifndef BOOST_NO_STD_WSTRING
template basic_parsed_options<wchar_t> parse_config_file(const char *filename, const options_description &desc, bool allow_unregistered);
#endif

// This versio, which accepts any options without validation, is disabled,
// in the hope that nobody will need it and we cant drop it altogether.
// Besides, probably the right way to handle all options is the '*' name.
#if 0
    parsed_options
    parse_config_file(std::istream& is)
    {
        detail::config_file_iterator cf(is, false);
        parsed_options result(0);
        copy(cf, detail::config_file_iterator(), 
             back_inserter(result.options));
        return result;
    }
#endif

auto parse_environment(const options_description &desc, const std::function<std::string(std::string)> &name_mapper) -> parsed_options
{
   parsed_options result(&desc);

   for(boost::environment_iterator i(environ), e; i != e; ++i)
   {
      std::string option_name = name_mapper(i->first);

      if(!option_name.empty())
      {
         option n;
         n.string_key = option_name;
         n.value.push_back(i->second);
         result.options.push_back(n);
      }
   }

   return result;
}

namespace detail
{
class prefix_name_mapper
{
public:
   prefix_name_mapper(std::string prefix)
      : prefix(std::move(prefix))
   {
   }

   auto operator()(const std::string &s) -> std::string
   {
      std::string result;
      if(s.find(prefix) == 0)
      {
         for(std::string::size_type n = prefix.size(); n < s.size(); ++n)
         {
            // Intel-Win-7.1 does not understand
            // push_back on string.
            result += static_cast<char>(tolower(s[n]));
         }
      }
      return result;
   }

private:
   std::string prefix;
};
} // namespace detail

auto parse_environment(const options_description &desc, const std::string &prefix) -> parsed_options
{
   return parse_environment(desc, detail::prefix_name_mapper(prefix));
}

auto parse_environment(const options_description &desc, const char *prefix) -> parsed_options
{
   return parse_environment(desc, std::string(prefix));
}

} // namespace argsy
