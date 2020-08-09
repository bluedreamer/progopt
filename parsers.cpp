// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "cmdline.h"
#include "config_file.h"
#include <convert.h>
#include <environment_iterator.h>
#include <options_description.h>
#include <parsers.h>
#include <positional_options.h>

#include <cctype>
#include <fstream>

#include <iostream>
#include <unistd.h>
#include <utility>

auto woption_from_option(const option &opt) -> woption
{
   woption result;
   result.string_key   = opt.string_key;
   result.position_key = opt.position_key;
   result.unregistered = opt.unregistered;

   std::transform(opt.value.begin(), opt.value.end(), back_inserter(result.value), std::bind(from_utf8, std::placeholders::_1));

   std::transform(opt.original_tokens.begin(), opt.original_tokens.end(), back_inserter(result.original_tokens),
                  std::bind(from_utf8, std::placeholders::_1));
   return result;
}

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

template<class charT>
auto parse_config_file(std::basic_istream<charT> &is, const options_description &desc, bool allow_unregistered)
   -> basic_parsed_options<charT>
{
   std::set<std::string> allowed_options;

   const auto &options = desc.options();
   for(const auto &option : options)
   {
      const option_description &d = option;

      if(d.long_name().empty())
      {
         boost::throw_exception(error("abbreviated option names are not permitted in options configuration files"));
      }

      allowed_options.insert(d.long_name());
   }

   // Parser return char strings
   parsed_options result(&desc);
   // TODO must put this back
   //   copy(basic_config_file_iterator<charT>(is, allowed_options, allow_unregistered), basic_config_file_iterator<charT>(),
   //        back_inserter(result.options));
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
    BOOST_PROGRAM_OPTIONS_DECL parsed_options
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
#if 0 // TODO fix the iterator shite
   for(environment_iterator i(environ), e; i != e; ++i)
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
#endif
   return result;
}

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

auto parse_environment(const options_description &desc, const std::string &prefix) -> parsed_options
{
   return parse_environment(desc, prefix_name_mapper(prefix));
}

auto parse_environment(const options_description &desc, const char *prefix) -> parsed_options
{
   return parse_environment(desc, std::string(prefix));
}
