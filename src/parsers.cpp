// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
#include "argsy/detail/cmdline.h"
#include "argsy/detail/config_file.h"
#include "argsy/environment_iterator.h"
#include "argsy/parsers.h"

#include <cctype>
#include <fstream>
#include <istream>

extern char **environ;

namespace argsy
{
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

template basic_parsed_options<wchar_t> parse_config_file(std::basic_istream<wchar_t> &is, const options_description &desc,
                                                         bool allow_unregistered);

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
template basic_parsed_options<wchar_t> parse_config_file(const char *filename, const options_description &desc, bool allow_unregistered);

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
