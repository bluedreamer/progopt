// Copyright Sascha Ochsenknecht 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/cmdline.hpp"
#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"
#include "argsy/variables_map.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "minitest.hpp"

void test_ambiguous()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->multitoken(), "the config file")(
      "output,c", argsy::value<std::string>(), "the output file")("output,o", argsy::value<std::string>(), "the output file");

   const char *cmdline[] = {"program", "-c", "file", "-o", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
   }
   catch(argsy::ambiguous_option &e)
   {
      BOOST_CHECK_EQUAL(e.alternatives().size(), 2);
      BOOST_CHECK_EQUAL(e.get_option_name(), "-c");
      BOOST_CHECK_EQUAL(e.alternatives()[0], "cfgfile");
      BOOST_CHECK_EQUAL(e.alternatives()[1], "output");
   }
}

void test_ambiguous_long()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->multitoken(), "the config file")(
      "output,c", argsy::value<std::string>(), "the output file")("output,o", argsy::value<std::string>(), "the output file");

   const char *cmdline[] = {"program", "--cfgfile", "file", "--output", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
   }
   catch(argsy::ambiguous_option &e)
   {
      BOOST_CHECK_EQUAL(e.alternatives().size(), 2);
      BOOST_CHECK_EQUAL(e.get_option_name(), "--output");
      BOOST_CHECK_EQUAL(e.alternatives()[0], "output");
      BOOST_CHECK_EQUAL(e.alternatives()[1], "output");
   }
}

void test_ambiguous_multiple_long_names()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,foo,c", argsy::value<std::string>()->multitoken(),
                      "the config file")("output,foo,o", argsy::value<std::string>(), "the output file");

   const char *cmdline[] = {"program", "--foo", "file"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
   }
   catch(argsy::ambiguous_option &e)
   {
      BOOST_CHECK_EQUAL(e.alternatives().size(), 2);
      BOOST_CHECK_EQUAL(e.get_option_name(), "--foo");
      BOOST_CHECK_EQUAL(e.alternatives()[0], "cfgfile");
      BOOST_CHECK_EQUAL(e.alternatives()[1], "output");
   }
}

void test_unknown_option()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>(), "the configfile");

   const char *cmdline[] = {"program", "-c", "file", "-f", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
   }
   catch(argsy::unknown_option &e)
   {
      BOOST_CHECK_EQUAL(e.get_option_name(), "-f");
      BOOST_CHECK_EQUAL(std::string(e.what()), "unrecognised option '-f'");
   }
}

void test_multiple_values()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->multitoken(), "the config file")("output,o", argsy::value<std::string>(),
                                                                                                 "the output file");

   const char *cmdline[] = {"program", "-o", "fritz", "hugo", "--cfgfile", "file", "c", "-o", "text.out"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
      notify(vm);
   }
   catch(argsy::validation_error &e)
   {
      // TODO: this is currently validation_error, shouldn't it be multiple_values ???
      //
      //   multiple_values is thrown only at one place untyped_value::xparse(),
      //    but I think this can never be reached
      //   because: untyped_value always has one value and this is filtered before reach specific
      //   validation and parsing
      //
      BOOST_CHECK_EQUAL(e.get_option_name(), "--cfgfile");
      BOOST_CHECK_EQUAL(std::string(e.what()), "option '--cfgfile' only takes a single argument");
   }
}

void test_multiple_occurrences()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>(), "the configfile");

   const char *cmdline[] = {"program", "--cfgfile", "file", "-c", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
      notify(vm);
   }
   catch(argsy::multiple_occurrences &e)
   {
      BOOST_CHECK_EQUAL(e.get_option_name(), "--cfgfile");
      BOOST_CHECK_EQUAL(std::string(e.what()), "option '--cfgfile' cannot be specified more than once");
   }
}

void test_multiple_occurrences_with_different_names()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,config-file,c", argsy::value<std::string>(), "the configfile");

   const char *cmdline[] = {"program", "--config-file", "file", "--cfgfile", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
      notify(vm);
   }
   catch(argsy::multiple_occurrences &e)
   {
      BOOST_CHECK((e.get_option_name() == "--cfgfile") || (e.get_option_name() == "--config-file"));
      BOOST_CHECK((std::string(e.what()) == "option '--cfgfile' cannot be specified more than once") ||
                  (std::string(e.what()) == "option '--config-file' cannot be specified more than once"));
   }
}

void test_multiple_occurrences_with_non_key_names()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,config-file,c", argsy::value<std::string>(), "the configfile");

   const char *cmdline[] = {"program", "--config-file", "file", "-c", "anotherfile"};

   argsy::variables_map vm;
   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
      notify(vm);
   }
   catch(argsy::multiple_occurrences &e)
   {
      BOOST_CHECK_EQUAL(e.get_option_name(), "--cfgfile");
      BOOST_CHECK_EQUAL(std::string(e.what()), "option '--cfgfile' cannot be specified more than once");
   }
}

void test_missing_value()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->multitoken(), "the config file")("output,o", argsy::value<std::string>(),
                                                                                                 "the output file");
   // missing value for option '-c'
   const char *cmdline[] = {"program", "-c", "-c", "output.txt"};

   argsy::variables_map vm;

   try
   {
      store(parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc), vm);
      notify(vm);
   }
   catch(argsy::invalid_command_line_syntax &e)
   {
      BOOST_CHECK_EQUAL(e.kind(), argsy::invalid_syntax::missing_parameter);
      BOOST_CHECK_EQUAL(e.tokens(), "--cfgfile");
   }
}

auto main(int /*ac*/, char * * /*av*/) -> int
{
   test_ambiguous();
   test_ambiguous_long();
   test_ambiguous_multiple_long_names();
   test_unknown_option();
   test_multiple_values();
   test_multiple_occurrences();
   test_multiple_occurrences_with_different_names();
   test_multiple_occurrences_with_non_key_names();
   test_missing_value();

   return 0;
}
