// Copyright Vladimir Prus 2002-2004.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/detail/utf8_codecvt_facet.hpp"
#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"
#include "argsy/variables_map.hpp"

#include <functional>
#include <sstream>

#include "minitest.hpp"

auto sv(const char *array[], unsigned size) -> std::vector<std::string>
{
   std::vector<std::string> r;
   for(unsigned i = 0; i < size; ++i)
   {
      r.emplace_back(array[i]);
   }
   return r;
}

void test_variable_map()
{
   argsy::options_description desc;
   desc.add_options()("foo,f", new argsy::untyped_value)("bar,b", argsy::value<std::string>())("biz,z", argsy::value<std::string>())(
      "baz", new argsy::untyped_value())("output,o", new argsy::untyped_value(), "");
   const char *             cmdline3_[] = {"--foo='12'", "--bar=11", "-z3", "-ofoo"};
   std::vector<std::string> cmdline3    = sv(cmdline3_, sizeof(cmdline3_) / sizeof(const char *));
   argsy::parsed_options    a3          = argsy::command_line_parser(cmdline3).options(desc).run();
   argsy::variables_map     vm;
   store(a3, vm);
   notify(vm);
   BOOST_REQUIRE(vm.size() == 4);
   BOOST_CHECK(vm["foo"].as<std::string>() == "'12'");
   BOOST_CHECK(vm["bar"].as<std::string>() == "11");
   BOOST_CHECK(vm.count("biz") == 1);
   BOOST_CHECK(vm["biz"].as<std::string>() == "3");
   BOOST_CHECK(vm["output"].as<std::string>() == "foo");

   int i;
   desc.add_options()("zee", argsy::bool_switch(), "")("zak", argsy::value<int>(&i), "")("opt", argsy::bool_switch(), "");

   const char *             cmdline4_[] = {"--zee", "--zak=13"};
   std::vector<std::string> cmdline4    = sv(cmdline4_, sizeof(cmdline4_) / sizeof(const char *));
   argsy::parsed_options    a4          = argsy::command_line_parser(cmdline4).options(desc).run();

   argsy::variables_map vm2;
   store(a4, vm2);
   notify(vm2);
   BOOST_REQUIRE(vm2.size() == 3);
   BOOST_CHECK(vm2["zee"].as<bool>() == true);
   BOOST_CHECK(vm2["zak"].as<int>() == 13);
   BOOST_CHECK(vm2["opt"].as<bool>() == false);
   BOOST_CHECK(i == 13);

   argsy::options_description desc2;
   desc2.add_options()("vee", argsy::value<std::string>()->default_value("42"))("voo", argsy::value<std::string>())(
      "iii", argsy::value<int>()->default_value(123));
   const char *             cmdline5_[] = {"--voo=1"};
   std::vector<std::string> cmdline5    = sv(cmdline5_, sizeof(cmdline5_) / sizeof(const char *));
   argsy::parsed_options    a5          = argsy::command_line_parser(cmdline5).options(desc2).run();

   argsy::variables_map vm3;
   store(a5, vm3);
   notify(vm3);
   BOOST_REQUIRE(vm3.size() == 3);
   BOOST_CHECK(vm3["vee"].as<std::string>() == "42");
   BOOST_CHECK(vm3["voo"].as<std::string>() == "1");
   BOOST_CHECK(vm3["iii"].as<int>() == 123);

   argsy::options_description desc3;
   desc3.add_options()("imp", argsy::value<int>()->implicit_value(100))("iim",
                                                                        argsy::value<int>()->implicit_value(200)->default_value(201))(
      "mmp,m", argsy::value<int>()->implicit_value(123)->default_value(124))("foo", argsy::value<int>());
   /* The -m option is implicit. It does not have value in inside the token,
      and we should not grab the next token.  */
   const char *             cmdline6_[] = {"--imp=1", "-m", "--foo=1"};
   std::vector<std::string> cmdline6    = sv(cmdline6_, sizeof(cmdline6_) / sizeof(const char *));
   argsy::parsed_options    a6          = argsy::command_line_parser(cmdline6).options(desc3).run();

   argsy::variables_map vm4;
   store(a6, vm4);
   notify(vm4);
   BOOST_REQUIRE(vm4.size() == 4);
   BOOST_CHECK(vm4["imp"].as<int>() == 1);
   BOOST_CHECK(vm4["iim"].as<int>() == 201);
   BOOST_CHECK(vm4["mmp"].as<int>() == 123);
}

int  stored_value;
void notifier(const std::vector<int> &v)
{
   stored_value = v.front();
}

void test_semantic_values()
{
   argsy::options_description desc;
   desc.add_options()("foo", new argsy::untyped_value())("bar", argsy::value<int>())("biz", argsy::value<std::vector<std::string>>())(
      "baz", argsy::value<std::vector<std::string>>()->multitoken())("int", argsy::value<std::vector<int>>()->notifier(&notifier));

   argsy::parsed_options       parsed(&desc);
   std::vector<argsy::option> &options = parsed.options;
   std::vector<std::string>    v;
   v.emplace_back("q");
   options.emplace_back("foo", std::vector<std::string>(1, "1"));
   options.emplace_back("biz", std::vector<std::string>(1, "a"));
   options.emplace_back("baz", v);
   options.emplace_back("bar", std::vector<std::string>(1, "1"));
   options.emplace_back("biz", std::vector<std::string>(1, "b x"));
   v.emplace_back("w");
   options.emplace_back("baz", v);

   argsy::variables_map vm;
   store(parsed, vm);
   notify(vm);
   BOOST_REQUIRE(vm.count("biz") == 1);
   BOOST_REQUIRE(vm.count("baz") == 1);
   const std::vector<std::string> av     = vm["biz"].as<std::vector<std::string>>();
   const std::vector<std::string> av2    = vm["baz"].as<std::vector<std::string>>();
   std::string                    exp1[] = {"a", "b x"};
   BOOST_CHECK(av == std::vector<std::string>(exp1, exp1 + 2));
   std::string exp2[] = {"q", "q", "w"};
   BOOST_CHECK(av2 == std::vector<std::string>(exp2, exp2 + 3));

   options.emplace_back("int", std::vector<std::string>(1, "13"));

   argsy::variables_map vm2;
   store(parsed, vm2);
   notify(vm2);
   BOOST_REQUIRE(vm2.count("int") == 1);
   BOOST_CHECK(vm2["int"].as<std::vector<int>>() == std::vector<int>(1, 13));
   BOOST_CHECK_EQUAL(stored_value, 13);

   std::vector<argsy::option> saved_options = options;

   options.emplace_back("bar", std::vector<std::string>(1, "2"));
   argsy::variables_map vm3;
   BOOST_CHECK_THROW(store(parsed, vm3), argsy::multiple_occurrences);

   options = saved_options;
   // Now try passing two int in one 'argv' element.
   // This should not work.
   options.emplace_back("int", std::vector<std::string>(1, "2 3"));
   argsy::variables_map vm4;
   BOOST_CHECK_THROW(store(parsed, vm4), argsy::validation_error);
}

void test_priority()
{
   argsy::options_description desc;
   desc.add_options()
      // Value of this option will be specified in two sources,
      // and only first one should be used.
      ("first", argsy::value<std::vector<int>>())
      // Value of this option will have default value in the first source,
      // and explicit assignment in the second, so the second should be used.
      ("second", argsy::value<std::vector<int>>()->default_value(std::vector<int>(1, 1), ""))("aux", argsy::value<std::vector<int>>())
      // This will have values in both sources, and values should be combined
      ("include", argsy::value<std::vector<int>>()->composing());

   const char *             cmdline1_[] = {"--first=1", "--aux=10", "--first=3", "--include=1"};
   std::vector<std::string> cmdline1    = sv(cmdline1_, sizeof(cmdline1_) / sizeof(const char *));

   argsy::parsed_options p1 = argsy::command_line_parser(cmdline1).options(desc).run();

   const char *             cmdline2_[] = {"--first=12", "--second=7", "--include=7"};
   std::vector<std::string> cmdline2    = sv(cmdline2_, sizeof(cmdline2_) / sizeof(const char *));

   argsy::parsed_options p2 = argsy::command_line_parser(cmdline2).options(desc).run();

   argsy::variables_map vm;
   store(p1, vm);

   BOOST_REQUIRE(vm.count("first") == 1);
   BOOST_REQUIRE(vm["first"].as<std::vector<int>>().size() == 2);
   BOOST_CHECK_EQUAL(vm["first"].as<std::vector<int>>()[0], 1);
   BOOST_CHECK_EQUAL(vm["first"].as<std::vector<int>>()[1], 3);

   BOOST_REQUIRE(vm.count("second") == 1);
   BOOST_REQUIRE(vm["second"].as<std::vector<int>>().size() == 1);
   BOOST_CHECK_EQUAL(vm["second"].as<std::vector<int>>()[0], 1);

   store(p2, vm);

   // Value should not change.
   BOOST_REQUIRE(vm.count("first") == 1);
   BOOST_REQUIRE(vm["first"].as<std::vector<int>>().size() == 2);
   BOOST_CHECK_EQUAL(vm["first"].as<std::vector<int>>()[0], 1);
   BOOST_CHECK_EQUAL(vm["first"].as<std::vector<int>>()[1], 3);

   // Value should change to 7
   BOOST_REQUIRE(vm.count("second") == 1);
   BOOST_REQUIRE(vm["second"].as<std::vector<int>>().size() == 1);
   BOOST_CHECK_EQUAL(vm["second"].as<std::vector<int>>()[0], 7);

   BOOST_REQUIRE(vm.count("include") == 1);
   BOOST_REQUIRE(vm["include"].as<std::vector<int>>().size() == 2);
   BOOST_CHECK_EQUAL(vm["include"].as<std::vector<int>>()[0], 1);
   BOOST_CHECK_EQUAL(vm["include"].as<std::vector<int>>()[1], 7);
}

void test_multiple_assignments_with_different_option_description()
{
   // Test that if we store option twice into the same variable_map,
   // and some of the options stored the first time are not present
   // in the options descrription provided the second time, we don't crash.

   argsy::options_description desc1("");
   desc1.add_options()("help,h", "")("includes", argsy::value<std::vector<std::string>>()->composing(), "");
   ;

   argsy::options_description desc2("");
   desc2.add_options()("output,o", "");

   std::vector<std::string> input1;
   input1.emplace_back("--help");
   input1.emplace_back("--includes=a");
   argsy::parsed_options p1 = argsy::command_line_parser(input1).options(desc1).run();

   std::vector<std::string> input2;
   input1.emplace_back("--output");
   argsy::parsed_options p2 = argsy::command_line_parser(input2).options(desc2).run();

   std::vector<std::string> input3;
   input3.emplace_back("--includes=b");
   argsy::parsed_options p3 = argsy::command_line_parser(input3).options(desc1).run();

   argsy::variables_map vm;
   store(p1, vm);
   store(p2, vm);
   store(p3, vm);

   BOOST_REQUIRE(vm.count("help") == 1);
   BOOST_REQUIRE(vm.count("includes") == 1);
   BOOST_CHECK_EQUAL(vm["includes"].as<std::vector<std::string>>()[0], "a");
   BOOST_CHECK_EQUAL(vm["includes"].as<std::vector<std::string>>()[1], "b");
}

auto main(int, char *[]) -> int
{
   test_variable_map();
   test_semantic_values();
   test_priority();
   test_multiple_assignments_with_different_option_description();
   return 0;
}
