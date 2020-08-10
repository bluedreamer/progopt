// Copyright Leo Goodstadt 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/cmdline.h"
#include "argsy/options_description.h"
#include "argsy/parsers.h"
#include "argsy/variables_map.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "minitest.hpp"

//
//  like BOOST_CHECK_EQUAL but with more descriptive error message
//
#define CHECK_EQUAL(description, a, b)                                                                                                     \
   if(a != b)                                                                                                                              \
   {                                                                                                                                       \
      std::cerr << "\n\nError:\n<<" << description << ">>\n  Expected text=\"" << b << "\"\n  Actual text  =\"" << a << "\"\n\n";          \
      assert(a == b);                                                                                                                      \
   }

template<typename EXCEPTION>
void test_each_exception_message(const std::string &test_description, const std::vector<const char *> &argv,
                                 argsy::options_description &desc, int style, std::string exception_msg, std::istream &is = std::cin)
{
   if(exception_msg.length() == 0)
   {
      return;
   }
   argsy::variables_map vm;
   unsigned             argc = argv.size();

   try
   {
      if(style == -1)
      {
         store(parse_config_file(is, desc), vm);
      }
      else
      {
         store(parse_command_line(argv.size(), &argv[0], desc, style), vm);
      }
      notify(vm);
   }
   catch(EXCEPTION &e)
   {
      // cerr << "Correct:\n\t" << e.what() << "\n";
      CHECK_EQUAL(test_description, e.what(), exception_msg);
      return;
   }
   catch(std::exception &e)
   {
      // concatenate argv without boost::algorithm::join
      std::string argv_txt;
      for(unsigned ii = 0; ii < argc - 1; ++ii)
      {
         argv_txt += argv[ii] + std::string(" ");
      }
      if(argc)
      {
         argv_txt += argv[argc - 1];
      }

      BOOST_ERROR("\n<<" + test_description + std::string(">>\n  Unexpected exception type!\n  Actual text  =\"") + e.what() +
                  "\"\n  argv         =\"" + argv_txt + "\"\n  Expected text=\"" + exception_msg + "\"\n");
      return;
   }
   BOOST_ERROR(test_description + ": No exception thrown. ");
}

//
//  test exception messages for all command line styles (unix/long/short/slash/config file)
//
//      try each command line style in turn
const int unix_style = argsy::command_line_style::unix_style;
const int short_dash = argsy::command_line_style::allow_dash_for_short | argsy::command_line_style::allow_short |
                       argsy::command_line_style::short_allow_adjacent | argsy::command_line_style::allow_sticky;
const int short_slash = argsy::command_line_style::allow_slash_for_short | argsy::command_line_style::allow_short |
                        argsy::command_line_style::short_allow_adjacent;
const int long_dash =
   argsy::command_line_style::allow_long | argsy::command_line_style::long_allow_adjacent | argsy::command_line_style::allow_guessing;

template<typename EXCEPTION>
void test_exception_message(const std::vector<std::vector<const char *>> &argv, argsy::options_description &desc,
                            const std::string &error_description, const char *expected_message_template[5])
{
   std::string expected_message;

   // unix
   expected_message = expected_message_template[0];
   test_each_exception_message<EXCEPTION>(error_description + " -- unix", argv[0], desc, unix_style, expected_message);

   // long dash only
   expected_message = expected_message_template[1];
   test_each_exception_message<EXCEPTION>(error_description + " -- long_dash", argv[1], desc, long_dash, expected_message);

   // short dash only
   expected_message = expected_message_template[2];
   test_each_exception_message<EXCEPTION>(error_description + " -- short_dash", argv[2], desc, short_dash, expected_message);

   // short slash only
   expected_message = expected_message_template[3];
   test_each_exception_message<EXCEPTION>(error_description + " -- short_slash", argv[3], desc, short_slash, expected_message);

   // config file only
   expected_message = expected_message_template[4];
   if(expected_message.length())
   {
      std::istringstream istrm(argv[4][0]);
      test_each_exception_message<EXCEPTION>(error_description + " -- config_file", argv[4], desc, -1, expected_message, istrm);
   }
}

#define VEC_STR_PUSH_BACK(vec, c_array) vec.push_back(std::vector<const char *>(c_array, c_array + sizeof(c_array) / sizeof(char *)));

//________________________________________________________________________________________
//
//  invalid_option_value
//
//________________________________________________________________________________________
void test_invalid_option_value_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("int-option,d", argsy::value<int>(), "An option taking an integer");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-d", "A_STRING"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--int", "A_STRING"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-d", "A_STRING"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/d", "A_STRING"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {"int-option=A_STRING"};
   VEC_STR_PUSH_BACK(argv, argv4);

   const char *expected_msg[5] = {
      "the argument ('A_STRING') for option '--int-option' is invalid", "the argument ('A_STRING') for option '--int-option' is invalid",
      "the argument ('A_STRING') for option '-d' is invalid",           "the argument ('A_STRING') for option '/d' is invalid",
      "the argument ('A_STRING') for option 'int-option' is invalid",
   };

   test_exception_message<argsy::invalid_option_value>(argv, desc, "invalid_option_value", expected_msg);
}

//________________________________________________________________________________________
//
//  missing_value
//
//________________________________________________________________________________________
void test_missing_value_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,e", argsy::value<std::string>(), "the config file")("output,o", argsy::value<std::string>(),
                                                                                   "the output file");
   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-e", "-e", "output.txt"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfgfile"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-e", "-e", "output.txt"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/e", "/e", "output.txt"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {""};
   VEC_STR_PUSH_BACK(argv, argv4);

   const char *expected_msg[5] = {
      "the required argument for option '--cfgfile' is missing",
      "the required argument for option '--cfgfile' is missing",
      "the required argument for option '-e' is missing",
      "", // Ignore probable bug in cmdline::finish_option
          //"the required argument for option '/e' is missing",
      "",
   };
   test_exception_message<argsy::invalid_command_line_syntax>(argv, desc, "invalid_syntax::missing_parameter", expected_msg);
}

//________________________________________________________________________________________
//
//  ambiguous_option
//
//________________________________________________________________________________________
void test_ambiguous_option_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile1,c", argsy::value<std::string>(), "the config file")("cfgfile2,o", argsy::value<std::string>(),
                                                                                    "the config file")("good,g", "good option")(
      "output,c", argsy::value<std::string>(), "the output file")("output", argsy::value<std::string>(), "the output file");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-ggc", "file", "-o", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfgfile", "file", "--cfgfile", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-ggc", "file", "-o", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/c", "file", "/o", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {"output=output.txt\n"};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "option '-c' is ambiguous and matches '--cfgfile1', and '--output'",
      "option '--cfgfile' is ambiguous and matches '--cfgfile1', and '--cfgfile2'",
      "option '-c' is ambiguous",
      "option '/c' is ambiguous",
      "option 'output' is ambiguous and matches different versions of 'output'",
   };
   test_exception_message<argsy::ambiguous_option>(argv, desc, "ambiguous_option", expected_msg);
}

//________________________________________________________________________________________
//
//  multiple_occurrences
//
//________________________________________________________________________________________
void test_multiple_occurrences_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>(), "the configfile");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-c", "file", "-c", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfgfi", "file", "--cfgfi", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-c", "file", "-c", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/c", "file", "/c", "anotherfile"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {"cfgfile=output.txt\ncfgfile=output.txt\n"};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "option '--cfgfile' cannot be specified more than once", "option '--cfgfile' cannot be specified more than once",
      "option '-c' cannot be specified more than once",        "option '/c' cannot be specified more than once",
      "option 'cfgfile' cannot be specified more than once",
   };
   test_exception_message<argsy::multiple_occurrences>(argv, desc, "multiple_occurrences", expected_msg);
}

//________________________________________________________________________________________
//
//  unknown_option
//
//________________________________________________________________________________________
void test_unknown_option_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("good,g", "good option");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-ggc", "file"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfgfile", "file"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-ggc", "file"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/c", "file"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {"cfgfile=output.txt\n"};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "unrecognised option '-ggc'", "unrecognised option '--cfgfile'", "unrecognised option '-ggc'",
      "unrecognised option '/c'",   "unrecognised option 'cfgfile'",
   };
   test_exception_message<argsy::unknown_option>(argv, desc, "unknown_option", expected_msg);
}

//________________________________________________________________________________________
//
//  validation_error::invalid_bool_value
//
//________________________________________________________________________________________
void test_invalid_bool_value_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("bool_option,b", argsy::value<bool>(), "bool_option");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-b", "file"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--bool_optio", "file"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-b", "file"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/b", "file"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {"bool_option=output.txt\n"};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "the argument ('file') for option '--bool_option' is invalid. Valid choices are 'on|off', 'yes|no', '1|0' and 'true|false'",
      "the argument ('file') for option '--bool_option' is invalid. Valid choices are 'on|off', 'yes|no', '1|0' and 'true|false'",
      "the argument ('file') for option '-b' is invalid. Valid choices are 'on|off', 'yes|no', '1|0' and 'true|false'",
      "the argument ('file') for option '/b' is invalid. Valid choices are 'on|off', 'yes|no', '1|0' and 'true|false'",
      "the argument ('output.txt') for option 'bool_option' is invalid. Valid choices are 'on|off', 'yes|no', '1|0' and 'true|false'",
   };
   test_exception_message<argsy::validation_error>(argv, desc, "validation_error::invalid_bool_value", expected_msg);
}

//________________________________________________________________________________________
//
//  validation_error::multiple_values_not_allowed
//
//________________________________________________________________________________________
//
//  Strange exception: sole purpose seems to be catching multitoken() associated with a scalar
//  validation_error::multiple_values_not_allowed seems thus to be a programmer error
//
//
void test_multiple_values_not_allowed_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->multitoken(),
                      "the config file")("good,g", "good option")("output,o", argsy::value<std::string>(), "the output file");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-c", "file", "c", "-o", "fritz", "hugo"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfgfil", "file", "c", "--outpu", "fritz", "hugo"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-c", "file", "c", "-o", "fritz", "hugo"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/c", "file", "c", "/o", "fritz", "hugo"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {""};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "option '--cfgfile' only takes a single argument",
      "option '--cfgfile' only takes a single argument",
      "option '-c' only takes a single argument",
      "option '/c' only takes a single argument",
      "",
   };
   test_exception_message<argsy::validation_error>(argv, desc, "validation_error::multiple_values_not_allowed", expected_msg);
}

//________________________________________________________________________________________
//
//  validation_error::at_least_one_value_required
//
//________________________________________________________________________________________
//
//  Strange exception: sole purpose seems to be catching zero_tokens() associated with a scalar
//  validation_error::multiple_values_not_allowed seems thus to be a programmer error
//
//
void test_at_least_one_value_required_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<int>()->zero_tokens(), "the config file")("other,o", argsy::value<std::string>(), "other");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-c"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--cfg", "--o", "name"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-c", "-o", "name"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/c"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {""};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "option '--cfgfile' requires at least one argument",
      "option '--cfgfile' requires at least one argument",
      "option '-c' requires at least one argument",
      "option '/c' requires at least one argument",
      "",
   };
   test_exception_message<argsy::validation_error>(argv, desc, "validation_error::at_least_one_value_required", expected_msg);
}

//________________________________________________________________________________________
//
//  required_option
//
//________________________________________________________________________________________
void test_required_option_exception_msg()
{
   argsy::options_description desc;
   desc.add_options()("cfgfile,c", argsy::value<std::string>()->required(),
                      "the config file")("good,g", "good option")("output,o", argsy::value<std::string>()->required(), "the output file");

   std::vector<std::vector<const char *>> argv;
   const char *                           argv0[] = {"program", "-g"};
   VEC_STR_PUSH_BACK(argv, argv0);
   const char *argv1[] = {"program", "--g"};
   VEC_STR_PUSH_BACK(argv, argv1);
   const char *argv2[] = {"program", "-g"};
   VEC_STR_PUSH_BACK(argv, argv2);
   const char *argv3[] = {"program", "/g"};
   VEC_STR_PUSH_BACK(argv, argv3);
   const char *argv4[] = {""};
   VEC_STR_PUSH_BACK(argv, argv4);
   const char *expected_msg[5] = {
      "the option '--cfgfile' is required but missing", "the option '--cfgfile' is required but missing",
      "the option '-c' is required but missing",        "the option '/c' is required but missing",
      "the option 'cfgfile' is required but missing",
   };
   test_exception_message<argsy::required_option>(argv, desc, "required_option", expected_msg);
}

/**
 * Check if this is the expected exception with the right message is being thrown inside
 * func
 */
template<typename EXCEPTION, typename FUNC>
void test_exception(const std::string &test_name, const std::string &exception_txt, FUNC func)
{
   try
   {
      argsy::options_description desc;
      argsy::variables_map       vm;
      func(desc, vm);
   }
   catch(EXCEPTION &e)
   {
      CHECK_EQUAL(test_name, e.what(), exception_txt);
      return;
   }
   catch(std::exception &e)
   {
      BOOST_ERROR(std::string(test_name + ":\nUnexpected exception. ") + e.what() + "\nExpected text:\n" + exception_txt + "\n\n");
      return;
   }
   BOOST_ERROR(test_name + ": No exception thrown. ");
}

//________________________________________________________________________________________
//
//  check_reading_file
//
//________________________________________________________________________________________
void check_reading_file(argsy::options_description &desc, argsy::variables_map &vm)
{
   desc.add_options()("output,o", argsy::value<std::string>(), "the output file");

   const char *file_name = "no_such_file";
   store(parse_config_file<char>(file_name, desc, true), vm);
}

//________________________________________________________________________________________
//
//  config_file_wildcard
//
//________________________________________________________________________________________
void config_file_wildcard(argsy::options_description &desc, argsy::variables_map &vm)
{
   desc.add_options()("outpu*", argsy::value<std::string>(), "the output file1")("outp*", argsy::value<std::string>(), "the output file2");
   std::istringstream is("output1=whichone\noutput2=whichone\n");
   store(parse_config_file(is, desc), vm);
}

//________________________________________________________________________________________
//
//  invalid_syntax::unrecognized_line
//
//________________________________________________________________________________________
void unrecognized_line(argsy::options_description &desc, argsy::variables_map &vm)
{
   std::istringstream is("funny wierd line\n");
   store(parse_config_file(is, desc), vm);
}

//________________________________________________________________________________________
//
//  abbreviated_options_in_config_file
//
//________________________________________________________________________________________
void abbreviated_options_in_config_file(argsy::options_description &desc, argsy::variables_map &vm)
{
   desc.add_options()(",o", argsy::value<std::string>(), "the output file");
   std::istringstream is("o=output.txt\n");
   store(parse_config_file(is, desc), vm);
}

//________________________________________________________________________________________
//
//  too_many_positional_options
//
//________________________________________________________________________________________
void too_many_positional_options(argsy::options_description &desc, argsy::variables_map &vm)
{
   const char *                          argv[] = {"program", "1", "2", "3"};
   argsy::positional_options_description positional_args;
   positional_args.add("two_positional_arguments", 2);
   store(argsy::command_line_parser(4, argv).options(desc).positional(positional_args).run(), vm);
}

//________________________________________________________________________________________
//
//  invalid_command_line_style
//
//________________________________________________________________________________________

void test_invalid_command_line_style_exception_msg()
{
   std::string                test_name = "invalid_command_line_style";
   argsy::options_description desc;
   desc.add_options()("output,o", argsy::value<std::string>(), "the output file");

   std::vector<int> invalid_styles;
   invalid_styles.push_back(argsy::command_line_style::allow_short | argsy::command_line_style::short_allow_adjacent);
   invalid_styles.push_back(argsy::command_line_style::allow_short | argsy::command_line_style::allow_dash_for_short);
   invalid_styles.push_back(argsy::command_line_style::allow_long);
   std::vector<std::string> invalid_diagnostics;
   invalid_diagnostics.emplace_back("argsy misconfiguration: choose one "
                                    "or other of 'command_line_style::allow_slash_for_short' "
                                    "(slashes) or 'command_line_style::allow_dash_for_short' "
                                    "(dashes) for short options.");
   invalid_diagnostics.emplace_back("argsy misconfiguration: choose one "
                                    "or other of 'command_line_style::short_allow_next' "
                                    "(whitespace separated arguments) or "
                                    "'command_line_style::short_allow_adjacent' ('=' "
                                    "separated arguments) for short options.");
   invalid_diagnostics.emplace_back("argsy misconfiguration: choose one "
                                    "or other of 'command_line_style::long_allow_next' "
                                    "(whitespace separated arguments) or "
                                    "'command_line_style::long_allow_adjacent' ('=' "
                                    "separated arguments) for long options.");

   const char *         argv[] = {"program"};
   argsy::variables_map vm;
   for(unsigned ii = 0; ii < 3; ++ii)
   {
      bool exception_thrown = false;
      try
      {
         store(parse_command_line(1, argv, desc, invalid_styles[ii]), vm);
      }
      catch(argsy::invalid_command_line_style &e)
      {
         std::string error_msg("arguments are not allowed for unabbreviated option names");
         CHECK_EQUAL(test_name, e.what(), invalid_diagnostics[ii]);
         exception_thrown = true;
      }
      catch(std::exception &e)
      {
         BOOST_ERROR(std::string(test_name + ":\nUnexpected exception. ") + e.what() + "\nExpected text:\n" + invalid_diagnostics[ii] +
                     "\n");
         exception_thrown = true;
      }
      if(!exception_thrown)
      {
         BOOST_ERROR(test_name << ": No exception thrown. ");
      }
   }
}

void test_empty_value_inner(argsy::options_description &opts, argsy::variables_map &vm)
{
   argsy::positional_options_description popts;
   opts.add_options()("foo", argsy::value<uint32_t>()->value_name("<time>")->required());
   popts.add("foo", 1);
   std::vector<std::string> tokens(1, "");
   argsy::parsed_options    parsed = argsy::command_line_parser(tokens)
                                     .style(argsy::command_line_style::default_style & ~argsy::command_line_style::allow_guessing)
                                     .options(opts)
                                     .positional(popts)
                                     .run();
   store(parsed, vm);
}

void test_empty_value()
{
   // Test that passing empty token for an option that requires integer does not result
   // in out-of-range error in error reporting code.
   test_exception<argsy::invalid_option_value>("test_empty_value", "the argument for option '--foo' is invalid", test_empty_value_inner);
}

auto main(int /*ac*/, char * * /*av*/) -> int
{
   test_ambiguous_option_exception_msg();
   test_unknown_option_exception_msg();
   test_multiple_occurrences_exception_msg();
   test_missing_value_exception_msg();
   test_invalid_option_value_exception_msg();
   test_invalid_bool_value_exception_msg();
   test_multiple_values_not_allowed_exception_msg();
   test_required_option_exception_msg();
   test_at_least_one_value_required_exception_msg();
   test_empty_value();

   std::string test_name;
   std::string expected_message;

   // check_reading_file
   test_name        = "check_reading_file";
   expected_message = "can not read options configuration file 'no_such_file'";
   test_exception<argsy::reading_file>(test_name, expected_message, check_reading_file);

   // config_file_wildcard
   test_name        = "config_file_wildcard";
   expected_message = "options 'outpu*' and 'outp*' will both match the same arguments from the configuration file";
   test_exception<argsy::error>(test_name, expected_message, config_file_wildcard);

   // unrecognized_line
   test_name        = "unrecognized_line";
   expected_message = "the options configuration file contains an invalid line 'funny wierd line'";
   test_exception<argsy::invalid_syntax>(test_name, expected_message, unrecognized_line);

   // abbreviated_options_in_config_file
   test_name        = "abbreviated_options_in_config_file";
   expected_message = "abbreviated option names are not permitted in options configuration files";
   test_exception<argsy::error>(test_name, expected_message, abbreviated_options_in_config_file);

   test_name        = "too_many_positional_options";
   expected_message = "too many positional options have been specified on the command line";
   test_exception<argsy::too_many_positional_options_error>(test_name, expected_message, too_many_positional_options);

   test_invalid_command_line_style_exception_msg();

   return 0;
}
