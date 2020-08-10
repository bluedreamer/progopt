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

void check_value(const std::string &option, const std::string &value)
{
   BOOST_CHECK(option == value);
}

void split_whitespace(const argsy::options_description &description)
{
   const char *cmdline = "prg --input input.txt \r --optimization 4  \t  --opt \n  option";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, " \t\n\r");

   BOOST_REQUIRE(tokens.size() == 7);

   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "option");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_equalsign(const argsy::options_description &description)
{
   const char *cmdline = "prg --input=input.txt  --optimization=4 --opt=option";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, "= ");

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "option");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_semi(const argsy::options_description &description)
{
   const char *cmdline = "prg;--input input.txt;--optimization 4;--opt option";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, "; ");

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "option");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_quotes(const argsy::options_description &description)
{
   const char *cmdline = R"(prg --input "input.txt input.txt" --optimization 4 --opt "option1 option2")";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, " ");

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input.txt input.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "option1 option2");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_escape(const argsy::options_description &description)
{
   const char *cmdline = R"(prg --input \"input.txt\" --optimization 4 --opt \"option1\ option2\")";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, " ");

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "\"input.txt\"");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "\"option1 option2\"");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_single_quote(const argsy::options_description &description)
{
   const char *cmdline = "prg --input 'input.txt input.txt' --optimization 4 --opt 'option1 option2'";

   std::vector<std::string> tokens = argsy::split_unix(cmdline, " ", "'");

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input.txt input.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "option1 option2");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

void split_defaults(const argsy::options_description &description)
{
   const char *cmdline = "prg --input \t \'input file.txt\' \t   --optimization 4 --opt \\\"option1\\ option2\\\"";

   std::vector<std::string> tokens = argsy::split_unix(cmdline);

   BOOST_REQUIRE(tokens.size() == 7);
   check_value(tokens[0], "prg");
   check_value(tokens[1], "--input");
   check_value(tokens[2], "input file.txt");
   check_value(tokens[3], "--optimization");
   check_value(tokens[4], "4");
   check_value(tokens[5], "--opt");
   check_value(tokens[6], "\"option1 option2\"");

   argsy::variables_map vm;
   store(argsy::command_line_parser(tokens).options(description).run(), vm);
   notify(vm);
}

auto main(int /*ac*/, char * * /*av*/) -> int
{
   argsy::options_description desc;
   desc.add_options()("input,i", argsy::value<std::string>(), "the input file")(
      "optimization,O", argsy::value<unsigned>(), "optimization level")("opt,o", argsy::value<std::string>(), "misc option");

   split_whitespace(desc);
   split_equalsign(desc);
   split_semi(desc);
   split_quotes(desc);
   split_escape(desc);
   split_single_quote(desc);
   split_defaults(desc);

   return 0;
}
