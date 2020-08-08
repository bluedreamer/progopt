// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "cmdline.h"
#include "detail_cmdline.h"
#include <errors.h>
#include <options_description.h>
#include <positional_options.h>
#include <value_semantic.h>

// TODO get rid of C headers
#include <cassert>
#include <cctype>
#include <climits>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include <cstdio>

#include <iostream>

auto invalid_syntax::get_template(kind_t kind) -> std::string
{
   // Initially, store the message in 'const char*' variable,
   // to avoid conversion to string in all cases.
   const char *msg;
   switch(kind)
   {
      case empty_adjacent_parameter:
         msg = "the argument for option '%canonical_option%' should follow immediately after the equal sign";
         break;
      case missing_parameter:
         msg = "the required argument for option '%canonical_option%' is missing";
         break;
      case unrecognized_line:
         msg = "the options configuration file contains an invalid line '%invalid_line%'";
         break;
      // none of the following are currently used:
      case long_not_allowed:
         msg = "the unabbreviated option '%canonical_option%' is not valid";
         break;
      case long_adjacent_not_allowed:
         msg = "the unabbreviated option '%canonical_option%' does not take any arguments";
         break;
      case short_adjacent_not_allowed:
         msg = "the abbreviated option '%canonical_option%' does not take any arguments";
         break;
      case extra_parameter:
         msg = "option '%canonical_option%' does not take any arguments";
         break;
      default:
         msg = "unknown command line syntax error for '%s'";
   }
   return msg;
}

detail_cmdline::detail_cmdline(const std::vector<std::string> &args)
{
   init(args);
}

detail_cmdline::detail_cmdline(int argc, const char *const *argv)
{
#if defined(BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS)
   vector<string> args;
   copy(argv + 1, argv + argc + !argc, inserter(args, args.end()));
   init(args);
#else
   init(std::vector<std::string>(argv + 1, argv + argc + !argc));
#endif
}

void detail_cmdline::init(const std::vector<std::string> &args)
{
   this->m_args         = args;
   m_style              = style_t::default_style;
   m_desc               = nullptr;
   m_positional         = nullptr;
   m_allow_unregistered = false;
}

void detail_cmdline::style(int style)
{
   if(style == 0)
   {
      style = default_style;
   }

   check_style(style);
   this->m_style = style_t(style);
}

void detail_cmdline::allow_unregistered()
{
   this->m_allow_unregistered = true;
}

void detail_cmdline::check_style(int style) const
{
   bool allow_some_long = (style & allow_long) || (style & allow_long_disguise);

   const char *error = nullptr;
   if(allow_some_long && !(style & long_allow_adjacent) && !(style & long_allow_next))
   {
      error = "boost::program_options misconfiguration: "
              "choose one or other of 'command_line_style::long_allow_next' "
              "(whitespace separated arguments) or "
              "'command_line_style::long_allow_adjacent' ('=' separated arguments) for "
              "long options.";
   }

   if(!error && (style & allow_short) && !(style & short_allow_adjacent) && !(style & short_allow_next))
   {
      error = "boost::program_options misconfiguration: "
              "choose one or other of 'command_line_style::short_allow_next' "
              "(whitespace separated arguments) or "
              "'command_line_style::short_allow_adjacent' ('=' separated arguments) for "
              "short options.";
   }

   if(!error && (style & allow_short) && !(style & allow_dash_for_short) && !(style & allow_slash_for_short))
   {
      error = "boost::program_options misconfiguration: "
              "choose one or other of 'command_line_style::allow_slash_for_short' "
              "(slashes) or 'command_line_style::allow_dash_for_short' (dashes) for "
              "short options.";
   }

   if(error)
      boost::throw_exception(invalid_command_line_style(error));

   // Need to check that if guessing and long disguise are enabled
   // -f will mean the same as -foo
}

auto detail_cmdline::is_style_active(style_t style) const -> bool
{
   return ((m_style & style) ? true : false);
}

void detail_cmdline::set_options_description(const options_description &desc)
{
   m_desc = &desc;
}

void detail_cmdline::set_positional_options(const positional_options_description &positional)
{
   m_positional = &positional;
}

auto detail_cmdline::get_canonical_option_prefix() -> int
{
   if(m_style & allow_long)
   {
      return allow_long;
   }

   if(m_style & allow_long_disguise)
   {
      return allow_long_disguise;
   }

   if((m_style & allow_short) && (m_style & allow_dash_for_short))
   {
      return allow_dash_for_short;
   }

   if((m_style & allow_short) && (m_style & allow_slash_for_short))
   {
      return allow_slash_for_short;
   }

   return 0;
}

auto detail_cmdline::run() -> std::vector<option>
{
   // The parsing is done by having a set of 'style parsers'
   // and trying then in order. Each parser is passed a vector
   // of unparsed tokens and can consume some of them (by
   // removing elements on front) and return a vector of options.
   //
   // We try each style parser in turn, untill some input
   // is consumed. The returned vector of option may contain the
   // result of just syntactic parsing of token, say --foo will
   // be parsed as option with name 'foo', and the style parser
   // is not required to care if that option is defined, and how
   // many tokens the value may take.
   // So, after vector is returned, we validate them.
   assert(m_desc);

   std::vector<style_parser> style_parsers;

   if(m_style_parser)
      style_parsers.push_back(m_style_parser);

   if(m_additional_parser)
      style_parsers.emplace_back(std::bind(&detail_cmdline::handle_additional_parser, this, std::placeholders::_1));

   if(m_style & allow_long)
      style_parsers.emplace_back(std::bind(&detail_cmdline::parse_long_option, this, std::placeholders::_1));

   if((m_style & allow_long_disguise))
      style_parsers.emplace_back(std::bind(&detail_cmdline::parse_disguised_long_option, this, std::placeholders::_1));

   if((m_style & allow_short) && (m_style & allow_dash_for_short))
      style_parsers.emplace_back(std::bind(&detail_cmdline::parse_short_option, this, std::placeholders::_1));

   if((m_style & allow_short) && (m_style & allow_slash_for_short))
      style_parsers.emplace_back(std::bind(&detail_cmdline::parse_dos_option, this, std::placeholders::_1));

   style_parsers.emplace_back(std::bind(&detail_cmdline::parse_terminator, this, std::placeholders::_1));

   std::vector<option>       result;
   std::vector<std::string> &args = m_args;
   while(!args.empty())
   {
      bool ok = false;
      for(unsigned i = 0; i < style_parsers.size(); ++i)
      {
         auto            current_size = static_cast<unsigned>(args.size());
         std::vector<option> next         = style_parsers[i](args);

         // Check that option names
         // are valid, and that all values are in place.
         if(!next.empty())
         {
            std::vector<std::string> e;
            for(unsigned k = 0; k < next.size() - 1; ++k)
            {
               finish_option(next[k], e, style_parsers);
            }
            // For the last option, pass the unparsed tokens
            // so that they can be added to next.back()'s values
            // if appropriate.
            finish_option(next.back(), args, style_parsers);
            for(const auto & j : next)
               result.push_back(j);
         }

         if(args.size() != current_size)
         {
            ok = true;
            break;
         }
      }

      if(!ok)
      {
         option opt;
         opt.value.push_back(args[0]);
         opt.original_tokens.push_back(args[0]);
         result.push_back(opt);
         args.erase(args.begin());
      }
   }

   /* If an key option is followed by a positional option,
      can can consume more tokens (e.g. it's multitoken option),
      give those tokens to it.  */
   std::vector<option> result2;
   for(unsigned i = 0; i < result.size(); ++i)
   {
      result2.push_back(result[i]);
      option &opt = result2.back();

      if(opt.string_key.empty())
         continue;

      const option_description *xd;
      try
      {
         xd = m_desc->find_nothrow(opt.string_key, is_style_active(allow_guessing), is_style_active(long_case_insensitive),
                                   is_style_active(short_case_insensitive));
      }
      catch(error_with_option_name &e)
      {
         // add context and rethrow
         e.add_context(opt.string_key, opt.original_tokens[0], get_canonical_option_prefix());
         throw;
      }

      if(!xd)
         continue;

      unsigned min_tokens = xd->semantic()->min_tokens();
      unsigned max_tokens = xd->semantic()->max_tokens();
      if(min_tokens < max_tokens && opt.value.size() < max_tokens)
      {
         // This option may grab some more tokens.
         // We only allow to grab tokens that are not already
         // recognized as key options.

         int      can_take_more = max_tokens - static_cast<int>(opt.value.size());
         unsigned j             = i + 1;
         for(; can_take_more && j < result.size(); --can_take_more, ++j)
         {
            option &opt2 = result[j];
            if(!opt2.string_key.empty())
               break;

            if(opt2.position_key == INT_MAX)
            {
               // We use INT_MAX to mark positional options that
               // were found after the '--' terminator and therefore
               // should stay positional forever.
               break;
            }

            assert(opt2.value.size() == 1);

            opt.value.push_back(opt2.value[0]);

            assert(opt2.original_tokens.size() == 1);

            opt.original_tokens.push_back(opt2.original_tokens[0]);
         }
         i = j - 1;
      }
   }
   result.swap(result2);

   // Assign position keys to positional options.
   int position_key = 0;
   for(auto & i : result)
   {
      if(i.string_key.empty())
         i.position_key = position_key++;
   }

   if(m_positional)
   {
      unsigned position = 0;
      for(auto & opt : result)
      {
         if(opt.position_key != -1)
         {
            if(position >= m_positional->max_total_count())
            {
               boost::throw_exception(too_many_positional_options_error());
            }
            opt.string_key = m_positional->name_for_position(position);
            ++position;
         }
      }
   }

   // set case sensitive flag
   for(auto & i : result)
   {
      if(i.string_key.size() > 2 || (i.string_key.size() > 1 && i.string_key[0] != '-'))
      {
         // it is a long option
         i.case_insensitive = is_style_active(long_case_insensitive);
      }
      else
      {
         // it is a short option
         i.case_insensitive = is_style_active(short_case_insensitive);
      }
   }

   return result;
}

void detail_cmdline::finish_option(option &opt, std::vector<std::string> &other_tokens, const std::vector<style_parser> &style_parsers)
{
   if(opt.string_key.empty())
      return;

   //
   // Be defensive:
   // will have no original token if option created by handle_additional_parser()
   std::string original_token_for_exceptions = opt.string_key;
   if(opt.original_tokens.size())
      original_token_for_exceptions = opt.original_tokens[0];

   try
   {
      // First check that the option is valid, and get its description.
      const option_description *xd = m_desc->find_nothrow(opt.string_key, is_style_active(allow_guessing),
                                                          is_style_active(long_case_insensitive), is_style_active(short_case_insensitive));

      if(!xd)
      {
         if(m_allow_unregistered)
         {
            opt.unregistered = true;
            return;
         }
         else
         {
            boost::throw_exception(unknown_option());
         }
      }
      const option_description &d = *xd;

      // Canonize the name
      opt.string_key = d.key(opt.string_key);

      // We check that the min/max number of tokens for the option
      // agrees with the number of tokens we have. The 'adjacent_value'
      // (the value in --foo=1) counts as a separate token, and if present
      // must be consumed. The following tokens on the command line may be
      // left unconsumed.
      unsigned min_tokens = d.semantic()->min_tokens();
      unsigned max_tokens = d.semantic()->max_tokens();

      auto present_tokens = static_cast<unsigned>(opt.value.size() + other_tokens.size());

      if(present_tokens >= min_tokens)
      {
         if(!opt.value.empty() && max_tokens == 0)
         {
            boost::throw_exception(invalid_command_line_syntax(invalid_command_line_syntax::extra_parameter));
         }

         // Grab min_tokens values from other_tokens, but only if those tokens
         // are not recognized as options themselves.
         if(opt.value.size() <= min_tokens)
         {
            min_tokens -= static_cast<unsigned>(opt.value.size());
         }
         else
         {
            min_tokens = 0;
         }

         // Everything's OK, move the values to the result.
         for(; !other_tokens.empty() && min_tokens--;)
         {
            // check if extra parameter looks like a known option
            // we use style parsers to check if it is syntactically an option,
            // additionally we check if an option_description exists
            std::vector<option>      followed_option;
            std::vector<std::string> next_token(1, other_tokens[0]);
            for(unsigned i = 0; followed_option.empty() && i < style_parsers.size(); ++i)
            {
               followed_option = style_parsers[i](next_token);
            }
            if(!followed_option.empty())
            {
               original_token_for_exceptions = other_tokens[0];
               const option_description *od =
                  m_desc->find_nothrow(other_tokens[0], is_style_active(allow_guessing), is_style_active(long_case_insensitive),
                                       is_style_active(short_case_insensitive));
               if(od)
                  boost::throw_exception(invalid_command_line_syntax(invalid_command_line_syntax::missing_parameter));
            }
            opt.value.push_back(other_tokens[0]);
            opt.original_tokens.push_back(other_tokens[0]);
            other_tokens.erase(other_tokens.begin());
         }
      }
      else
      {
         boost::throw_exception(invalid_command_line_syntax(invalid_command_line_syntax::missing_parameter));
      }
   }
   // use only original token for unknown_option / ambiguous_option since by definition
   //    they are unrecognised / unparsable
   catch(error_with_option_name &e)
   {
      // add context and rethrow
      e.add_context(opt.string_key, original_token_for_exceptions, get_canonical_option_prefix());
      throw;
   }
}

auto detail_cmdline::parse_long_option(std::vector<std::string> &args) -> std::vector<option>
{
   std::vector<option> result;
   const std::string & tok = args[0];
   if(tok.size() >= 3 && tok[0] == '-' && tok[1] == '-')
   {
      std::string name, adjacent;

      std::string::size_type p = tok.find('=');
      if(p != tok.npos)
      {
         name     = tok.substr(2, p - 2);
         adjacent = tok.substr(p + 1);
         if(adjacent.empty())
            boost::throw_exception(invalid_command_line_syntax(invalid_command_line_syntax::empty_adjacent_parameter, name, name,
                                                               get_canonical_option_prefix()));
      }
      else
      {
         name = tok.substr(2);
      }
      option opt;
      opt.string_key = name;
      if(!adjacent.empty())
         opt.value.push_back(adjacent);
      opt.original_tokens.push_back(tok);
      result.push_back(opt);
      args.erase(args.begin());
   }
   return result;
}

auto detail_cmdline::parse_short_option(std::vector<std::string> &args) -> std::vector<option>
{
   const std::string &tok = args[0];
   if(tok.size() >= 2 && tok[0] == '-' && tok[1] != '-')
   {
      std::vector<option> result;

      std::string name     = tok.substr(0, 2);
      std::string adjacent = tok.substr(2);

      // Short options can be 'grouped', so that
      // "-d -a" becomes "-da". Loop, processing one
      // option at a time. We exit the loop when either
      // we've processed all the token, or when the remainder
      // of token is considered to be value, not further grouped
      // option.
      for(;;)
      {
         const option_description *d;
         try
         {
            d = m_desc->find_nothrow(name, false, false, is_style_active(short_case_insensitive));
         }
         catch(error_with_option_name &e)
         {
            // add context and rethrow
            e.add_context(name, name, get_canonical_option_prefix());
            throw;
         }

         // FIXME: check for 'allow_sticky'.
         if(d && (m_style & allow_sticky) && d->semantic()->max_tokens() == 0 && !adjacent.empty())
         {
            // 'adjacent' is in fact further option.
            option opt;
            opt.string_key = name;
            result.push_back(opt);

            if(adjacent.empty())
            {
               args.erase(args.begin());
               break;
            }

            name = std::string("-") + adjacent[0];
            adjacent.erase(adjacent.begin());
         }
         else
         {
            option opt;
            opt.string_key = name;
            opt.original_tokens.push_back(tok);
            if(!adjacent.empty())
               opt.value.push_back(adjacent);
            result.push_back(opt);
            args.erase(args.begin());
            break;
         }
      }
      return result;
   }
   return std::vector<option>();
}

auto detail_cmdline::parse_dos_option(std::vector<std::string> &args) -> std::vector<option>
{
   std::vector<option> result;
   const std::string & tok = args[0];
   if(tok.size() >= 2 && tok[0] == '/')
   {
      std::string name     = "-" + tok.substr(1, 1);
      std::string adjacent = tok.substr(2);

      option opt;
      opt.string_key = name;
      if(!adjacent.empty())
         opt.value.push_back(adjacent);
      opt.original_tokens.push_back(tok);
      result.push_back(opt);
      args.erase(args.begin());
   }
   return result;
}

auto detail_cmdline::parse_disguised_long_option(std::vector<std::string> &args) -> std::vector<option>
{
   const std::string &tok = args[0];
   if(tok.size() >= 2 && ((tok[0] == '-' && tok[1] != '-') || ((m_style & allow_slash_for_short) && tok[0] == '/')))
   {
      try
      {
         if(m_desc->find_nothrow(tok.substr(1, tok.find('=') - 1), is_style_active(allow_guessing), is_style_active(long_case_insensitive),
                                 is_style_active(short_case_insensitive)))
         {
            args[0].insert(0, "-");
            if(args[0][1] == '/')
               args[0][1] = '-';
            return parse_long_option(args);
         }
      }
      catch(error_with_option_name &e)
      {
         // add context and rethrow
         e.add_context(tok, tok, get_canonical_option_prefix());
         throw;
      }
   }
   return std::vector<option>();
}

auto detail_cmdline::parse_terminator(std::vector<std::string> &args) -> std::vector<option>
{
   std::vector<option> result;
   const std::string & tok = args[0];
   if(tok == "--")
   {
      for(unsigned i = 1; i < args.size(); ++i)
      {
         option opt;
         opt.value.push_back(args[i]);
         opt.original_tokens.push_back(args[i]);
         opt.position_key = INT_MAX;
         result.push_back(opt);
      }
      args.clear();
   }
   return result;
}

auto detail_cmdline::handle_additional_parser(std::vector<std::string> &args) -> std::vector<option>
{
   std::vector<option>                 result;
   std::pair<std::string, std::string> r = m_additional_parser(args[0]);
   if(!r.first.empty())
   {
      option next;
      next.string_key = r.first;
      if(!r.second.empty())
         next.value.push_back(r.second);
      result.push_back(next);
      args.erase(args.begin());
   }
   return result;
}

void detail_cmdline::set_additional_parser(additional_parser p)
{
   m_additional_parser = p;
}

void detail_cmdline::extra_style_parser(style_parser s)
{
   m_style_parser = s;
}
