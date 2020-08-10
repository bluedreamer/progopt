// Copyright Sascha Ochsenknecht 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "argsy/cmdline.hpp"
#include "argsy/detail/cmdline.hpp"
#include "argsy/options_description.hpp"
#include "argsy/parsers.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

#include "minitest.hpp"

// Test free function collect_unrecognized()
//
//  it collects the tokens of all not registered options. It can be used
//  to pass them to an own parser implementation

void test_unrecognize_cmdline()
{
   argsy::options_description desc;

   std::string              content = "prg --input input.txt --optimization 4 --opt option";
   std::vector<std::string> tokens  = argsy::split_unix(content);

   argsy::detail::cmdline cmd(tokens);
   cmd.set_options_description(desc);
   cmd.allow_unregistered();

   std::vector<argsy::option> opts   = cmd.run();
   std::vector<std::string>   result = collect_unrecognized(opts, argsy::include_positional);

   BOOST_CHECK_EQUAL(result.size(), 7);
   BOOST_CHECK_EQUAL(result[0], "prg");
   BOOST_CHECK_EQUAL(result[1], "--input");
   BOOST_CHECK_EQUAL(result[2], "input.txt");
   BOOST_CHECK_EQUAL(result[3], "--optimization");
   BOOST_CHECK_EQUAL(result[4], "4");
   BOOST_CHECK_EQUAL(result[5], "--opt");
   BOOST_CHECK_EQUAL(result[6], "option");
}

void test_unrecognize_config()
{
   argsy::options_description desc;

   std::string content = " input = input.txt\n"
                         " optimization = 4\n"
                         " opt = option\n";

   std::stringstream          ss(content);
   std::vector<argsy::option> opts   = parse_config_file(ss, desc, true).options;
   std::vector<std::string>   result = collect_unrecognized(opts, argsy::include_positional);

   BOOST_CHECK_EQUAL(result.size(), 6);
   BOOST_CHECK_EQUAL(result[0], "input");
   BOOST_CHECK_EQUAL(result[1], "input.txt");
   BOOST_CHECK_EQUAL(result[2], "optimization");
   BOOST_CHECK_EQUAL(result[3], "4");
   BOOST_CHECK_EQUAL(result[4], "opt");
   BOOST_CHECK_EQUAL(result[5], "option");
}

auto main(int /*ac*/, char * * /*av*/) -> int
{
   test_unrecognize_cmdline();
   test_unrecognize_config();

   return 0;
}
