// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/cmdline.h"
#include "argsy/detail/cmdline.h"
#include "argsy/options_description.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "minitest.hpp"

/* To facilitate testing, declare a number of error codes. Otherwise,
   we'd have to specify the type of exception that should be thrown.
*/

const int s_success                    = 0;
const int s_unknown_option             = 1;
const int s_ambiguous_option           = 2;
const int s_long_not_allowed           = 3;
const int s_long_adjacent_not_allowed  = 4;
const int s_short_adjacent_not_allowed = 5;
const int s_empty_adjacent_parameter   = 6;
const int s_missing_parameter          = 7;
const int s_extra_parameter            = 8;
const int s_unrecognized_line          = 9;

auto translate_syntax_error_kind(argsy::invalid_command_line_syntax::kind_t k) -> int
{
   argsy::invalid_command_line_syntax::kind_t table[] = {
      argsy::invalid_command_line_syntax::long_not_allowed,           argsy::invalid_command_line_syntax::long_adjacent_not_allowed,
      argsy::invalid_command_line_syntax::short_adjacent_not_allowed, argsy::invalid_command_line_syntax::empty_adjacent_parameter,
      argsy::invalid_command_line_syntax::missing_parameter,          argsy::invalid_command_line_syntax::extra_parameter,
      argsy::invalid_command_line_syntax::unrecognized_line};
   argsy::invalid_command_line_syntax::kind_t *b, *e, *i;
   b = table;
   e = table + sizeof(table) / sizeof(table[0]);
   i = std::find(b, e, k);
   assert(i != e);
   return std::distance(b, i) + 3;
}

struct test_case
{
   const char *input;
   int         expected_status;
   const char *expected_result;
};

/* Parses the syntax description in 'syntax' and initialized
   'cmd' accordingly'
   The "boost::argsy" in parameter type is needed because CW9
   has std::detail and it causes an ambiguity.
*/
void apply_syntax(argsy::options_description &desc, const char *syntax)
{
   std::string       s;
   std::stringstream ss;
   ss << syntax;
   while(ss >> s)
   {
      argsy::value_semantic *v = nullptr;

      if(*(s.end() - 1) == '=')
      {
         v = argsy::value<std::string>();
         s.resize(s.size() - 1);
      }
      else if(*(s.end() - 1) == '?')
      {
         v = argsy::value<std::string>()->implicit_value("default");
         s.resize(s.size() - 1);
      }
      else if(*(s.end() - 1) == '*')
      {
         v = argsy::value<std::vector<std::string>>()->multitoken();
         s.resize(s.size() - 1);
      }
      else if(*(s.end() - 1) == '+')
      {
         v = argsy::value<std::vector<std::string>>()->multitoken();
         s.resize(s.size() - 1);
      }
      if(v)
      {
         desc.add_options()(s.c_str(), v, "");
      }
      else
      {
         desc.add_options()(s.c_str(), "");
      }
   }
}

void test_cmdline(const char *syntax, argsy::command_line_style::style_t style, const test_case *cases)
{
   for(int i = 0; cases[i].input; ++i)
   {
      // Parse input
      std::vector<std::string> xinput;
      {
         std::string       s;
         std::stringstream ss;
         ss << cases[i].input;
         while(ss >> s)
         {
            xinput.push_back(s);
         }
      }
      argsy::options_description desc;
      apply_syntax(desc, syntax);

      argsy::detail::cmdline cmd(xinput);
      cmd.style(style);
      cmd.set_options_description(desc);

      std::string result;
      int         status = 0;

      try
      {
         std::vector<argsy::option> options = cmd.run();

         for(auto opt : options)
         {
            if(opt.position_key != -1)
            {
               if(!result.empty())
               {
                  result += " ";
               }
               result += opt.value[0];
            }
            else
            {
               if(!result.empty())
               {
                  result += " ";
               }
               result += opt.string_key + ":";
               for(size_t k = 0; k < opt.value.size(); ++k)
               {
                  if(k != 0)
                  {
                     result += "-";
                  }
                  result += opt.value[k];
               }
            }
         }
      }
      catch(argsy::unknown_option &)
      {
         status = s_unknown_option;
      }
      catch(argsy::ambiguous_option &)
      {
         status = s_ambiguous_option;
      }
      catch(argsy::invalid_command_line_syntax &e)
      {
         status = translate_syntax_error_kind(e.kind());
      }
      BOOST_CHECK_EQUAL(status, cases[i].expected_status);
      BOOST_CHECK_EQUAL(result, cases[i].expected_result);
   }
}

void test_long_options()
{
   auto style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_adjacent);

   test_case test_cases1[] = {// Test that long options are recognized and everything else
                              // is treated like arguments
                              {"--foo foo -123 /asd", s_success, "foo: foo -123 /asd"},

                              // Unknown option
                              {"--unk", s_unknown_option, ""},

                              // Test that abbreviated names do not work
                              {"--fo", s_unknown_option, ""},

                              // Test for disallowed parameter
                              {"--foo=13", s_extra_parameter, ""},

                              // Test option with required parameter
                              {"--bar=", s_empty_adjacent_parameter, ""},
                              {"--bar", s_missing_parameter, ""},

                              {"--bar=123", s_success, "bar:123"},
                              {nullptr, 0, nullptr}};
   test_cmdline("foo bar=", style, test_cases1);

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_next);

   test_case test_cases2[] = {{"--bar 10", s_success, "bar:10"},
                              {"--bar", s_missing_parameter, ""},
                              // Since --bar accepts a parameter, --foo is
                              // considered a value, even though it looks like
                              // an option.
                              {"--bar --foo", s_success, "bar:--foo"},
                              {nullptr, 0, nullptr}};
   test_cmdline("foo bar=", style, test_cases2);
   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_adjacent |
                                           argsy::command_line_style::long_allow_next);

   test_case test_cases3[] = {{"--bar=10", s_success, "bar:10"}, {"--bar 11", s_success, "bar:11"}, {nullptr, 0, nullptr}};
   test_cmdline("foo bar=", style, test_cases3);

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_adjacent |
                                           argsy::command_line_style::long_allow_next | argsy::command_line_style::case_insensitive);

   // Test case insensitive style.
   // Note that option names are normalized to lower case.
   test_case test_cases4[] = {{"--foo", s_success, "foo:"},      {"--Foo", s_success, "foo:"}, {"--bar=Ab", s_success, "bar:Ab"},
                              {"--Bar=ab", s_success, "bar:ab"}, {"--giz", s_success, "Giz:"}, {nullptr, 0, nullptr}};
   test_cmdline("foo bar= baz? Giz", style, test_cases4);
}

void test_short_options()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::allow_dash_for_short |
                                           argsy::command_line_style::short_allow_adjacent);

   test_case test_cases1[] = {{"-d d /bar", s_success, "-d: d /bar"},
                              // This is treated as error when long options are disabled
                              {"--foo", s_success, "--foo"},
                              {"-d13", s_extra_parameter, ""},
                              {"-f14", s_success, "-f:14"},
                              {"-g -f1", s_success, "-g: -f:1"},
                              {"-f", s_missing_parameter, ""},
                              {nullptr, 0, nullptr}};
   test_cmdline(",d ,f= ,g", style, test_cases1);

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::allow_dash_for_short |
                                           argsy::command_line_style::short_allow_next);

   test_case test_cases2[] = {{"-f 13", s_success, "-f:13"},     {"-f -13", s_success, "-f:-13"},    {"-f", s_missing_parameter, ""},
                              {"-f /foo", s_success, "-f:/foo"}, {"-f -d", s_missing_parameter, ""}, {nullptr, 0, nullptr}};
   test_cmdline(",d ,f=", style, test_cases2);

   style =
      argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_next |
                                      argsy::command_line_style::allow_dash_for_short | argsy::command_line_style::short_allow_adjacent);

   test_case test_cases3[] = {
      {"-f10", s_success, "-f:10"}, {"-f 10", s_success, "-f:10"}, {"-f -d", s_missing_parameter, ""}, {nullptr, 0, nullptr}};
   test_cmdline(",d ,f=", style, test_cases3);

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_next |
                                           argsy::command_line_style::allow_dash_for_short |
                                           argsy::command_line_style::short_allow_adjacent | argsy::command_line_style::allow_sticky);

   test_case test_cases4[] = {{"-de", s_success, "-d: -e:"},
                              {"-df10", s_success, "-d: -f:10"},
                              // FIXME: review
                              //{"-d12", s_extra_parameter, ""},
                              {"-f12", s_success, "-f:12"},
                              {"-fe", s_success, "-f:e"},
                              {nullptr, 0, nullptr}};
   test_cmdline(",d ,f= ,e", style, test_cases4);
}

void test_dos_options()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::allow_slash_for_short |
                                           argsy::command_line_style::short_allow_adjacent);

   test_case test_cases1[] = {{"/d d -bar", s_success, "-d: d -bar"}, {"--foo", s_success, "--foo"},   {"/d13", s_extra_parameter, ""},
                              {"/f14", s_success, "-f:14"},           {"/f", s_missing_parameter, ""}, {nullptr, 0, nullptr}};
   test_cmdline(",d ,f=", style, test_cases1);

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::allow_slash_for_short |
                                           argsy::command_line_style::short_allow_next | argsy::command_line_style::short_allow_adjacent |
                                           argsy::command_line_style::allow_sticky);

   test_case test_cases2[] = {{"/de", s_extra_parameter, ""}, {"/fe", s_success, "-f:e"}, {nullptr, 0, nullptr}};
   test_cmdline(",d ,f= ,e", style, test_cases2);
}

void test_disguised_long()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_adjacent |
                                           argsy::command_line_style::allow_dash_for_short | argsy::command_line_style::short_allow_next |
                                           argsy::command_line_style::allow_long_disguise | argsy::command_line_style::long_allow_adjacent);

   test_case test_cases1[] = {{"-foo -f", s_success, "foo: foo:"},
                              {"-goo=x -gy", s_success, "goo:x goo:y"},
                              {"-bee=x -by", s_success, "bee:x bee:y"},
                              {nullptr, 0, nullptr}};
   test_cmdline("foo,f goo,g= bee,b?", style, test_cases1);

   style                   = argsy::detail::cmdline::style_t(style | argsy::command_line_style::allow_slash_for_short);
   test_case test_cases2[] = {{"/foo -f", s_success, "foo: foo:"}, {"/goo=x", s_success, "goo:x"}, {nullptr, 0, nullptr}};
   test_cmdline("foo,f goo,g= bee,b?", style, test_cases2);
}

void test_guessing()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_adjacent |
                                           argsy::command_line_style::allow_dash_for_short | argsy::command_line_style::allow_long |
                                           argsy::command_line_style::long_allow_adjacent | argsy::command_line_style::allow_guessing |
                                           argsy::command_line_style::allow_long_disguise);

   test_case test_cases1[] = {{"--opt1", s_success, "opt123:"},
                              {"--opt", s_ambiguous_option, ""},
                              {"--f=1", s_success, "foo:1"},
                              {"-far", s_success, "foo:ar"},
                              {nullptr, 0, nullptr}};
   test_cmdline("opt123 opt56 foo,f=", style, test_cases1);

   test_case test_cases2[] = {{"--fname file --fname2 file2", s_success, "fname: file fname2: file2"},
                              {"--fnam file --fnam file2", s_ambiguous_option, ""},
                              {"--fnam file --fname2 file2", s_ambiguous_option, ""},
                              {"--fname2 file2 --fnam file", s_ambiguous_option, ""},
                              {nullptr, 0, nullptr}};
   test_cmdline("fname fname2", style, test_cases2);
}

void test_arguments()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(
      argsy::command_line_style::allow_short | argsy::command_line_style::allow_long | argsy::command_line_style::allow_dash_for_short |
      argsy::command_line_style::short_allow_adjacent | argsy::command_line_style::long_allow_adjacent);

   test_case test_cases1[] = {
      {"-f file -gx file2", s_success, "-f: file -g:x file2"}, {"-f - -gx - -- -e", s_success, "-f: - -g:x - -e"}, {nullptr, 0, nullptr}};
   test_cmdline(",f ,g= ,e", style, test_cases1);

   // "--" should stop options regardless of whether long options are
   // allowed or not.

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_adjacent |
                                           argsy::command_line_style::allow_dash_for_short);

   test_case test_cases2[] = {{"-f - -gx - -- -e", s_success, "-f: - -g:x - -e"}, {nullptr, 0, nullptr}};
   test_cmdline(",f ,g= ,e", style, test_cases2);
}

void test_prefix()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(
      argsy::command_line_style::allow_short | argsy::command_line_style::allow_long | argsy::command_line_style::allow_dash_for_short |
      argsy::command_line_style::short_allow_adjacent | argsy::command_line_style::long_allow_adjacent);

   test_case test_cases1[] = {{"--foo.bar=12", s_success, "foo.bar:12"}, {nullptr, 0, nullptr}};

   test_cmdline("foo*=", style, test_cases1);
}

auto at_option_parser(std::string const &s) -> std::pair<std::string, std::string>
{
   if('@' == s[0])
   {
      return std::make_pair(std::string("response-file"), s.substr(1));
   }
   else
   {
      return std::pair<std::string, std::string>();
   }
}

auto at_option_parser_broken(std::string const &s) -> std::pair<std::string, std::string>
{
   if('@' == s[0])
   {
      return std::make_pair(std::string("some garbage"), s.substr(1));
   }
   else
   {
      return std::pair<std::string, std::string>();
   }
}

void test_additional_parser()
{
   argsy::options_description desc;
   desc.add_options()("response-file", argsy::value<std::string>(), "response file")("foo", argsy::value<int>(),
                                                                                     "foo")("bar,baz", argsy::value<int>(), "bar");

   std::vector<std::string> input;
   input.emplace_back("@config");
   input.emplace_back("--foo=1");
   input.emplace_back("--baz=11");

   argsy::detail::cmdline cmd(input);
   cmd.set_options_description(desc);
   cmd.set_additional_parser(at_option_parser);

   std::vector<argsy::option> result = cmd.run();

   BOOST_REQUIRE(result.size() == 3);
   BOOST_CHECK_EQUAL(result[0].string_key, "response-file");
   BOOST_CHECK_EQUAL(result[0].value[0], "config");
   BOOST_CHECK_EQUAL(result[1].string_key, "foo");
   BOOST_CHECK_EQUAL(result[1].value[0], "1");
   BOOST_CHECK_EQUAL(result[2].string_key, "bar");
   BOOST_CHECK_EQUAL(result[2].value[0], "11");

   // Test that invalid options returned by additional style
   // parser are detected.
   argsy::detail::cmdline cmd2(input);
   cmd2.set_options_description(desc);
   cmd2.set_additional_parser(at_option_parser_broken);

   BOOST_CHECK_THROW(cmd2.run(), argsy::unknown_option);
}

auto at_option_parser2(std::vector<std::string> &args) -> std::vector<argsy::option>
{
   std::vector<argsy::option> result;
   if('@' == args[0][0])
   {
      // Simulate reading the response file.
      result.emplace_back("foo", std::vector<std::string>(1, "1"));
      result.emplace_back("bar", std::vector<std::string>(1, "1"));
      args.erase(args.begin());
   }
   return result;
}

void test_style_parser()
{
   argsy::options_description desc;
   desc.add_options()("foo", argsy::value<int>(), "foo")("bar", argsy::value<int>(), "bar");

   std::vector<std::string> input;
   input.emplace_back("@config");

   argsy::detail::cmdline cmd(input);
   cmd.set_options_description(desc);
   cmd.extra_style_parser(at_option_parser2);

   std::vector<argsy::option> result = cmd.run();

   BOOST_REQUIRE(result.size() == 2);
   BOOST_CHECK_EQUAL(result[0].string_key, "foo");
   BOOST_CHECK_EQUAL(result[0].value[0], "1");
   BOOST_CHECK_EQUAL(result[1].string_key, "bar");
   BOOST_CHECK_EQUAL(result[1].value[0], "1");
}

void test_unregistered()
{
   // Check unregisted option when no options are registed at all.
   argsy::options_description desc;

   std::vector<std::string> input;
   input.emplace_back("--foo=1");
   input.emplace_back("--bar");
   input.emplace_back("1");
   input.emplace_back("-b");
   input.emplace_back("-biz");

   argsy::detail::cmdline cmd(input);
   cmd.set_options_description(desc);
   cmd.allow_unregistered();

   std::vector<argsy::option> result = cmd.run();
   BOOST_REQUIRE(result.size() == 5);
   // --foo=1
   BOOST_CHECK_EQUAL(result[0].string_key, "foo");
   BOOST_CHECK_EQUAL(result[0].unregistered, true);
   BOOST_CHECK_EQUAL(result[0].value[0], "1");
   // --bar
   BOOST_CHECK_EQUAL(result[1].string_key, "bar");
   BOOST_CHECK_EQUAL(result[1].unregistered, true);
   BOOST_CHECK(result[1].value.empty());
   // '1' is considered a positional option, not a value to
   // --bar
   BOOST_CHECK(result[2].string_key.empty());
   BOOST_CHECK(result[2].position_key == 0);
   BOOST_CHECK_EQUAL(result[2].unregistered, false);
   BOOST_CHECK_EQUAL(result[2].value[0], "1");
   // -b
   BOOST_CHECK_EQUAL(result[3].string_key, "-b");
   BOOST_CHECK_EQUAL(result[3].unregistered, true);
   BOOST_CHECK(result[3].value.empty());
   // -biz
   BOOST_CHECK_EQUAL(result[4].string_key, "-b");
   BOOST_CHECK_EQUAL(result[4].unregistered, true);
   BOOST_CHECK_EQUAL(result[4].value[0], "iz");

   // Check sticky short options together with unregisted options.

   desc.add_options()("help,h", "")("magic,m", argsy::value<std::string>(), "");

   input.clear();
   input.emplace_back("-hc");
   input.emplace_back("-mc");

   argsy::detail::cmdline cmd2(input);
   cmd2.set_options_description(desc);
   cmd2.allow_unregistered();

   result = cmd2.run();

   BOOST_REQUIRE(result.size() == 3);
   BOOST_CHECK_EQUAL(result[0].string_key, "help");
   BOOST_CHECK_EQUAL(result[0].unregistered, false);
   BOOST_CHECK(result[0].value.empty());
   BOOST_CHECK_EQUAL(result[1].string_key, "-c");
   BOOST_CHECK_EQUAL(result[1].unregistered, true);
   BOOST_CHECK(result[1].value.empty());
   BOOST_CHECK_EQUAL(result[2].string_key, "magic");
   BOOST_CHECK_EQUAL(result[2].unregistered, false);
   BOOST_CHECK_EQUAL(result[2].value[0], "c");

   // CONSIDER:
   // There's a corner case:
   //   -foo
   // when 'allow_long_disguise' is set. Should this be considered
   // disguised long option 'foo' or short option '-f' with value 'oo'?
   // It's not clear yet, so I'm leaving the decision till later.
}

void test_implicit_value()
{
   argsy::detail::cmdline::style_t style;

   style = argsy::detail::cmdline::style_t(argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_adjacent);

   test_case test_cases1[] = {// 'bar' does not even look like option, so is consumed
                              {"--foo bar", s_success, "foo:bar"},
                              // '--bar' looks like option, and such option exists, so we don't consume this token
                              {"--foo --bar", s_success, "foo: bar:"},
                              // '--biz' looks like option, but does not match any existing one.
                              // Presently this results in parse error, since
                              // (1) in cmdline.cpp:finish_option, we only consume following tokens if they are
                              // requires
                              // (2) in cmdline.cpp:run, we let options consume following positional options
                              // For --biz, an exception is thrown between 1 and 2.
                              // We might want to fix that in future.
                              {"--foo --biz", s_unknown_option, ""},
                              {nullptr, 0, nullptr}};

   test_cmdline("foo? bar?", style, test_cases1);
}

auto main(int /*ac*/, char * * /*av*/) -> int
{
   test_long_options();
   test_short_options();
   test_dos_options();
   test_disguised_long();
   test_guessing();
   test_arguments();
   test_prefix();
   test_additional_parser();
   test_style_parser();
   test_unregistered();
   test_implicit_value();

   return 0;
}
