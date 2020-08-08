// Copyright Vladimir Prus 2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <convert.h>

#include <iterator>

template<class charT>
basic_command_line_parser<charT>::basic_command_line_parser(const std::vector<std::basic_string<charT>> &xargs)
   : detail_cmdline(to_internal(xargs))
{
}

template<class charT>
basic_command_line_parser<charT>::basic_command_line_parser(int argc, const charT *const argv[])
   : detail_cmdline(to_internal(std::vector<std::basic_string<charT>>(argv + 1, argv + argc)))
   , m_desc()
{
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::options(const options_description &desc)
{
   detail_cmdline::set_options_description(desc);
   m_desc = &desc;
   return *this;
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::positional(const positional_options_description &desc)
{
   detail_cmdline::set_positional_options(desc);
   return *this;
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::style(int xstyle)
{
   detail_cmdline::style(xstyle);
   return *this;
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::extra_parser(ext_parser ext)
{
   detail_cmdline::set_additional_parser(ext);
   return *this;
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::allow_unregistered()
{
   detail_cmdline::allow_unregistered();
   return *this;
}

template<class charT>
basic_command_line_parser<charT> &basic_command_line_parser<charT>::extra_style_parser(style_parser s)
{
   detail_cmdline::extra_style_parser(s);
   return *this;
}

template<class charT>
basic_parsed_options<charT> basic_command_line_parser<charT>::run()
{
   // save the canonical prefixes which were used by this cmdline parser
   //    eventually inside the parsed results
   //    This will be handy to format recognisable options
   //    for diagnostic messages if everything blows up much later on
   parsed_options result(m_desc, detail_cmdline::get_canonical_option_prefix());
   result.options = detail_cmdline::run();

   // Presense of parsed_options -> wparsed_options conversion
   // does the trick.
   return basic_parsed_options<charT>(result);
}

template<class charT>
basic_parsed_options<charT> parse_command_line(int argc, const charT *const argv[], const options_description &desc, int style,
                                               std::function<std::pair<std::string, std::string>(const std::string &)> ext)
{
   return basic_command_line_parser<charT>(argc, argv).options(desc).style(style).extra_parser(ext).run();
}

template<class charT>
std::vector<std::basic_string<charT>> collect_unrecognized(const std::vector<basic_option<charT>> &options,
                                                           enum collect_unrecognized_mode          mode)
{
   std::vector<std::basic_string<charT>> result;
   for(unsigned i = 0; i < options.size(); ++i)
   {
      if(options[i].unregistered || (mode == include_positional && options[i].position_key != -1))
      {
         copy(options[i].original_tokens.begin(), options[i].original_tokens.end(), back_inserter(result));
      }
   }
   return result;
}
