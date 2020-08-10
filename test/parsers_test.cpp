// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"
#include "argsy/variables_map.hpp"

#include <cstdlib> // for putenv
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "minitest.hpp"

#define TEST_CHECK_THROW(expression, exception, description)                                                                               \
   try                                                                                                                                     \
   {                                                                                                                                       \
      expression;                                                                                                                          \
      BOOST_ERROR(description);                                                                                                            \
      throw 10;                                                                                                                            \
   }                                                                                                                                       \
   catch(exception &)                                                                                                                      \
   {                                                                                                                                       \
   }

auto msp(const std::string &s1) -> std::pair<std::string, std::vector<std::vector<std::string>>>
{
   return std::make_pair(s1, std::vector<std::vector<std::string>>());
}

auto msp(const std::string &s1, const std::string &s2) -> std::pair<std::string, std::vector<std::vector<std::string>>>
{
   std::vector<std::vector<std::string>> v(1);
   v[0].push_back(s2);
   return std::make_pair(s1, v);
}

void check_value(const argsy::option &option, const char *name, const char *value)
{
   BOOST_CHECK(option.string_key == name);
   BOOST_REQUIRE(option.value.size() == 1);
   BOOST_CHECK(option.value.front() == value);
}

auto sv(const char *array[], unsigned size) -> std::vector<std::string>
{
   std::vector<std::string> r;
   for(unsigned i = 0; i < size; ++i)
   {
      r.emplace_back(array[i]);
   }
   return r;
}

auto additional_parser(const std::string &) -> std::pair<std::string, std::string>
{
   return std::pair<std::string, std::string>();
}

namespace command_line
{
#if 0
// The following commented out blocks used to test parsing
// command line without syntax specification behaviour.
// It is disabled now and probably will never be enabled again:
// it is not possible to figure out what command line means without
// user's help.
void test_parsing_without_specifying_options() {
    char* cmdline1[] = { "--a", "--b=12", "-f", "-g4", "-", "file" };
    options_and_arguments a1 = parse_command_line(cmdline1,
            cmdline1 + sizeof(cmdline1) / sizeof(cmdline1[0]));
    BOOST_REQUIRE(a1.options().size() == 4);
    BOOST_CHECK(a1.options()[0] == msp("a", ""));
    BOOST_CHECK(a1.options()[1] == msp("b", "12"));
    BOOST_CHECK(a1.options()[2] == msp("-f", ""));
    BOOST_CHECK(a1.options()[3] == msp("-g", "4"));
    BOOST_REQUIRE(a1.arguments().size() == 2);
    BOOST_CHECK(a1.arguments()[0] == "-");
    BOOST_CHECK(a1.arguments()[1] == "file");
    char* cmdline2[] = { "--a", "--", "file" };
    options_and_arguments a2 = parse_command_line(cmdline2,
            cmdline2 + sizeof(cmdline2) / sizeof(cmdline2[0]));
    BOOST_REQUIRE(a2.options().size() == 1);
    BOOST_CHECK(a2.options()[0] == msp("a", ""));
    BOOST_CHECK(a2.arguments().size() == 1);
    BOOST_CHECK(a2.arguments()[0] == "file");
}
#endif

void test_many_different_options()
{
   argsy::options_description desc;
   desc.add_options()("foo,f", new argsy::untyped_value(), "")( // Explicit qualification is a workaround for vc6
      "bar,b", argsy::value<std::string>(), "")("car,voiture", new argsy::untyped_value())("dog,dawg", new argsy::untyped_value())(
      "baz", new argsy::untyped_value())("plug*", new argsy::untyped_value());
   const char *               cmdline3_[] = {"--foo=12", "-f4", "--bar=11", "-b4", "--voiture=15", "--dawg=16", "--dog=17", "--plug3=10"};
   std::vector<std::string>   cmdline3    = sv(cmdline3_, sizeof(cmdline3_) / sizeof(const char *));
   std::vector<argsy::option> a3          = argsy::command_line_parser(cmdline3).options(desc).run().options;
   BOOST_CHECK_EQUAL(a3.size(), 8u);
   check_value(a3[0], "foo", "12");
   check_value(a3[1], "foo", "4");
   check_value(a3[2], "bar", "11");
   check_value(a3[3], "bar", "4");
   check_value(a3[4], "car", "15");
   check_value(a3[5], "dog", "16");
   check_value(a3[6], "dog", "17");
   check_value(a3[7], "plug3", "10");

   // Regression test: check that '0' as style is interpreted as
   // 'default_style'
   std::vector<argsy::option> a4 =
      parse_command_line(sizeof(cmdline3_) / sizeof(const char *), cmdline3_, desc, 0, additional_parser).options;
   // The default style is unix-style, where the first argument on the command-line
   // is the name of a binary, not an option value, so that should be ignored
   BOOST_CHECK_EQUAL(a4.size(), 7u);
   check_value(a4[0], "foo", "4");
   check_value(a4[1], "bar", "11");
   check_value(a4[2], "bar", "4");
   check_value(a4[3], "car", "15");
   check_value(a4[4], "dog", "16");
   check_value(a4[5], "dog", "17");
   check_value(a4[6], "plug3", "10");
}

void test_not_crashing_with_empty_string_values()
{
   // Check that we don't crash on empty values of type 'string'
   const char *               cmdline4[] = {"", "--open", ""};
   argsy::options_description desc2;
   desc2.add_options()("open", argsy::value<std::string>());
   argsy::variables_map vm;
   argsy::store(argsy::parse_command_line(sizeof(cmdline4) / sizeof(const char *), const_cast<char **>(cmdline4), desc2), vm);
}

void test_multitoken()
{
   const char *               cmdline5[] = {"", "-p7", "-o", "1", "2", "3", "-x8"};
   argsy::options_description desc3;
   desc3.add_options()(",p", argsy::value<std::string>())(",o", argsy::value<std::string>()->multitoken())(",x",
                                                                                                           argsy::value<std::string>());
   std::vector<argsy::option> a5 =
      parse_command_line(sizeof(cmdline5) / sizeof(const char *), const_cast<char **>(cmdline5), desc3, 0, additional_parser).options;
   BOOST_CHECK_EQUAL(a5.size(), 3u);
   check_value(a5[0], "-p", "7");
   BOOST_REQUIRE(a5[1].value.size() == 3);
   BOOST_CHECK_EQUAL(a5[1].string_key, "-o");
   BOOST_CHECK_EQUAL(a5[1].value[0], "1");
   BOOST_CHECK_EQUAL(a5[1].value[1], "2");
   BOOST_CHECK_EQUAL(a5[1].value[2], "3");
   check_value(a5[2], "-x", "8");
}

void test_multitoken_and_multiname()
{
   const char *               cmdline[] = {"program", "-fone", "-b", "two", "--foo", "three", "four", "-zfive", "--fee", "six"};
   argsy::options_description desc;
   desc.add_options()("bar,b", argsy::value<std::string>())("foo,fee,f", argsy::value<std::string>()->multitoken())(
      "fizbaz,baz,z", argsy::value<std::string>());

   std::vector<argsy::option> parsed_options =
      parse_command_line(sizeof(cmdline) / sizeof(const char *), const_cast<char **>(cmdline), desc, 0, additional_parser).options;

   BOOST_CHECK_EQUAL(parsed_options.size(), 5u);
   check_value(parsed_options[0], "foo", "one");
   check_value(parsed_options[1], "bar", "two");
   BOOST_CHECK_EQUAL(parsed_options[2].string_key, "foo");
   BOOST_REQUIRE(parsed_options[2].value.size() == 2);
   BOOST_CHECK_EQUAL(parsed_options[2].value[0], "three");
   BOOST_CHECK_EQUAL(parsed_options[2].value[1], "four");
   check_value(parsed_options[3], "fizbaz", "five");
   check_value(parsed_options[4], "foo", "six");

   const char *cmdline_2[] = {"program", "-fone", "-b", "two", "--fee", "three", "four", "-zfive", "--foo", "six"};

   parsed_options =
      parse_command_line(sizeof(cmdline_2) / sizeof(const char *), const_cast<char **>(cmdline_2), desc, 0, additional_parser).options;

   BOOST_CHECK_EQUAL(parsed_options.size(), 5u);
   check_value(parsed_options[0], "foo", "one");
   check_value(parsed_options[1], "bar", "two");
   BOOST_CHECK_EQUAL(parsed_options[2].string_key, "foo");
   BOOST_REQUIRE(parsed_options[2].value.size() == 2);
   BOOST_CHECK_EQUAL(parsed_options[2].value[0], "three");
   BOOST_CHECK_EQUAL(parsed_options[2].value[1], "four");
   check_value(parsed_options[3], "fizbaz", "five");
   check_value(parsed_options[4], "foo", "six");
}

void test_multitoken_vector_option()
{
   argsy::options_description desc4("");
   desc4.add_options()("multitoken,multi-token,m", argsy::value<std::vector<std::string>>()->multitoken(),
                       "values")("file", argsy::value<std::string>(), "the file to process");
   argsy::positional_options_description p;
   p.add("file", 1);
   const char *               cmdline6[] = {"", "-m", "token1", "token2", "--", "some_file"};
   std::vector<argsy::option> a6 = argsy::command_line_parser(sizeof(cmdline6) / sizeof(const char *), const_cast<char **>(cmdline6))
                                      .options(desc4)
                                      .positional(p)
                                      .run()
                                      .options;
   BOOST_CHECK_EQUAL(a6.size(), 2u);
   BOOST_REQUIRE(a6[0].value.size() == 2);
   BOOST_CHECK_EQUAL(a6[0].string_key, "multitoken");
   BOOST_CHECK_EQUAL(a6[0].value[0], "token1");
   BOOST_CHECK_EQUAL(a6[0].value[1], "token2");
   BOOST_CHECK_EQUAL(a6[1].string_key, "file");
   BOOST_REQUIRE(a6[1].value.size() == 1);
   BOOST_CHECK_EQUAL(a6[1].value[0], "some_file");
}

} // namespace command_line

void test_command_line()
{
#if 0
    command_line::test_parsing_without_specifying_options();
#endif
   command_line::test_many_different_options();
   // Check that we don't crash on empty values of type 'string'
   command_line::test_not_crashing_with_empty_string_values();
   command_line::test_multitoken();
   command_line::test_multitoken_vector_option();
   command_line::test_multitoken_and_multiname();
}

void test_config_file(const char *config_file)
{
   argsy::options_description desc;
   desc.add_options()("gv1", new argsy::untyped_value)("gv2", new argsy::untyped_value)("empty_value", new argsy::untyped_value)(
      "plug*", new argsy::untyped_value)("m1.v1", new argsy::untyped_value)("m1.v2", new argsy::untyped_value)(
      "m1.v3,alias3", new argsy::untyped_value)("b", argsy::bool_switch());

   const char content1[] = " gv1 = 0#asd\n"
                           "empty_value = \n"
                           "plug3 = 7\n"
                           "b = true\n"
                           "[m1]\n"
                           "v1 = 1\n"
                           "\n"
                           "v2 = 2\n"
                           "v3 = 3\n";

   std::stringstream          ss(content1);
   std::vector<argsy::option> a1 = parse_config_file(ss, desc).options;
   BOOST_REQUIRE(a1.size() == 7);
   check_value(a1[0], "gv1", "0");
   check_value(a1[1], "empty_value", "");
   check_value(a1[2], "plug3", "7");
   check_value(a1[3], "b", "true");
   check_value(a1[4], "m1.v1", "1");
   check_value(a1[5], "m1.v2", "2");
   check_value(a1[6], "m1.v3", "3");

   // same test, but now options come from file
   std::vector<argsy::option> a2 = parse_config_file<char>(config_file, desc).options;
   BOOST_REQUIRE(a2.size() == 7);
   check_value(a2[0], "gv1", "0");
   check_value(a2[1], "empty_value", "");
   check_value(a2[2], "plug3", "7");
   check_value(a2[3], "b", "true");
   check_value(a2[4], "m1.v1", "1");
   check_value(a2[5], "m1.v2", "2");
   check_value(a2[6], "m1.v3", "3");
}

void test_environment()
{
   argsy::options_description desc;
   desc.add_options()("foo", new argsy::untyped_value, "")("bar", new argsy::untyped_value, "");

#if(defined(_WIN32) && !defined(__BORLANDC__)) || (defined(__CYGWIN__))
   _putenv("PO_TEST_FOO=1");
#else
   putenv(const_cast<char *>("PO_TEST_FOO=1"));
#endif
   argsy::parsed_options p = parse_environment(desc, "PO_TEST_");

   BOOST_REQUIRE(p.options.size() == 1);
   BOOST_CHECK(p.options[0].string_key == "foo");
   BOOST_REQUIRE(p.options[0].value.size() == 1);
   BOOST_CHECK(p.options[0].value[0] == "1");

   // TODO: since 'bar' does not allow a value, it cannot appear in environemt,
   // which already has a value.
}

void test_unregistered()
{
   argsy::options_description desc;

   const char *               cmdline1_[] = {"--foo=12", "--bar", "1"};
   std::vector<std::string>   cmdline1    = sv(cmdline1_, sizeof(cmdline1_) / sizeof(const char *));
   std::vector<argsy::option> a1          = argsy::command_line_parser(cmdline1).options(desc).allow_unregistered().run().options;

   BOOST_REQUIRE(a1.size() == 3);
   BOOST_CHECK(a1[0].string_key == "foo");
   BOOST_CHECK(a1[0].unregistered == true);
   BOOST_REQUIRE(a1[0].value.size() == 1);
   BOOST_CHECK(a1[0].value[0] == "12");
   BOOST_CHECK(a1[1].string_key == "bar");
   BOOST_CHECK(a1[1].unregistered == true);
   BOOST_CHECK(a1[2].string_key == "");
   BOOST_CHECK(a1[2].unregistered == false);

   std::vector<std::string> a2 = collect_unrecognized(a1, argsy::include_positional);
   BOOST_CHECK(a2[0] == "--foo=12");
   BOOST_CHECK(a2[1] == "--bar");
   BOOST_CHECK(a2[2] == "1");

   // Test that storing unregisted options has no effect
   argsy::variables_map vm;

   store(argsy::command_line_parser(cmdline1).options(desc).allow_unregistered().run(), vm);

   BOOST_CHECK_EQUAL(vm.size(), 0u);

   const char content1[] = "gv1 = 0\n"
                           "[m1]\n"
                           "v1 = 1\n";

   std::stringstream          ss(content1);
   std::vector<argsy::option> a3 = parse_config_file(ss, desc, true).options;
   BOOST_REQUIRE(a3.size() == 2);
   std::cout << "XXX" << a3[0].value.front() << "\n";
   check_value(a3[0], "gv1", "0");
   check_value(a3[1], "m1.v1", "1");
}

auto main(int, char *av[]) -> int
{
   test_command_line();
   test_config_file(av[1]);
   test_environment();
   test_unregistered();
   return 0;
}
